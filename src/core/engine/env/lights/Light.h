#ifndef TOOMANYBLOCKS_LIGHT_H
#define TOOMANYBLOCKS_LIGHT_H

#include <engine/comp/SceneComponent.h>

#include <glm/glm.hpp>

#define MAX_LIGHTS 84  // TODO: Replace with dynamic stuff

enum LightPriority {
    High = 0,
    Medium,
    Low,
    Count
};

enum class LightType : uint8_t {
    Directional,
    Spot,
    Point
};

struct alignas(16) GPULight {
    unsigned int lightType;
    unsigned int priority;
    unsigned int shadowMapIndex;
    float intensity;
    glm::vec3 lightPosition;
    float range;                    // Used by point- / spotlight
    glm::vec3 direction;
    float fovy;                     // Used by spotlicht
    glm::vec3 color;
    float innerCutoffAngle;         // Used by spotlicht
};

class Light : public SceneComponent {
protected:
    glm::vec3 m_color;
    float m_intensity;
    float m_range;
    LightPriority m_priority;
    int m_shadowAtlasIndex;

public:
    Light(const glm::vec3& color, float intensity, float range)
        : m_color(color), m_intensity(intensity), m_range(range) {}
    virtual ~Light() = default;

    inline glm::vec3 getColor() const { return m_color; }
    inline float getIntensity() const { return m_intensity; }
    inline float getRange() const { return m_range; }
    inline LightPriority getPriotity() const { return m_priority; }
    inline int getShadowAtlasIndex() const { return m_shadowAtlasIndex; }

    inline void setColor(const glm::vec3& color) { m_color = color; }
    inline void setIntensity(float intensity) { m_intensity = intensity; }
    inline void setRange(float range) { m_range = range; }
    inline void setPriotity(LightPriority prio) { m_priority = prio; }
    inline void setShadowAtlasIndex(int index) { m_shadowAtlasIndex = index; }

    virtual LightType getType() const = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;
    virtual glm::mat4 getViewMatrix() const = 0;
    inline glm::mat4 getViewProjMatrix() const { return getProjectionMatrix() * getViewMatrix(); }
};

#endif