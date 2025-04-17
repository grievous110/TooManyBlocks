#ifndef WORLD_H
#define WORLD_H

#include "engine/env/Chunk.h"
#include "engine/persistence/ChunkStorage.h"
#include "engine/rendering/BlockToTextureMapping.h"
#include "engine/rendering/MeshCreate.h"
#include "threading/ThreadPool.h"
#include <filesystem>
#include <glm/vec3.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

class World {
private:
	uint32_t m_seed;
	const std::filesystem::path m_worldDir;
	ChunkStorage m_cStorage;
	std::unordered_map<glm::ivec3, Chunk, coord_hash> m_loadedChunks;
	std::queue<std::tuple<glm::ivec3, std::shared_ptr<Block[]>, std::shared_ptr<RawChunkMeshData>>> m_loadedMeshData;
	std::unordered_map<glm::ivec3, uint16_t, coord_hash> m_pendingChanges;
	std::mutex m_chunkGenQueueMtx;	
	
	std::unordered_set<glm::ivec3, coord_hash> determineActiveChunks(const glm::ivec3& position, int renderDistance);
	
	void processDataFromWorkers();

public:
	const BlockToTextureMap texMap;

	World(const std::filesystem::path& worldDir);

	uint32_t seed() const { return m_seed; }

	Chunk* getChunk(const glm::ivec3& location);

	void updateChunks(const glm::ivec3& position, int renderDistance);

	void syncedSaveChunks();

	void setBlock(const glm::ivec3& position, uint16_t newBlocks);

	const std::unordered_map<glm::ivec3, Chunk, coord_hash>& loadedChunks() const { return m_loadedChunks; }
};

#endif