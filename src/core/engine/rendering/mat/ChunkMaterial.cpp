#include "ChunkMaterial.h"
#include "engine/env/lights/Spotlight.h"
#include "engine/rendering/GLUtils.h"
#include "Logger.h"
#include <gl/glew.h>

bool ChunkMaterial::supportsPass(PassType passType) const {
	return passType == PassType::MainPass || passType == PassType::ShadowPass;
}

void ChunkMaterial::bindForPass(PassType passType, const RenderContext& context) const {
	if (passType == PassType::MainPass) {
		if (m_shader) {
			m_shader->bind();
			m_shader->setUniform("u_viewProjection", context.viewProjection);
			m_shader->setUniform("u_cameraPosition", context.viewportTransform.getPosition());
			
			// Pass texture data
			if (m_textureAtlas) {
				m_textureAtlas->bind(0);
				m_shader->setUniform("u_textureAtlas", 0);
				m_shader->setUniform("u_textureAtlasSize", m_textureAtlas->width());
				m_shader->setUniform("u_textureSize", 16u);
			} else {
				lgr::lout.error("Texture atlas not loaded for ChunkMaterial");
			}

			// Pass light info
			int activeLightCount = std::min<int>(context.lights.size(), MAX_LIGHTS); // !!! Debug cap !!! TODO: Remove when uniforms buffers are integrated
			m_shader->setUniform("u_lightCount", activeLightCount);
			m_shader->setAndBindUBO("LightViewProjBlock", *context.lightBuff, 0);
			m_shader->setAndBindUBO("LightViewProjBlock", *context.lightViewProjectionBuff, 1);

			// Pass depth buffers for shadowmapping
			for (int prio = 0; prio < LightPriority::Count; prio++) {
				std::shared_ptr<FrameBuffer> frameBuff = context.shadowMapAtlases[prio];
				if (frameBuff) {
					const std::string strPrio = std::to_string(prio);
					frameBuff->getDepthTexture()->bind(prio + 1);	
					m_shader->setUniform("u_shadowMapAtlas[" + strPrio + "]", prio + 1);
					m_shader->setUniform("u_shadowMapAtlasSizes[" + strPrio + "]", static_cast<unsigned int>(frameBuff->getDepthTexture()->width()));
					m_shader->setUniform("u_shadowMapSizes[" + strPrio + "]", context.shadowMapSizes[prio]);
				} else {
					lgr::lout.error("Shadow map atlas not loaded for ChunkMaterial");
				}
			}
		}
	} else if (passType == PassType::ShadowPass) {
		if (m_depthShader) {
			m_depthShader->bind();
			m_depthShader->setUniform("u_lightViewProjection", context.viewProjection);
		} else {
			lgr::lout.error("Depth shader not loaded for ChunkMaterial");
		}
	}
}

void ChunkMaterial::bindForMeshDraw(PassType passType, const RenderContext &context) const {
	if (passType == PassType::MainPass) {
		if (m_shader) {
			m_shader->bind();
			m_shader->setUniform("u_chunkPosition", context.meshTransform.getPosition());
		} else {
			lgr::lout.error("Main shader not loaded for ChunkMaterial");
		}
	} else if (passType == PassType::ShadowPass) {
		if (m_depthShader) {
			m_depthShader->bind();
			m_depthShader->setUniform("u_chunkPosition", context.meshTransform.getPosition());
		} else {
			lgr::lout.error("Depth shader not loaded for ChunkMaterial");
		}
	}
}
