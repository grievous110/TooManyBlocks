#ifndef FRUSTUMCULLING_H
#define FRUSTUMCULLING_H

#include <glm/glm.hpp>

class Frustum {
private:
    glm::vec4 planes[6];

public:
    Frustum(const glm::mat4& viewProjMatrix);

    bool isBoxInside(const glm::vec3& min, const glm::vec3& max) const;

    bool isSphereInside(const glm::vec3& center, float radius) const;
};

#endif