#ifndef WORLD_H
#define WORLD_H

#include "engine/env/Chunk.h"
#include <glm/vec3.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

class World {
private:
	uint32_t m_seed;
	std::unordered_map<glm::ivec3, Chunk, coord_hash> m_loadedChunks;

public:
	World(uint32_t seed) : m_seed(seed) {}

	uint32_t seed() const { return m_seed; }

	Chunk* getChunk(const glm::ivec3& location);

	void loadChunk(const glm::ivec3& location);

	void unloadChunk(const glm::ivec3& location);
};

#endif