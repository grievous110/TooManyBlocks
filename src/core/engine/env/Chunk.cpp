#include "engine/env/Chunk.h"

bool isBlockFaceVisible(const Chunk& chunk, int x, int y, int z, AxisDirection faceDirection) {
    // Check chunk/world bounds
    if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_DEPTH) {
        return true; // Block is on the edge and visible from this direction
    }

    // Access neighboring block based on face direction
    switch (faceDirection) {
    case AxisDirection::NegativeX:  // (x - 1, y, z) LEFT
        return (x - 1 < 0 || !chunk.blocks[chunkBlockIndex(x - 1, y, z)].isSolid);
    case AxisDirection::PositiveX: // (x + 1, y, z) RIGHT
        return (x + 1 >= CHUNK_WIDTH || !chunk.blocks[chunkBlockIndex(x + 1, y, z)].isSolid);
    case AxisDirection::PositiveY:   // (x, y + 1, z) TOP
        return (y + 1 >= CHUNK_HEIGHT || !chunk.blocks[chunkBlockIndex(x, y + 1, z)].isSolid);
    case AxisDirection::NegativeY:// (x, y - 1, z) BOTTOM
        return (y - 1 < 0 || !chunk.blocks[chunkBlockIndex(x, y - 1, z)].isSolid);
    case AxisDirection::PositiveZ: // (x, y, z + 1) FRONT
        return (z + 1 >= CHUNK_DEPTH || !chunk.blocks[chunkBlockIndex(x, y, z + 1)].isSolid);
    case AxisDirection::NegativeZ:  // (x, y, z - 1) BACK
        return (z - 1 < 0 || !chunk.blocks[chunkBlockIndex(x, y, z - 1)].isSolid);
    default:
        return false;
    }
}