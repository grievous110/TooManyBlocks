#ifndef CHUNK_H
#define CHUNK_H

#include "datatypes/DatatypeDefs.h"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

constexpr int CHUNK_SIZE = 32;
constexpr int CHUNK_WIDTH = CHUNK_SIZE;
constexpr int CHUNK_DEPTH = CHUNK_SIZE;
constexpr int CHUNK_HEIGHT = CHUNK_SIZE;
constexpr int CHUNK_SLICE_SIZE = CHUNK_WIDTH * CHUNK_HEIGHT; 	// Vertical slice size in a chunk
constexpr int CHUNK_PLANE_SIZE = CHUNK_WIDTH * CHUNK_DEPTH; 	// Horizontal plane size in a chunk
constexpr int BLOCKS_PER_CHUNK = CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT;

class Mesh;

struct coord_hash {
	size_t operator() (const glm::ivec3& v) const {
		size_t h1 = std::hash<int>()(v.x);
        size_t h2 = std::hash<int>()(v.y);
        size_t h3 = std::hash<int>()(v.z);
		// Combining the hashes
        return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};

struct Block {
	uint16_t type;
	bool isSolid;
};

class Chunk {
	friend class World;

private:
	bool m_isBeingRebuild;
	bool m_changed;
	std::shared_ptr<Block[]> m_blocks;
	std::shared_ptr<Mesh> m_mesh;

public:	
	static glm::ivec3 worldToChunkOrigin(const glm::vec3& worldPos);
	static glm::ivec3 worldToChunkLocal(const glm::ivec3& chunkOrigin, const glm::ivec3& worldBlockPos);

	Chunk() : m_isBeingRebuild(false), m_changed(false) {}

	inline bool isBeingRebuild() const { return m_isBeingRebuild; }
	inline bool isChanged() const { return m_changed; }
	inline bool isLoaded() const { return m_blocks != nullptr; }
	inline const Block* blocks() const { return m_blocks.get(); }
	inline Mesh* getMesh() const { return m_mesh.get(); }
};

constexpr int chunkBlockIndex(int x, int y, int z) {
    return z * CHUNK_SLICE_SIZE  + y * CHUNK_WIDTH + x;
}

bool isBlockFaceVisible(const Block* blocks, int x, int y, int z, AxisDirection faceDirection);

#endif