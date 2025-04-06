#include "AppConstants.h"
#include "Application.h"
#include "engine/rendering/mat/ChunkMaterial.h"
#include "engine/rendering/Mesh.h"
#include "engine/rendering/MeshCreate.h"
#include "engine/worldgen/PerlinNoise.h"
#include "Logger.h"
#include "providers/Provider.h"
#include "World.h"
#include <fstream>
#include <glm/glm.hpp>
#include <json/JsonParser.h>
#include <memory>
#include <unordered_set>
#include <vector>

World::World(const std::filesystem::path& worldDir) : m_worldDir(worldDir) {
    // Load world data
    std::ifstream file((worldDir / "info.json").string(), std::ios::binary);
    Json::JsonValue info = Json::parseJson(std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()));
    m_seed = static_cast<uint32_t>(std::stoul(info["seed"].toString()));
    file.close();
}

std::shared_ptr<Chunk> World::getChunk(const glm::ivec3& location) {
    if (m_loadedChunks.find(location) != m_loadedChunks.end()) {
        return m_loadedChunks[location];
    }
    return nullptr;
}

void World::updateChunks(const glm::ivec3 &position, int renderDistance) {
    std::unordered_set<glm::ivec3, coord_hash> activeChunks;
    
    glm::ivec3 centerChunk(
        (position.x < 0 ? (position.x - CHUNK_WIDTH + 1) : position.x) / CHUNK_WIDTH * CHUNK_WIDTH,
        (position.y < 0 ? (position.y - CHUNK_HEIGHT + 1) : position.y) / CHUNK_HEIGHT * CHUNK_HEIGHT,
        (position.z < 0 ? (position.z - CHUNK_DEPTH + 1) : position.z) / CHUNK_DEPTH * CHUNK_DEPTH
    );

    // Compute the range of chunk coordinates to load
    for (int x = -renderDistance * CHUNK_WIDTH; x <= renderDistance * CHUNK_WIDTH; x += CHUNK_WIDTH) {
        for (int y = -renderDistance * CHUNK_HEIGHT; y <= renderDistance * CHUNK_HEIGHT; y += CHUNK_HEIGHT) {
            for (int z = -renderDistance * CHUNK_DEPTH; z <= renderDistance * CHUNK_DEPTH; z += CHUNK_DEPTH) {
                glm::ivec3 offset(x * CHUNK_WIDTH, y * CHUNK_HEIGHT, z * CHUNK_DEPTH);
                glm::ivec3 chunkPos = centerChunk + glm::ivec3(x, y, z);

                // Only include chunks within Euclidean distance
                if (glm::length(glm::vec3(chunkPos - centerChunk)) <= renderDistance * CHUNK_WIDTH) {
                    activeChunks.insert(chunkPos);
                }
            }
        }
    }

    // Step 1: Process data from queues
    {
        std::lock_guard<std::mutex> lock(m_chunkGenQueueMtx);
        
        if (!m_loadedMeshData.empty()) {
            Provider* provider = Application::getContext()->provider;

            std::shared_ptr<Material> material = provider->getChachedMaterial("ChunkMaterial");
            // Check if material has been cached and create it if not
            if (!material) {
                std::shared_ptr<Shader> shader = provider->getShaderFromFile(Res::Shader::CHUNK);
                std::shared_ptr<Shader> depthShader = provider->getShaderFromFile(Res::Shader::CHUNK_DEPTH);
                std::shared_ptr<Shader> ssaoGBuffShader = provider->getShaderFromFile(Res::Shader::CHUNK_SSAO_GBUFFER);
                std::shared_ptr<Texture> texture = provider->getTextureFromFile(Res::Texture::BLOCK_TEX_ATLAS);
                material = std::make_shared<ChunkMaterial>(shader, depthShader, ssaoGBuffShader, texture);
                provider->putMaterial("ChunkMaterial", material);
            }

            while (!m_loadedMeshData.empty()) {
                std::tuple<glm::ivec3, std::shared_ptr<Chunk>, std::shared_ptr<RawChunkMeshData>> data = m_loadedMeshData.front();
                m_loadedMeshData.pop();
                
                glm::ivec3 chunkPos = std::get<glm::ivec3>(data);
                std::shared_ptr<Chunk> chunk = std::get<std::shared_ptr<Chunk>>(data);

                chunk->mesh = buildFromChunkMeshData(*std::get<std::shared_ptr<RawChunkMeshData>>(data));
                chunk->mesh->assignMaterial(material);
                chunk->mesh->getLocalTransform().setPosition(chunkPos);
                m_loadedChunks[chunkPos] = chunk;
            }
        }
    }

    // Step 2: Unload chunks that are no longer in the active set
    for (auto it = m_loadedChunks.begin(); it != m_loadedChunks.end(); ) {
        if (activeChunks.find(it->first) == activeChunks.end()) {
            it = m_loadedChunks.erase(it);
        } else {
            ++it; // Chunk is still active
        }
    }

    // Step 3: Load chunks that are in the active set but not yet loaded
    for (const glm::ivec3& chunkPos : activeChunks) {
        if (m_loadedChunks.find(chunkPos) == m_loadedChunks.end()) {            
            // Put placeholder
            m_loadedChunks[chunkPos] = nullptr;

            ThreadPool* tPool = Application::getContext()->workerPool;
            tPool->pushJob(this, [this, chunkPos] {
                std::shared_ptr<Chunk> newChunk = std::make_shared<Chunk>();
                PerlinNoise noiseGenerator(m_seed);

                // Generate height values for the xz plane in global coordinates
                std::shared_ptr<float> heightValues = noiseGenerator.generatePerlinNoise(
                    {CHUNK_WIDTH, CHUNK_DEPTH}, {chunkPos.x, chunkPos.z}, 32, 2
                );
                std::shared_ptr<float> ironOre = noiseGenerator.generatePerlinNoise(
                    {CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH}, {chunkPos.x, chunkPos.y, chunkPos.z}, 16, 2
                );

                for (int x = 0; x < CHUNK_WIDTH; x++) {
                    for (int y = 0; y < CHUNK_HEIGHT; y++) {
                        for (int z = 0; z < CHUNK_DEPTH; z++) {
                            // Surface height
                            float surfaceHeight = heightValues.get()[z * CHUNK_DEPTH + x] * 10.0f;
                
                            // Global y coordinate
                            int globalY = chunkPos.y + y;
                            
                            // Get iron ore noise value
                            float ironValue = ironOre.get()[z * CHUNK_HEIGHT * CHUNK_WIDTH + y * CHUNK_WIDTH + x];
                
                            // Conditions for placing blocks
                            if (globalY < static_cast<int>(floor(surfaceHeight))) {
                                // Default to stone
                                newChunk->blocks[chunkBlockIndex(x, y, z)] = {STONE, true};
                
                                // Apply ore generation logic
                                if (globalY < 0) { // Only generate iron below Y=60
                                    float threshold = 0.6f; // Adjust spawn probability
                                    
                                    if (ironValue > threshold ) {
                                        newChunk->blocks[chunkBlockIndex(x, y, z)] = {IRON_ORE, true};
                                    }
                                }
                            } else if (globalY == static_cast<int>(floor(surfaceHeight))) {
                                newChunk->blocks[chunkBlockIndex(x, y, z)] = {GRASS, true};
                            } else {
                                newChunk->blocks[chunkBlockIndex(x, y, z)] = {GRASS, false};
                            }
                        }
                    }
                }

                std::shared_ptr<RawChunkMeshData> meshData = generateMeshForChunkGreedy(*newChunk, texMap);

                {
                    std::lock_guard<std::mutex> lock(m_chunkGenQueueMtx);
                    m_loadedMeshData.push(std::make_tuple(chunkPos, newChunk, meshData));
                }
            });
        }
    }
}