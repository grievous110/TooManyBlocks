#include "ChunkMaterial.h"
#include "engine/env/lights/Spotlight.h"
#include "Logger.h"

bool ChunkMaterial::supportsPass(PassType passType) const {
	return passType == PassType::MainPass || passType == PassType::ShadowPass;
}

void ChunkMaterial::bindForPass(PassType passType, const RenderContext& context) const {
	if (passType == PassType::MainPass) {
		m_shader->bind();
		m_shader->setUniform("u_viewProjection", context.viewProjection);
		m_shader->setUniform("u_chunkPosition", context.meshTransform.getPosition());
		m_shader->setUniform("u_cameraPosition", context.viewportTransform.getPosition());
		
		// Pass texture data
		if (m_textureAtlas) {
			m_textureAtlas->bind(0);
			m_shader->setUniform("u_textureAtlas", 0);
			m_shader->setUniform("u_textureAtlasSize", static_cast<unsigned int>(m_textureAtlas->width()));
			m_shader->setUniform("u_textureSize", 16u);
		} else {
			lgr::lout.error("Texture atlas not loaded for ChunkMaterial");
		}

		// Pass light info
		int activeLightCount = std::min<int>(context.lights.size(), 10);
		m_shader->setUniform("u_lightCount", activeLightCount);
		for (int i = 0; i < activeLightCount; i++) {
			m_shader->setUniform("u_lightViewProjections[" + std::to_string(i) + "]", context.lights[i]->getViewProjMatrix());
			const std::string lightvar = "u_lights[" + std::to_string(i) + "]";
			Transform lightTr = context.lights[i]->getGlobalTransform();
			
			m_shader->setUniform(lightvar + ".lightType", static_cast<unsigned int>(context.lights[i]->getType()));
			m_shader->setUniform(lightvar + ".priority", static_cast<unsigned int>(context.lights[i]->getPriotity()));
			m_shader->setUniform(lightvar + ".shadowMapIndex", static_cast<unsigned int>(context.lights[i]->getShadowAtlasIndex()));
			m_shader->setUniform(lightvar + ".lightPosition", lightTr.getPosition());
			m_shader->setUniform(lightvar + ".direction", lightTr.getForward());
			m_shader->setUniform(lightvar + ".color", context.lights[i]->getColor());
			m_shader->setUniform(lightvar + ".intensity", context.lights[i]->getIntensity());

			if (context.lights[i]->getType() == LightType::Directional) {
				// Nothing specific
			} else if (context.lights[i]->getType() == LightType::Spot) {
				Spotlight* spotlight = dynamic_cast<Spotlight*>(context.lights[i].get());
				m_shader->setUniform(lightvar + ".direction", lightTr.getForward());
				m_shader->setUniform(lightvar + ".range", context.lights[i]->getRange());
				m_shader->setUniform(lightvar + ".fovy", spotlight->getFovy());
				m_shader->setUniform(lightvar + ".innerCutoffAngle", spotlight->getInnerCutoffAngle());
			} else if (context.lights[i]->getType() == LightType::Point) {
				m_shader->setUniform(lightvar + ".range", context.lights[i]->getRange());
			}
		}

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
	} else if (passType == PassType::ShadowPass) {
		if (m_depthShader) {
			m_depthShader->bind();
			m_depthShader->setUniform("u_lightViewProjection", context.viewProjection);
			m_depthShader->setUniform("u_chunkPosition", context.meshTransform.getPosition());
		} else {
			lgr::lout.error("Depth shader not loaded for ChunkMaterial");
		}
	}
}

void ChunkMaterial::unbindForPass(PassType passType) const {
	if (passType == PassType::MainPass) {
		m_shader->unbind();
		m_textureAtlas->unbind();
	} else if (passType == PassType::ShadowPass) {
		m_depthShader->unbind();
	}
}