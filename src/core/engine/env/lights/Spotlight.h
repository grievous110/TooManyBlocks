#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "engine/env/lights/Light.h"
#include <algorithm>

class Spotlight : public Light {
protected:
    float m_fovy;
    float m_innerCutoffAngle;

public:
    Spotlight(const glm::vec3& color, float intensity, float fovy, float range) : Light(color, intensity, range), m_fovy(fovy), m_innerCutoffAngle(fovy) {}
    virtual ~Spotlight() = default;

    inline float getFovy() const { return m_fovy; }
    inline float getInnerCutoffAngle() const { return m_innerCutoffAngle; }

    inline void setFovy(float fovy) { m_fovy = fovy; m_innerCutoffAngle = std::min<float>(m_innerCutoffAngle, m_fovy); }
    inline void setInnerCutoffAngle(float angle) { m_innerCutoffAngle = std::min<float>(angle, m_fovy); }

    LightType getType() const override;
    glm::mat4 getProjectionMatrix() const override;
    glm::mat4 getViewMatrix() const override;
};

#endif