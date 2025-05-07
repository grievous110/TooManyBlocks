#include "ChunkMaterial.h"

#include <GL/glew.h>

#include <sstream>

#include "Logger.h"
#include "engine/env/lights/Spotlight.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/Renderer.h"

bool ChunkMaterial::supportsPass(PassType passType) const {
    return passType == PassType::ShadowPass || passType == PassType::AmbientOcclusion || passType == PassType::MainPass;
}

void ChunkMaterial::bindForPass(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        if (m_mainShader) {
            m_mainShader->bind();
            m_mainShader->setUniform("u_viewProjection", context.tInfo.viewProjection);
            m_mainShader->setUniform("u_cameraPosition", context.tInfo.viewportTransform.getPosition());

            // Pass texture data
            if (m_textureAtlas) {
                m_textureAtlas->bind(0);
                m_mainShader->setUniform("u_textureAtlas", 0);
                m_mainShader->setUniform("u_textureAtlasSize", m_textureAtlas->width());
                m_mainShader->setUniform("u_textureSize", 16u);
            } else {
                lgr::lout.error("Texture atlas not loaded for ChunkMaterial");
            }

            // Pass light info
            m_mainShader->setUniform("u_lightCount", static_cast<int>(context.lInfo.activeLightsCount));
            m_mainShader->setAndBindUBO("LightViewProjBlock", *context.lInfo.lightBuff, 0);
            m_mainShader->setAndBindUBO("LightViewProjBlock", *context.lInfo.lightViewProjectionBuff, 1);

            if (context.ssaoInfo.output) {
                context.ssaoInfo.output->bind(1);
                m_mainShader->setUniform("u_ssaoTexture", 1);
            }
            m_mainShader->setUniform("u_screenResolution", context.currScreenRes);

            // Pass depth buffers for shadowmapping
            for (int prio = 0; prio < LightPriority::Count; prio++) {
                if (Texture* shadowAtlas = context.lInfo.shadowMapAtlases[prio]) {
                    const std::string strPrio = std::to_string(prio);
                    shadowAtlas->bind(prio + 2);
                    m_mainShader->setUniform("u_shadowMapAtlas[" + strPrio + "]", prio + 2);
                    m_mainShader->setUniform("u_shadowMapAtlasSizes[" + strPrio + "]", shadowAtlas->width());
                    m_mainShader->setUniform("u_shadowMapSizes[" + strPrio + "]", context.lInfo.shadowMapSizes[prio]);
                } else {
                    lgr::lout.error("Shadow map atlas for prio " + std::to_string(prio) + " not loaded for ChunkMaterial");
                }
            }
        }
    } else if (passType == PassType::ShadowPass) {
        if (m_depthShader) {
            m_depthShader->bind();
            m_depthShader->setUniform("u_viewProjection", context.tInfo.viewProjection);
        } else {
            lgr::lout.error("Depth shader not loaded for ChunkMaterial");
        }
    } else if (passType == PassType::AmbientOcclusion) {
        if (m_ssaoGBuffShader) {
            m_ssaoGBuffShader->bind();
            m_ssaoGBuffShader->setUniform("u_view", context.tInfo.view);
            m_ssaoGBuffShader->setUniform("u_projection", context.tInfo.projection);
        } else {
            lgr::lout.error("SSAO shader not loaded for ChunkMaterial");
        }
    }
}

void ChunkMaterial::bindForObjectDraw(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        if (m_mainShader) {
            m_mainShader->bind();
            m_mainShader->setUniform("u_chunkPosition", context.tInfo.meshTransform.getPosition());
        } else {
            lgr::lout.error("Main shader not loaded for ChunkMaterial");
        }
    } else if (passType == PassType::ShadowPass) {
        if (m_depthShader) {
            m_depthShader->bind();
            m_depthShader->setUniform("u_chunkPosition", context.tInfo.meshTransform.getPosition());
        } else {
            lgr::lout.error("Depth shader not loaded for ChunkMaterial");
        }
    } else if (passType == PassType::AmbientOcclusion) {
        if (m_ssaoGBuffShader) {
            m_ssaoGBuffShader->bind();
            m_ssaoGBuffShader->setUniform("u_chunkPosition", context.tInfo.meshTransform.getPosition());
        } else {
            lgr::lout.error("SSAO shader not loaded for ChunkMaterial");
        }
    }
}
