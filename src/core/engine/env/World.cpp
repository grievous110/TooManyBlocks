#include "World.h"

#include <json/JsonParser.h>

#include <algorithm>
#include <fstream>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "AppConstants.h"
#include "Application.h"
#include "Logger.h"
#include "engine/rendering/MeshCreate.h"
#include "engine/rendering/StaticMesh.h"
#include "engine/rendering/mat/ChunkMaterial.h"
#include "engine/worldgen/PerlinNoise.h"
#include "providers/Provider.h"

static void generateChunkBlocks(Block* blocks, const glm::ivec3& chunkPos, uint32_t seed) {
    PerlinNoise noiseGenerator(seed);

    // Generate height values for the xz plane in global coordinates
    std::unique_ptr<float[]> heightValues =
        noiseGenerator.generatePerlinNoise({CHUNK_WIDTH, CHUNK_DEPTH}, {chunkPos.x, chunkPos.z}, 32, 2);
    std::unique_ptr<float[]> ironOre = noiseGenerator.generatePerlinNoise(
        {CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH}, {chunkPos.x, chunkPos.y, chunkPos.z}, 16, 2
    );

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                // Surface height
                float surfaceHeight = heightValues[z * CHUNK_DEPTH + x] * 10.0f;

                // Global y coordinate
                int globalY = chunkPos.y + y;

                // Get iron ore noise value
                float ironValue = ironOre[z * CHUNK_HEIGHT * CHUNK_WIDTH + y * CHUNK_WIDTH + x];

                // Conditions for placing blocks
                if (globalY < static_cast<int>(floor(surfaceHeight))) {
                    // Default to stone
                    blocks[chunkBlockIndex(x, y, z)] = {STONE, true};

                    // Apply ore generation logic
                    if (globalY < 0) {           // Only generate iron below Y=60
                        float threshold = 0.6f;  // Adjust spawn probability

                        if (ironValue > threshold) {
                            blocks[chunkBlockIndex(x, y, z)] = {IRON_ORE, true};
                        }
                    }
                } else if (globalY == static_cast<int>(floor(surfaceHeight))) {
                    blocks[chunkBlockIndex(x, y, z)] = {GRASS, true};
                } else {
                    blocks[chunkBlockIndex(x, y, z)] = {AIR, false};
                }
            }
        }
    }
}

std::unordered_set<glm::ivec3, coord_hash> World::determineActiveChunks(
    const glm::ivec3& position, int renderDistance
) {
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
    return activeChunks;
}

void World::processDataFromWorkers() {
    if (ApplicationContext* context = Application::getContext()) {
        Provider* provider = context->provider;
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

        while (!m_workerResultQueue.empty()) {
            WorkerResult data = std::move(m_workerResultQueue.front());
            m_workerResultQueue.pop();

            auto it = m_loadedChunks.find(data.chunkPos);
            if (it != m_loadedChunks.end()) {
                Chunk& chunk = it->second;

                data.meshBlueprint->bake();  // Upload meshdata to gpu
                if (!chunk.isLoaded()) {
                    // 1. Case: Chunk has been initially loaded (blocks + mesh must be set)
                    chunk.m_blocks = std::move(data.blockData);
                    chunk.m_mesh = std::make_unique<StaticMesh>(
                        std::static_pointer_cast<StaticMesh::Internal>(data.meshBlueprint->createInstance()), material
                    );
                    chunk.m_mesh->getLocalTransform().setPosition(data.chunkPos);
                } else if (chunk.isBeingRebuild()) {
                    // 2. Case: Chunk has been rebuilded (Reset flag and replace mesh)
                    chunk.m_isBeingRebuild = false;

                    // *No need to set blocks*
                    chunk.m_mesh = std::make_unique<StaticMesh>(
                        std::static_pointer_cast<StaticMesh::Internal>(data.meshBlueprint->createInstance()), material
                    );
                    chunk.m_mesh->getLocalTransform().setPosition(data.chunkPos);
                }
            }
        }
    }
}

World::World(const std::filesystem::path& worldDir) : m_worldDir(worldDir), m_cStorage(worldDir) {
    // Load world data
    std::ifstream file((worldDir / "info.json").string(), std::ios::binary);
    Json::JsonValue info =
        Json::parseJson(std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()));
    m_seed = static_cast<uint32_t>(std::stoul(info["seed"].toString()));
    file.close();
}

Chunk* World::getChunk(const glm::ivec3& location) {
    auto it = m_loadedChunks.find(location);
    if (it != m_loadedChunks.end()) {
        if (it->second.isLoaded()) {
            return &it->second;
        }
    }
    return nullptr;
}

