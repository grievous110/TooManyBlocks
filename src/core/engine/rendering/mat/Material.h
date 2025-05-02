#ifndef TOOMANYBLOCKS_MATERIAL_H
#define TOOMANYBLOCKS_MATERIAL_H

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "datatypes/RawBuffer.h"
#include "datatypes/Transform.h"
#include "engine/env/lights/Light.h"
#include "engine/rendering/lowlevelapi/FrameBuffer.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"

struct RenderContext {
    glm::uvec2 currentScreenResolution;
    RawBuffer<Light*> lights;
    std::weak_ptr<UniformBuffer> lightBuff;
    std::weak_ptr<UniformBuffer> lightViewProjectionBuff;
    std::weak_ptr<UniformBuffer> jointMatrices;
    glm::mat4 viewProjection;
    glm::mat4 projection;
    glm::mat4 view;
    Transform viewportTransform;
    Transform meshTransform;

    LightPriority currentLightPrio;
    unsigned int lightShadowAtlasIndex;
    std::array<unsigned int, LightPriority::Count> shadowMapSizes;
    std::array<std::weak_ptr<Texture>, LightPriority::Count> shadowMapAtlases;

    std::weak_ptr<Texture> ssaoOutput;
};

enum PassType {
    ShadowPass,
    AmbientOcclusion,
    MainPass
};

class Material {
public:
    std::shared_ptr<Shader> m_shader;

public:
    Material(std::shared_ptr<Shader> shader = nullptr) : m_shader(shader) {}
    virtual ~Material() = default;

    inline std::shared_ptr<Shader> getShader() const { return m_shader; }

    virtual bool supportsPass(PassType passType) const = 0;
    virtual void bindForPass(PassType passType, const RenderContext& context) const = 0;
    virtual void bindForObjectDraw(PassType passType, const RenderContext& context) const = 0;
};

#endif