#ifndef CHUNKSTORAGE_H
#define CHUNKSTORAGE_H

#include "engine/env/Chunk.h"
#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

class ChunkStorage {
private:
    const std::filesystem::path m_chunkStoragePath;
    mutable std::unordered_map<glm::ivec3, std::weak_ptr<std::mutex>, coord_hash> m_chunkLocks;
    mutable std::mutex m_lockMapMutex;

    std::shared_ptr<std::mutex> getChunkMutex(const glm::ivec3& pos) const;

public:
    ChunkStorage(const std::filesystem::path& worldPath);
    
    bool hasChunk(const glm::ivec3& chunkPos) const;

    std::shared_ptr<Block[]> loadChunkData(const glm::ivec3& chunkPos) const;

    void saveChunkData(const glm::ivec3& chunkPos, const Block* blocks) const;
};

#endif