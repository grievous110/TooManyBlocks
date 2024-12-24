#include "engine/rendering/mat/SimpleMaterial.h"

bool SimpleMaterial::supportsPass(PassType passType) const {
	return passType == PassType::MainPass;
}

void SimpleMaterial::bindForPass(PassType passType, const RenderContext& context) const {
	if (passType == PassType::MainPass) {
		m_shader->bind();
		m_shader->setUniform("u_color", m_color);
		m_shader->setUniform("u_useTexture", m_texture != nullptr);
		m_shader->setUniform("u_mvp", context.viewProjection * context.modelMatrix);
		if (m_texture != nullptr) {
			m_texture->bind(0);
			m_shader->setUniform("u_texture", 0);
		}
	}
}

void SimpleMaterial::unbindForPass(PassType passType) const {
	if (passType == PassType::MainPass) {
		m_shader->unbind();
		m_texture->unbind();
	}
}