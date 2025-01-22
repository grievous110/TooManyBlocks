#ifndef SCENEOPTIMIZING_H
#define SCENEOPTIMIZING_H

#include "engine/rendering/Mesh.h"
#include "engine/env/lights/Light.h"
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

std::vector<std::shared_ptr<Mesh>> cullMeshesOutOfView(const std::vector<std::shared_ptr<Mesh>>& meshes, const glm::mat4& viewProj);

std::vector<std::shared_ptr<Light>> prioritizeLights(const std::vector<std::shared_ptr<Light>>& lights, const RenderContext& context);

#endif