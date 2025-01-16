#include "engine/rendering/mat/ChunkMaterial.h"
#include "Logger.h"

bool ChunkMaterial::supportsPass(PassType passType) const {
	return passType == PassType::MainPass;
}

void ChunkMaterial::bindForPass(PassType passType, const RenderContext& context) const {
	if (passType == PassType::MainPass) {
		m_shader->bind();
		m_shader->setUniform("u_viewProjection", context.viewProjection);
		m_shader->setUniform("u_chunkPosition", context.meshTransform.getPosition());
		m_shader->setUniform("u_cameraPosition", context.cameraTransform.getPosition());
		if (m_textureAtlas) {
			m_textureAtlas->bind(0);
			m_shader->setUniform("u_textureAtlas", 0);
			m_shader->setUniform("u_textureAtlasSize", static_cast<unsigned int>(m_textureAtlas->width()));
			m_shader->setUniform("u_textureSize", 16u);
		} else {
			lgr::lout.error("Texture atlas not loaded for ChunkMaterial");
		}
	}
}

void ChunkMaterial::unbindForPass(PassType passType) const {
	if (passType == PassType::MainPass) {
		m_shader->unbind();
		m_textureAtlas->unbind();
	}
}