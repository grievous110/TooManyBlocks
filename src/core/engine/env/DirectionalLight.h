#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "engine/env/Light.h"

class DirectionalLight : public Light {
protected:
    float m_width;
    float m_height;

public:
    DirectionalLight(const glm::vec3& color, float intensity, float range, float width, float height) : Light(color, intensity, range), m_width(width), m_height(height) {}
    virtual ~DirectionalLight() = default;

    inline float getWidth() const { return m_width; }
    inline float getHeight() const { return m_height; }

    inline void setWidth(float width) { m_width = width; }
    inline void setHeight(float height) { m_height = height; }
 
    glm::mat4 getProjectionMatrix() const override;
    glm::mat4 getViewMatrix() const override;
};

#endif