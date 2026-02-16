#ifndef TOOMANYBLOCKS_LIGHTPROCESSOR_H
#define TOOMANYBLOCKS_LIGHTPROCESSOR_H

#include <array>
#include <glm/glm.hpp>
#include <vector>

#include "engine/env/lights/Light.h"
#include "engine/rendering/lowlevelapi/FrameBuffer.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"

struct RenderContext;

class LightProcessor {
private:
    size_t m_totalSupportedLights;
    std::array<FrameBuffer, LightPriority::Count> m_shadowMapAtlases;
    std::array<unsigned int, LightPriority::Count> m_shadowMapSizes;
    std::array<unsigned int, LightPriority::Count> m_maxShadowMapsPerPriority;

    std::vector<GPULight> m_lightBuffer;
    UniformBuffer m_lightUniformBuffer;

    std::vector<glm::mat4> m_lightViewProjectionBuffer;
    UniformBuffer m_lightViewProjectionUniformBuffer;

public:
    static std::array<unsigned int, LightPriority::Count> prioritizeLights(
        const std::vector<Light*>& lights,
        std::vector<Light*>& outputBuffer,
        const std::array<unsigned int, LightPriority::Count>& maxShadowMapsPerPriority,
        const RenderContext& context
    );

    LightProcessor();

    void clearShadowMaps();

    void prepareShadowPass(const Light* light);

    void updateBuffers(const std::vector<Light*>& activeLights);

    std::array<Texture*, LightPriority::Count> getShadowMapAtlases() const;

    inline size_t totalSupportedLights() const { return m_totalSupportedLights; }

    inline const std::array<unsigned int, LightPriority::Count>& getShadowMapSizes() const { return m_shadowMapSizes; }

    inline const std::array<unsigned int, LightPriority::Count>& getShadowMapCounts() const {
        return m_maxShadowMapsPerPriority;
    }

    inline const UniformBuffer* getShaderLightUniformBuffer() const { return &m_lightUniformBuffer; }

    inline const UniformBuffer* getLightViewProjectionUniformBuffer() const {
        return &m_lightViewProjectionUniformBuffer;
    }
};

#endif