void World::updateChunks(const glm::ivec3& position, int renderDistance) {
    // Step 1: Determine active chunk positions
    std::unordered_set<glm::ivec3, coord_hash> activeChunks = determineActiveChunks(position, renderDistance);

    // Step 2: Unload chunks that are no longer in the active set
    for (auto it = m_loadedChunks.begin(); it != m_loadedChunks.end();) {
        if (activeChunks.find(it->first) == activeChunks.end()) {
            if (it->second.isMarkedForSave()) {
                // Save chunk that will be unloaded but has changes
                ThreadPool* tPool = Application::getContext()->workerPool;

                glm::ivec3 chunkPos = it->first;
                std::shared_ptr<Block[]> blockData = std::move(it->second.m_blocks);
                tPool->pushJob(this, [this, chunkPos, blockData] {
                    try {
                        m_cStorage.saveChunkData(chunkPos, blockData.get());
                    } catch (const std::exception& e) {
                        lgr::lout.error(e.what());
                    }
                });
            }
            it = m_loadedChunks.erase(it);
        } else {
            ++it;  // Chunk is still active
        }
    }

    // Step 3: Process data from queues
    {
        std::lock_guard<std::mutex> lock(m_chunkGenQueueMtx);
        if (!m_workerResultQueue.empty()) {
            processDataFromWorkers();
        }
    }

    // Step 4: Process pending changes for unloaded chunks
    // * Currently unhanlded *
    m_pendingChanges.clear();

    // Step 5: Load chunks that are in the active set but not yet loaded or need mesh rebuild
    for (const glm::ivec3& chunkPos : activeChunks) {
        auto it = m_loadedChunks.find(chunkPos);
        if (it == m_loadedChunks.end()) {
            // Chunk does not exist -> Needs to be fully loaded
            // Put placeholder chunk (Chunk with no block data / mesh)
            m_loadedChunks[chunkPos] = Chunk();

            // Create new one
            ThreadPool* tPool = Application::getContext()->workerPool;
            tPool->pushJob(this, [this, chunkPos] {
                std::unique_ptr<Block[]> blocks;
                if (m_cStorage.hasChunk(chunkPos)) {
                    blocks = m_cStorage.loadChunkData(chunkPos);
                } else {
                    blocks = std::unique_ptr<Block[]>(new Block[BLOCKS_PER_CHUNK], std::default_delete<Block[]>());
                    generateChunkBlocks(blocks.get(), chunkPos, m_seed);
                }

                std::unique_ptr<IBlueprint> meshBp = generateMeshForChunkGreedy(blocks.get(), texMap);

                {
                    std::lock_guard<std::mutex> lock(m_chunkGenQueueMtx);
                    m_workerResultQueue.push({chunkPos, std::move(blocks), std::move(meshBp)});
                }
            });
        } else if (it->second.isChanged() && !it->second.isBeingRebuild()) {
            // Chunk already exists and needs a rebuild (and no other worker is currently rebuilding this) -> rebuild
            // only mesh data
            ThreadPool* tPool = Application::getContext()->workerPool;

            // Make copy of blockdata
            std::shared_ptr<Block[]> blocksCopy(new Block[BLOCKS_PER_CHUNK], std::default_delete<Block[]>());
            const Block* src = it->second.blocks();
            std::copy(src, src + BLOCKS_PER_CHUNK, blocksCopy.get());

            it->second.m_isBeingRebuild = true;
            it->second.m_changed = false;
            tPool->pushJob(this, [this, chunkPos, blocksCopy] {
                std::unique_ptr<IBlueprint> meshBp = generateMeshForChunkGreedy(blocksCopy.get(), texMap);

                {
                    std::lock_guard<std::mutex> lock(m_chunkGenQueueMtx);
                    m_workerResultQueue.push({chunkPos, nullptr, std::move(meshBp)});
                }
            });
        }
    }
}

void World::syncedSaveChunks() {
    for (auto& entry : m_loadedChunks) {
        if (entry.second.isMarkedForSave()) {
            m_cStorage.saveChunkData(entry.first, entry.second.blocks());
            entry.second.m_isMarkedForSave = false;
        }
    }
}

void World::setBlock(const glm::ivec3& position, uint16_t newBlock) {
    glm::ivec3 chunkPos = Chunk::worldToChunkOrigin(position);
    if (Chunk* chunk = getChunk(chunkPos)) {
        // Immediate data change if chunk is loaded
        glm::ivec3 relChunkPos = Chunk::worldToChunkLocal(chunkPos, position);
        chunk->m_blocks[chunkBlockIndex(relChunkPos.x, relChunkPos.y, relChunkPos.z)] = {newBlock, newBlock != AIR};
        chunk->m_changed = true;
        chunk->m_isMarkedForSave = true;
    } else {
        // Queue changes
        m_pendingChanges[position] = newBlock;
    }
}