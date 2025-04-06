#ifndef BOUNDINGVOLUME_H
#define BOUNDINGVOLUME_H

#include "datatypes/RawBuffer.h"
#include "engine/env/lights/Light.h"
#include "engine/rendering/Renderable.h"
#include <array>
#include <cfloat>
#include <glm/glm.hpp>

// Axis Aligned Bounding Box
struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;

    static BoundingBox invalid() {
        return { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };
    }
    
    static BoundingBox notCullable() {
        return { glm::vec3(-FLT_MAX), glm::vec3(FLT_MAX) };
    }

    inline glm::vec3 center() const {
        return (min + max) * 0.5f;
    }

    inline bool isInvalid() const {
        return min == glm::vec3(FLT_MAX) && max == glm::vec3(-FLT_MAX);
    }

    inline bool isNotCullable() const {
        return min == glm::vec3(-FLT_MAX) && max == glm::vec3(FLT_MAX); 
    }

    inline bool operator==(const BoundingBox& other) const {
        return min == other.min && max == other.max;
    }

    inline bool operator!=(const BoundingBox& other) const {
        return !(*this == other);
    }
};

class Frustum {
private:
    glm::vec4 planes[6];

public:
    Frustum(const glm::mat4& viewProjMatrix);

    bool isBoxInside(const glm::vec3& min, const glm::vec3& max) const;

    bool isSphereInside(const glm::vec3& center, float radius) const;
};

void cullObjectsOutOfView(const std::vector<Renderable*>& meshes, RawBuffer<Renderable*>& outputBuffer, const glm::mat4& viewProj);

#endif