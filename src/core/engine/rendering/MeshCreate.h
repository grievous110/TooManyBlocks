#ifndef TOOMANYBLOCKS_MESHCREATE_H
#define TOOMANYBLOCKS_MESHCREATE_H

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

#include "engine/blueprints/Blueprint.h"
#include "engine/env/Chunk.h"
#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/BlockToTextureMapping.h"
#include "engine/rendering/RenderData.h"
#include "engine/rendering/StaticMesh.h"
#include "util/BitOperations.h"

#define POSITION_BITMASK  0x3FF
#define X_POSITION_OFFSET 20
#define Y_POSITION_OFFSET 10
#define Z_POSITION_OFFSET 0

#define TEXINDEX_BITMASK  0xFFFF
#define TEXINDEX_OFFSET   16

#define X_UV_BITMASK      0x3F
#define Y_UV_BITMASK      0x3F
#define X_UV_OFFSET       10
#define Y_UV_OFFSET       4

#define NORMAL_BITMASK    0x07
#define NORMAL_OFFSET     0

struct UVCoord {
    uint8_t x : 6;
    uint8_t y : 6;
};

struct CompactChunkVertex {
    uint32_t position;    // 4 bytes compressed for 10bit positions (x, y, z)
    uint32_t packedData;  // 4 bytes for compressed data (texIndex, normal, uv)

    CompactChunkVertex(
        const glm::ivec3& pos = glm::ivec3(0),
        uint16_t texIndex = 0,
        UVCoord uv = {0, 0},
        AxisDirection normal = AxisDirection::PositiveX
    ) {
        setPosition(pos);
        setTexIndex(texIndex);
        setUV(uv);
        setNormal(normal);
    }

    inline void setPosition(const glm::ivec3& pos) {
        SET_BITS(
            position, static_cast<uint32_t>(pos.x), POSITION_BITMASK, X_POSITION_OFFSET
        );  // 10 bit x coord [0 - 1023]
        SET_BITS(
            position, static_cast<uint32_t>(pos.y), POSITION_BITMASK, Y_POSITION_OFFSET
        );  // 10 bit y coord [0 - 1023]
        SET_BITS(
            position, static_cast<uint32_t>(pos.z), POSITION_BITMASK, Z_POSITION_OFFSET
        );  // 10 bit z coord [0 - 1023]
    }

    inline void setTexIndex(uint16_t texIndex) {
        SET_BITS(
            packedData, static_cast<uint32_t>(texIndex), TEXINDEX_BITMASK, TEXINDEX_OFFSET
        );  // 16 bit texture index [0 - 65535]
    }

    inline void setUV(UVCoord uv) {
        SET_BITS(packedData, static_cast<uint32_t>(uv.x), X_UV_BITMASK, X_UV_OFFSET);  // 6 bit x uv coord [0 - 63]
        SET_BITS(packedData, static_cast<uint32_t>(uv.y), Y_UV_BITMASK, Y_UV_OFFSET);  // 6 bit y uv coord [0 - 63]
    }

    inline void setNormal(AxisDirection normal) {
        SET_BITS(packedData, static_cast<uint32_t>(normal), NORMAL_BITMASK, NORMAL_OFFSET);  // Compressed in 3 bit
    }

    inline glm::ivec3 getPosition() const {
        glm::ivec3 pos;
        pos.x = GET_BITS(position, POSITION_BITMASK, X_POSITION_OFFSET);
        pos.y = GET_BITS(position, POSITION_BITMASK, Y_POSITION_OFFSET);
        pos.z = GET_BITS(position, POSITION_BITMASK, Z_POSITION_OFFSET);
        return pos;
    }

    inline glm::vec3 getNormal() const {
        switch (getNormalEnum()) {
            case AxisDirection::PositiveX: return glm::vec3(1.0f, 0.0f, 0.0f);
            case AxisDirection::NegativeX: return glm::vec3(-1.0f, 0.0f, 0.0f);
            case AxisDirection::PositiveY: return glm::vec3(0.0f, 1.0f, 0.0f);
            case AxisDirection::NegativeY: return glm::vec3(0.0f, -1.0f, 0.0f);
            case AxisDirection::PositiveZ: return glm::vec3(0.0f, 0.0f, 1.0f);
            case AxisDirection::NegativeZ: return glm::vec3(0.0f, 0.0f, -1.0f);
            default: return glm::vec3(0.0f, 0.0f, -1.0f);
        }
    }

    inline uint16_t getTexIndex() const {
        return static_cast<uint16_t>(GET_BITS(packedData, TEXINDEX_BITMASK, TEXINDEX_OFFSET));
    }

    inline UVCoord getUV() const {
        UVCoord uv;
        uv.x = static_cast<uint8_t>(GET_BITS(packedData, X_UV_BITMASK, X_UV_OFFSET));
        uv.y = static_cast<uint8_t>(GET_BITS(packedData, Y_UV_BITMASK, Y_UV_OFFSET));
        return uv;
    }

    inline AxisDirection getNormalEnum() const {
        return static_cast<AxisDirection>(GET_BITS(packedData, NORMAL_BITMASK, NORMAL_OFFSET));
    }
};

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;

    bool operator==(const Vertex& other) const {
        return position == other.position && uv == other.uv && normal == other.normal;
    }
};

struct SkeletalVertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::uvec4 joints;
    glm::vec4 weights;
};

std::unique_ptr<IBlueprint> generateMeshForChunk(const Block* blocks, const BlockToTextureMap& texMap);

std::unique_ptr<IBlueprint> generateMeshForChunkGreedy(const Block* blocks, const BlockToTextureMap& texMap);

std::unique_ptr<IBlueprint> readMeshDataFromObjFile(const std::string& filePath, bool flipWinding = false);

std::unique_ptr<IBlueprint> readSkeletalMeshFromGlbFile(const std::string& filePath, bool flipWinding = false);

#endif