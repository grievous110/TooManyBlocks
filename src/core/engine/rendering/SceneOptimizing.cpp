#include "SceneOptimizing.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include <algorithm>
#include <array>
#include <cfloat>

struct ScoredLight {
    Light* light;
    float score;
};

Frustum::Frustum(const glm::mat4& viewProjMatrix) {
    enum Planes { Near, Far, Left, Right, Top, Bottom };
    // Extract planes from the combined projection-view viewProjMatrix
    planes[Left]   = glm::vec4(viewProjMatrix[0][3] + viewProjMatrix[0][0], viewProjMatrix[1][3] + viewProjMatrix[1][0], viewProjMatrix[2][3] + viewProjMatrix[2][0], viewProjMatrix[3][3] + viewProjMatrix[3][0]);
    planes[Right]  = glm::vec4(viewProjMatrix[0][3] - viewProjMatrix[0][0], viewProjMatrix[1][3] - viewProjMatrix[1][0], viewProjMatrix[2][3] - viewProjMatrix[2][0], viewProjMatrix[3][3] - viewProjMatrix[3][0]);
    planes[Bottom] = glm::vec4(viewProjMatrix[0][3] + viewProjMatrix[0][1], viewProjMatrix[1][3] + viewProjMatrix[1][1], viewProjMatrix[2][3] + viewProjMatrix[2][1], viewProjMatrix[3][3] + viewProjMatrix[3][1]);
    planes[Top]    = glm::vec4(viewProjMatrix[0][3] - viewProjMatrix[0][1], viewProjMatrix[1][3] - viewProjMatrix[1][1], viewProjMatrix[2][3] - viewProjMatrix[2][1], viewProjMatrix[3][3] - viewProjMatrix[3][1]);
    planes[Near]   = glm::vec4(viewProjMatrix[0][3] + viewProjMatrix[0][2], viewProjMatrix[1][3] + viewProjMatrix[1][2], viewProjMatrix[2][3] + viewProjMatrix[2][2], viewProjMatrix[3][3] + viewProjMatrix[3][2]);
    planes[Far]    = glm::vec4(viewProjMatrix[0][3] - viewProjMatrix[0][2], viewProjMatrix[1][3] - viewProjMatrix[1][2], viewProjMatrix[2][3] - viewProjMatrix[2][2], viewProjMatrix[3][3] - viewProjMatrix[3][2]);

    // Normalize the planes
    for (int i = 0; i < 6; i++) {
        planes[i] /= glm::length(glm::vec3(planes[i]));
    }
}

bool Frustum::isBoxInside(const glm::vec3& min, const glm::vec3& max) const {
    for (int i = 0; i < 6; i++) {
        const glm::vec4& plane = planes[i];

        // Check if all corners of the bounding box are outside the plane
        glm::vec3 positiveCorner = glm::vec3(
            (plane.x > 0) ? max.x : min.x,
            (plane.y > 0) ? max.y : min.y,
            (plane.z > 0) ? max.z : min.z
        );

        if (glm::dot(glm::vec3(plane), positiveCorner) + plane.w < 0) {
            return false; // Box is outside the frustum
        }
    }
    return true; // Box is inside or intersects the frustum
}

bool Frustum::isSphereInside(const glm::vec3& center, float radius) const {
    for (int i = 0; i < 6; i++) {
        if (glm::dot(glm::vec3(planes[i]), center) + planes[i].w < -radius) {
            return false;
        }
    }
    return true;
}

bool isInvalidMeshBounds(const MeshBounds &bounds) {
    return glm::vec3(FLT_MAX) == bounds.min && glm::vec3(-FLT_MAX) == bounds.max;
}

void cullMeshesOutOfView(const std::vector<std::shared_ptr<Mesh>>& meshes, RawBuffer<Mesh*>& outputBuffer, const glm::mat4& viewProj) {
    const Frustum frustum(viewProj);

    outputBuffer.clear();
	for (const auto& mesh : meshes) {
        const MeshBounds bounds = mesh->getMeshBounds();
        if (!isInvalidMeshBounds(bounds)) {
            glm::vec3 pos = mesh->getGlobalTransform().getPosition();
            if (frustum.isBoxInside(bounds.min + pos, bounds.max + pos)) {
                outputBuffer.push_back(mesh.get());
            }
        }
	}
}

void prioritizeLights(const std::vector<std::shared_ptr<Light>>& lights, RawBuffer<Light*>& outputBuffer, const std::array<unsigned int, LightPriority::Count>& maxShadowMapsPerPriority, const RenderContext& context) {
    const Frustum cameraFrustum(context.viewProjection);

    // Calculate scores for lights
    glm::vec3 cameraPosition = context.viewportTransform.getPosition();
    std::vector<std::pair<Light*, float>> scoredLights;
    for (const auto& light : lights) {
        if (!cameraFrustum.isSphereInside(light->getGlobalTransform().getPosition(), light->getRange())) {
            // Skip lights outside the cameras view frustum. TODO: isSphereInside() method might be inaccurate for directional lights
            continue;
        }
        float distance = glm::distance(cameraPosition, light->getGlobalTransform().getPosition());
        float score = (1.0f / (distance + 1.0f)) * light->getRange() * light->getIntensity();
        scoredLights.emplace_back(light.get(), score);
    }

    // Sort lights by score in descending order
    std::sort(scoredLights.begin(), scoredLights.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    // Assign priorities and shadow atlas indices
    std::array<unsigned int, LightPriority::Count> currentShadowMapCounts = {0, 0, 0};

    outputBuffer.clear();
    for (const auto& sLight : scoredLights) {
        for (int prio = LightPriority::High; prio <= LightPriority::Low; prio++) {
            if (currentShadowMapCounts[prio] < maxShadowMapsPerPriority[prio]) {
                sLight.first->setPriotity(static_cast<LightPriority>(prio));
                sLight.first->setShadowAtlasIndex(currentShadowMapCounts[prio]++);
                outputBuffer.push_back(sLight.first);
                break;
            }
        }
    }
}
