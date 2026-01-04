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
    GPULight m_internal;

public:
    Light(LightType type, const glm::vec3& color, float intensity, float range);
    virtual ~Light() = default;

    inline glm::vec3 getColor() const { return m_internal.color; }
    inline float getIntensity() const { return m_internal.intensity; }
    inline float getRange() const { return m_internal.range; }
    inline LightPriority getPriotity() const { return static_cast<LightPriority>(m_internal.priority); }
    inline int getShadowAtlasIndex() const { return m_internal.shadowMapIndex; }

    inline void setColor(const glm::vec3& color) { m_internal.color = color; }
    inline void setIntensity(float intensity) { m_internal.intensity = intensity; }
    inline void setRange(float range) { m_internal.range = range; }
    inline void setPriotity(LightPriority prio) { m_internal.priority = static_cast<unsigned int>(prio); }
    inline void setShadowAtlasIndex(int index) { m_internal.shadowMapIndex = index; }

    virtual GPULight toGPULight();
    virtual LightType getType() const = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;
    virtual glm::mat4 getViewMatrix() const = 0;
    inline glm::mat4 getViewProjMatrix() const { return getProjectionMatrix() * getViewMatrix(); }
};

#endif