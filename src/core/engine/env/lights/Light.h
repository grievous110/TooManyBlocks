#ifndef LIGHT_H
#define LIGHT_H

#include <engine/comp/SceneComponent.h>
#include <glm/glm.hpp>

class Light : public SceneComponent {
protected:
    glm::vec3 m_color;
    float m_intensity;
    float m_range;

public:
    Light(const glm::vec3& color, float intensity, float range) : m_color(color), m_intensity(intensity), m_range(range) {}
    virtual ~Light() = default;

    inline glm::vec3 getColor() const { return m_color; }
    inline float getRange() const { return m_range; }
    inline float getIntensity() const { return m_intensity; }

    inline void setColor(const glm::vec3& color) { m_color = color; }
    inline void setIntensity(float intensity) { m_intensity = intensity; }
    inline void setRange(float range) { m_range = range; }

    virtual glm::mat4 getProjectionMatrix() const = 0;
    virtual glm::mat4 getViewMatrix() const = 0;
};

#endif