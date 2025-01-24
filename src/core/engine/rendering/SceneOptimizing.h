#ifndef SCENEOPTIMIZING_H
#define SCENEOPTIMIZING_H

#include "datatypes/RawBuffer.h"
#include "engine/rendering/Mesh.h"
#include "engine/env/lights/Light.h"
#include <array>
#include <glm/glm.hpp>

class Frustum {
private:
    glm::vec4 planes[6];

public:
    Frustum(const glm::mat4& viewProjMatrix);

    bool isBoxInside(const glm::vec3& min, const glm::vec3& max) const;

    bool isSphereInside(const glm::vec3& center, float radius) const;
};

bool isInvalidMeshBounds(const MeshBounds& bounds);

void cullMeshesOutOfView(const std::vector<std::shared_ptr<Mesh>>& meshes, RawBuffer<Mesh*>& outputBuffer, const glm::mat4& viewProj);

void prioritizeLights(const std::vector<std::shared_ptr<Light>>& lights, RawBuffer<Light*>& outputBuffer, const std::array<unsigned int, LightPriority::Count>& maxShadowMapsPerPriority, const RenderContext& context);

#endif