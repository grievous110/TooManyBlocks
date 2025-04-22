#ifndef TOOMANYBLOCKS_CHUNK_H
#define TOOMANYBLOCKS_CHUNK_H

#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

#include "datatypes/DatatypeDefs.h"

constexpr int CHUNK_SIZE = 32;
constexpr int CHUNK_WIDTH = CHUNK_SIZE;
constexpr int CHUNK_DEPTH = CHUNK_SIZE;
constexpr int CHUNK_HEIGHT = CHUNK_SIZE;
constexpr int CHUNK_SLICE_SIZE = CHUNK_WIDTH * CHUNK_HEIGHT;  // Vertical slice size in a chunk
constexpr int CHUNK_PLANE_SIZE = CHUNK_WIDTH * CHUNK_DEPTH;   // Horizontal plane size in a chunk
constexpr int BLOCKS_PER_CHUNK = CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT;

class StaticMesh;

struct coord_hash {
    size_t operator()(const glm::ivec3& v) const {
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
    bool m_isBeingRebuild;   // true if a worker is currently building its mesh
    bool m_changed;          // If any block has been changed since the last rebuild started
    bool m_isMarkedForSave;  // If there are changes that need to be written back chunk file
    std::shared_ptr<Block[]> m_blocks;
    std::shared_ptr<StaticMesh> m_mesh;

public:
    static glm::ivec3 worldToChunkOrigin(const glm::vec3& worldPos);
    static glm::ivec3 worldToChunkLocal(const glm::ivec3& chunkOrigin, const glm::ivec3& worldBlockPos);

    Chunk() : m_isBeingRebuild(false), m_changed(false), m_isMarkedForSave(false) {}

    inline bool isBeingRebuild() const { return m_isBeingRebuild; }
    inline bool isChanged() const { return m_changed; }
    inline bool isMarkedForSave() const { return m_isMarkedForSave; }
    inline bool isLoaded() const { return m_blocks != nullptr; }
    inline const Block* blocks() const { return m_blocks.get(); }
    inline StaticMesh* getMesh() const { return m_mesh.get(); }
};

constexpr int chunkBlockIndex(int x, int y, int z) { return z * CHUNK_SLICE_SIZE + y * CHUNK_WIDTH + x; }

bool isBlockFaceVisible(const Block* blocks, int x, int y, int z, AxisDirection faceDirection);

#endif