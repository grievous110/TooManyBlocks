#ifndef WORLD_H
#define WORLD_H

#include "engine/env/Chunk.h"
#include "engine/rendering/BlockToTextureMapping.h"
#include "engine/rendering/MeshCreate.h"
#include "threading/ThreadPool.h"
#include <glm/vec3.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <tuple>
#include <unordered_map>

class World {
private:
	uint32_t m_seed;
	ThreadPool workerPool;
	std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>, coord_hash> m_loadedChunks;
	std::queue<std::tuple<glm::ivec3, std::shared_ptr<Chunk>, std::shared_ptr<RawChunkMeshData>>> m_loadedMeshData;
	std::mutex m_chunkMeshDataMtx;
	
public:
	const BlockToTextureMap texMap;

	World(uint32_t seed) : m_seed(seed), workerPool(4) {}

	uint32_t seed() const { return m_seed; }

	std::shared_ptr<Chunk> getChunk(const glm::ivec3& location);

	void updateChunks(const glm::ivec3& position, int renderDistance);

	const std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>, coord_hash>& loadedChunks() const { return m_loadedChunks; }
};

#endif