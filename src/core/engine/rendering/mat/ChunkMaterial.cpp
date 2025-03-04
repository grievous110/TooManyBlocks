#include "ChunkMaterial.h"
#include "engine/env/lights/Spotlight.h"
#include "engine/rendering/GLUtils.h"
#include "Logger.h"
#include <gl/glew.h>
#include <sstream>

bool ChunkMaterial::supportsPass(PassType passType) const {
	return passType == PassType::ShadowPass || passType == PassType::AmbientOcclusion || passType == PassType::MainPass;
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
			m_shader->setUniform("u_lightCount", static_cast<int>(context.lights.size()));
			m_shader->setAndBindUBO("LightViewProjBlock", *context.lightBuff.lock(), 0);
			m_shader->setAndBindUBO("LightViewProjBlock", *context.lightViewProjectionBuff.lock(), 1);

			if (std::shared_ptr<Texture> ssaoTexture = context.ssaoOutput.lock()) {
				ssaoTexture->bind(1);
				m_shader->setUniform("u_ssaoTexture", 1);
			}
			m_shader->setUniform("u_screenResolution", context.currentScreenResolution);

			// Pass depth buffers for shadowmapping
			for (int prio = 0; prio < LightPriority::Count; prio++) {
				if(std::shared_ptr<Texture> shadowMapAtlas = context.shadowMapAtlases[prio].lock()) {
					const std::string strPrio = std::to_string(prio);
					shadowMapAtlas->bind(prio + 2);
					m_shader->setUniform("u_shadowMapAtlas[" + strPrio + "]", prio + 2);
					m_shader->setUniform("u_shadowMapAtlasSizes[" + strPrio + "]", shadowMapAtlas->width());
					m_shader->setUniform("u_shadowMapSizes[" + strPrio + "]", context.shadowMapSizes[prio]);
				} else {
					lgr::lout.error("Shadow map atlas not loaded for ChunkMaterial");
				}
			}
		}
	}  else if (passType == PassType::ShadowPass) {
		if (m_depthShader) {
			m_depthShader->bind();
			m_depthShader->setUniform("u_viewProjection", context.viewProjection);
		} else {
			lgr::lout.error("Depth shader not loaded for ChunkMaterial");
		}
	} else if (passType == PassType::AmbientOcclusion) {
		if (m_ssaoGBuffShader) {
			m_ssaoGBuffShader->bind();
			m_ssaoGBuffShader->setUniform("u_view", context.view);
			m_ssaoGBuffShader->setUniform("u_projection", context.projection);
		} else {
			lgr::lout.error("SSAO shader not loaded for ChunkMaterial");
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
	} else if (passType == PassType::AmbientOcclusion) {
		if (m_ssaoGBuffShader) {
			m_ssaoGBuffShader->bind();
			m_ssaoGBuffShader->setUniform("u_chunkPosition", context.meshTransform.getPosition());
		} else {
			lgr::lout.error("SSAO shader not loaded for ChunkMaterial");
		}
	}
}
