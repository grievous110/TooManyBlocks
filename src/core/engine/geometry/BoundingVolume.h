#ifndef TOOMANYBLOCKS_BOUNDINGVOLUME_H
#define TOOMANYBLOCKS_BOUNDINGVOLUME_H

#include <cfloat>
#include <glm/glm.hpp>

struct BoundingSphere {
    glm::vec3 center;
    float radius;

    static BoundingSphere invalid() { return {glm::vec3(0), -FLT_MAX}; }

    static BoundingSphere notCullable() { return {glm::vec3(0), FLT_MAX}; }

    inline BoundingSphere movedBy(const glm::vec3& delta) const { return {center + delta, radius}; }

    inline BoundingSphere scaledBy(float delta) const { return {center, radius + delta}; }

    inline bool isInvalid() const { return radius == -FLT_MAX; }

    inline bool isNotCullable() const { return radius == FLT_MAX; }

    inline bool operator==(const BoundingSphere& other) const {
        return center == other.center && radius == other.radius;
    }

    inline bool operator!=(const BoundingSphere& other) const { return !(*this == other); }
};

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
