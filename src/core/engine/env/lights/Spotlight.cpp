#include "Spotlight.h"

Spotlight::Spotlight(const glm::vec3& color, float intensity, float fovy, float range)
    : Light(LightType::Spot, color, intensity, range) {
    m_internal.fovy = fovy;
    m_internal.innerCutoffAngle = fovy;
}

LightType Spotlight::getType() const { return LightType::Spot; }

glm::mat4 Spotlight::getProjectionMatrix() const {
    return glm::perspective(glm::radians(m_internal.fovy), 1.0f, 0.1f, m_internal.range);
}

glm::mat4 Spotlight::getViewMatrix() const {
    Transform tr = getGlobalTransform();
    glm::vec3 position = tr.getPosition();

    return glm::lookAt(position, position + tr.getForward(), tr.getUp());
}
