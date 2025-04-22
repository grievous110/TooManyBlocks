#ifndef TOOMANYBLOCKS_BOUNDINGVOLUME_H
#define TOOMANYBLOCKS_BOUNDINGVOLUME_H

#include <array>
#include <cfloat>
#include <glm/glm.hpp>

#include "datatypes/RawBuffer.h"
#include "engine/env/lights/Light.h"
#include "engine/rendering/Renderable.h"

// Axis Aligned Bounding Box
struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;

    static BoundingBox invalid() { return {glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX)}; }

    static BoundingBox notCullable() { return {glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX)}; }

    inline glm::vec3 center() const { return (min + max) * 0.5f; }

    inline BoundingBox movedBy(const glm::vec3& delta) const { return {min + delta, max + delta}; }

    inline bool isInvalid() const { return min == glm::vec3(FLT_MAX) && max == glm::vec3(-FLT_MAX); }

    inline bool isNotCullable() const { return min == glm::vec3(-FLT_MAX) && max == glm::vec3(FLT_MAX); }

    inline bool operator==(const BoundingBox& other) const { return min == other.min && max == other.max; }

    inline bool operator!=(const BoundingBox& other) const { return !(*this == other); }
};

#endif