#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "engine/env/lights/Light.h"

class Spotlight : public Light {
protected:
    float m_fovy;

public:
    Spotlight(const glm::vec3& color, float intensity, float fovy, float range) : Light(color, intensity, range), m_fovy(fovy) {}
    virtual ~Spotlight() = default;

    inline float getFovy() const { return m_fovy; }

    inline void setFovy(float fovy) { m_fovy = fovy; }

    glm::mat4 getProjectionMatrix() const override;
    glm::mat4 getViewMatrix() const override;
};

#endif