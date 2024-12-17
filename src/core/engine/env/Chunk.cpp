#include "engine/env/Chunk.h"

bool isBlockFaceVisible(const Chunk& chunk, int x, int y, int z, FaceDirection faceDirection) {
    // Check chunk/world bounds
    if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_DEPTH) {
        return true; // Block is on the edge and visible from this direction
    }

    // Access neighboring block based on face direction
    switch (faceDirection) {
    case LEFT:  // (x - 1, y, z)
        return (x - 1 < 0 || !chunk.blocks[chunkBlockIndex(x - 1, y, z)].isSolid);
    case RIGHT: // (x + 1, y, z)
        return (x + 1 >= CHUNK_WIDTH || !chunk.blocks[chunkBlockIndex(x + 1, y, z)].isSolid);
    case TOP:   // (x, y + 1, z)
        return (y + 1 >= CHUNK_HEIGHT || !chunk.blocks[chunkBlockIndex(x, y + 1, z)].isSolid);
    case BOTTOM:// (x, y - 1, z)
        return (y - 1 < 0 || !chunk.blocks[chunkBlockIndex(x, y - 1, z)].isSolid);
    case FRONT: // (x, y, z + 1)
        return (z + 1 >= CHUNK_DEPTH || !chunk.blocks[chunkBlockIndex(x, y, z + 1)].isSolid);
    case BACK:  // (x, y, z - 1)
        return (z - 1 < 0 || !chunk.blocks[chunkBlockIndex(x, y, z - 1)].isSolid);
    default:
        return false;
    }
}