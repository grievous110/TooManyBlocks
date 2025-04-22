#ifndef TOOMANYBLOCKS_LIGHTPROCESSOR_H
#define TOOMANYBLOCKS_LIGHTPROCESSOR_H

#include <array>
#include <glm/glm.hpp>
#include <memory>

#include "datatypes/RawBuffer.h"
#include "engine/env/lights/Light.h"
#include "engine/rendering/lowlevelapi/FrameBuffer.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"

struct RenderContext;

class LightProcessor {
private:
    size_t m_totalSupportedLights;
    std::array<std::unique_ptr<FrameBuffer>, LightPriority::Count> m_shadowMapAtlases;
    std::array<unsigned int, LightPriority::Count> m_shadowMapSizes;
    std::array<unsigned int, LightPriority::Count> m_maxShadowMapsPerPriority;

    RawBuffer<ShaderLightStruct> m_lightBuffer;
    std::shared_ptr<UniformBuffer> m_lightUniformBuffer;

    RawBuffer<glm::mat4> m_lightViewProjectionBuffer;
    std::shared_ptr<UniformBuffer> m_lightViewProjectionUniformBuffer;

    bool isInitialized;

public:
    static void prioritizeLights(
        const std::vector<Light*>& lights,
        RawBuffer<Light*>& outputBuffer,
        const std::array<unsigned int, LightPriority::Count>& maxShadowMapsPerPriority,
        const RenderContext& context
    );

    LightProcessor() : m_totalSupportedLights(0), isInitialized(false) {}

    void initialize();

    void clearShadowMaps();

    void prepareShadowPass(const Light* light);

    void updateBuffers(const RawBuffer<Light*>& activeLights);

    std::array<std::weak_ptr<Texture>, LightPriority::Count> getShadowMapAtlases() const;

    inline size_t totalSupportedLights() const { return m_totalSupportedLights; }

    inline const std::array<unsigned int, LightPriority::Count>& getShadowMapSizes() const { return m_shadowMapSizes; }

    inline const std::array<unsigned int, LightPriority::Count>& getShadowMapCounts() const {
        return m_maxShadowMapsPerPriority;
    }

    inline std::weak_ptr<UniformBuffer> getShaderLightUniformBuffer() const {
        return std::weak_ptr<UniformBuffer>(m_lightUniformBuffer);
    }

    inline std::weak_ptr<UniformBuffer> getLightViewProjectionUniformBuffer() const {
        return std::weak_ptr<UniformBuffer>(m_lightViewProjectionUniformBuffer);
    }
};

#endif