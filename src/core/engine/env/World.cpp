#include "Application.h"
#include "engine/rendering/mat/ChunkMaterial.h"
#include "engine/rendering/Mesh.h"
#include "engine/rendering/MeshCreate.h"
#include "engine/rendering/ShaderPathsConstants.h"
#include "engine/worldgen/PerlinNoise.h"
#include "Logger.h"
#include "providers/Provider.h"
#include "World.h"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_set>
#include <vector>

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
        std::lock_guard<std::mutex> lock(m_chunkMeshDataMtx);
        
        if (!m_loadedMeshData.empty()) {
            Provider* provider = Application::getContext()->provider;

            std::shared_ptr<Material> material = provider->getChachedMaterial("ChunkMaterial");
            if (!material) {
                std::shared_ptr<Shader> shader = provider->getShaderFromFile(CHUNK_SHADER);
                std::shared_ptr<Shader> depthShader = provider->getShaderFromFile(CHUNK_DEPTH_SHADER);
                std::shared_ptr<Shader> ssaoGBuffShader = provider->getShaderFromFile(CHUNK_SSAO_GBUFFER_SHADER);
                std::shared_ptr<Texture> texture = provider->getTextureFromFile("res/textures/blockTexAtlas.png");
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

            workerPool.pushJob(
                [this, chunkPos] {
                    std::shared_ptr<Chunk> newChunk = std::make_shared<Chunk>();
                    PerlinNoise noiseGenerator(m_seed);

                    // Generate height values for the xz plane in global coordinates
                    std::shared_ptr<float> heightValues = noiseGenerator.generatePerlinNoise(
                        {CHUNK_WIDTH, CHUNK_DEPTH}, {chunkPos.x, chunkPos.z}, 32, 2
                    );

                    for (int x = 0; x < CHUNK_WIDTH; x++) {
                        for (int z = 0; z < CHUNK_DEPTH; z++) {
                            // Height based on noise
                            float height = heightValues.get()[z * CHUNK_DEPTH + x] * 10.0f;

                            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                                // Global y coordinate
                                int globalY = chunkPos.y + y;

                                if (globalY < static_cast<int>(floor(height))) {
                                    newChunk->blocks[chunkBlockIndex(x, y, z)] = {STONE,  true};
                                } else if (globalY == static_cast<int>(floor(height))) {
                                    newChunk->blocks[chunkBlockIndex(x, y, z)] = {GRASS,  true};
                                }
                            }
                        }
                    }

                    std::shared_ptr<RawChunkMeshData> meshData = generateMeshForChunkGreedy(*newChunk, texMap);

                    {
                        std::lock_guard<std::mutex> lock(m_chunkMeshDataMtx);
                        m_loadedMeshData.push(std::make_tuple(chunkPos, newChunk, meshData));
                    }
                }
            );
        }
    }
}