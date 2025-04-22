#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <glm/glm.hpp>

#include "datatypes/RawBuffer.h"
#include "engine/rendering/Renderable.h"

class Frustum {
private:
    glm::vec4 planes[6];

public:
    Frustum(const glm::mat4& viewProjMatrix);

    bool isBoxInside(const glm::vec3& min, const glm::vec3& max) const;

    bool isSphereInside(const glm::vec3& center, float radius) const;
};

void cullObjectsOutOfView(
    const std::vector<Renderable*>& meshes, RawBuffer<Renderable*>& outputBuffer, const glm::mat4& viewProj
);

#endif