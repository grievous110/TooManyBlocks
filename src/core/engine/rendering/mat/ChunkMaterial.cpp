#include "ChunkMaterial.h"

#include <GL/glew.h>

#include <sstream>

#include "Logger.h"
#include "engine/env/lights/Spotlight.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/Renderer.h"

bool ChunkMaterial::isReady() const {
    return m_mainShader.isReady() && m_depthShader.isReady() && m_ssaoGBuffShader.isReady() && m_textureAtlas.isReady();
}

bool ChunkMaterial::supportsPass(PassType passType) const {
    return passType == PassType::ShadowPass || passType == PassType::AmbientOcclusion || passType == PassType::MainPass;
}

void ChunkMaterial::bindForPass(PassType passType, const RenderContext& context) {
    if (passType == PassType::MainPass) {
        Shader& mainShader = m_mainShader.value();

        mainShader.use();
        mainShader.setUniform("u_viewProjection", context.tInfo.viewProjection);
        mainShader.setUniform("u_cameraPosition", context.tInfo.viewportTransform.getPosition());

        // Pass texture data
        if (m_textureAtlas.isReady()) {
            m_textureAtlas.value().bindToUnit(0);
            mainShader.setUniform("u_textureAtlas", 0);
            mainShader.setUniform("u_textureAtlasSize", m_textureAtlas.value().width());
            mainShader.setUniform("u_textureSize", 16u);
        } else {
            lgr::lout.error("Texture atlas not loaded for ChunkMaterial");
        }

        // Pass light info
        mainShader.setUniform("u_lightCount", static_cast<int>(context.lInfo.activeLightsCount));
        mainShader.bindUniformBuffer("LightsBlock", *context.lInfo.lightBuff);
        mainShader.bindUniformBuffer("LightViewProjBlock", *context.lInfo.lightViewProjectionBuff);

        if (context.ssaoInfo.output) {
            context.ssaoInfo.output->bindToUnit(1);
            mainShader.setUniform("u_ssaoTexture", 1);
        }
        mainShader.setUniform("u_screenResolution", context.currScreenRes);

        // Pass depth buffers for shadowmapping
        for (int prio = 0; prio < LightPriority::Count; prio++) {
            if (const Texture* shadowAtlas = context.lInfo.shadowMapAtlases[prio]) {
                const std::string strPrio = std::to_string(prio);
                shadowAtlas->bindToUnit(prio + 2);
                mainShader.setUniform("u_shadowMapAtlas[" + strPrio + "]", prio + 2);
                mainShader.setUniform("u_shadowMapAtlasSizes[" + strPrio + "]", shadowAtlas->width());
                mainShader.setUniform("u_shadowMapSizes[" + strPrio + "]", context.lInfo.shadowMapSizes[prio]);
            } else {
                lgr::lout.error("Shadow map atlas for prio " + std::to_string(prio) + " not loaded for ChunkMaterial");
            }
        }
    } else if (passType == PassType::ShadowPass) {
        Shader& depthShader = m_depthShader.value();

        depthShader.use();
        depthShader.setUniform("u_viewProjection", context.tInfo.viewProjection);
    } else if (passType == PassType::AmbientOcclusion) {
        Shader& ssaoGBuffShader = m_ssaoGBuffShader.value();
        
        ssaoGBuffShader.use();
        ssaoGBuffShader.setUniform("u_view", context.tInfo.view);
        ssaoGBuffShader.setUniform("u_projection", context.tInfo.projection);
    }
}

void ChunkMaterial::bindForObjectDraw(PassType passType, const RenderContext& context) {
    if (passType == PassType::MainPass) {
        Shader& mainShader = m_mainShader.value();

        mainShader.use();
        mainShader.setUniform("u_chunkPosition", context.tInfo.meshTransform.getPosition());
    } else if (passType == PassType::ShadowPass) {
        Shader& depthShader = m_depthShader.value();

        depthShader.use();
        depthShader.setUniform("u_chunkPosition", context.tInfo.meshTransform.getPosition());
    } else if (passType == PassType::AmbientOcclusion) {
        Shader& ssaoGBuffShader = m_ssaoGBuffShader.value();

        ssaoGBuffShader.use();
        ssaoGBuffShader.setUniform("u_chunkPosition", context.tInfo.meshTransform.getPosition());
    }
}
