#include "DirectionalLight.h"

glm::mat4 DirectionalLight::getProjectionMatrix() const {
    float halfWidth = m_width / 2.0f;
    float halfHeight = m_height / 2.0f;
    return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0.1f, m_range);
}

glm::mat4 DirectionalLight::getViewMatrix() const {
    Transform tr = getGlobalTransform();
    glm::vec3 position = tr.getPosition();

    return glm::lookAt(position, position + tr.getForward(), tr.getUp());
}