#include "Spotlight.h"

LightType Spotlight::getType() const { return LightType::Spot; }

glm::mat4 Spotlight::getProjectionMatrix() const { return glm::perspective(glm::radians(m_fovy), 1.0f, 0.1f, m_range); }

glm::mat4 Spotlight::getViewMatrix() const {
    Transform tr = getGlobalTransform();
    glm::vec3 position = tr.getPosition();

    return glm::lookAt(position, position + tr.getForward(), tr.getUp());
}
