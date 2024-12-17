#include "engine/rendering/Mesh.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"
#include "engine/worldgen/PerlinNoise.h"
#include "engine/env/World.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

using namespace std;

Chunk* World::getChunk(const glm::ivec3& location) {
    if (m_loadedChunks.find(location) != m_loadedChunks.end()) {
        return &m_loadedChunks[location];
    }
    return nullptr;
}

void World::loadChunk(const glm::ivec3& location) {
    if (m_loadedChunks.find(location) == m_loadedChunks.end()) {
        
        Chunk newChunk;
        PerlinNoise noiseGenerator(m_seed);
        shared_ptr<float> heightValues = noiseGenerator.generatePerlinNoise({CHUNK_WIDTH, CHUNK_DEPTH}, {location.x, location.z}, 64, 2);

        for (int x = 0; x < CHUNK_WIDTH; x++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                float height = heightValues.get()[z * CHUNK_DEPTH + x] * 10.0f;
                for (int y = 0; y < CHUNK_HEIGHT; y++) {
                    newChunk.blocks[chunkBlockIndex(x, y, z)] = {0, y < height};
                }                
            }
        }

        m_loadedChunks[location] = newChunk;
    }
}

void World::unloadChunk(const glm::ivec3& location) {
    m_loadedChunks.erase(location);
}