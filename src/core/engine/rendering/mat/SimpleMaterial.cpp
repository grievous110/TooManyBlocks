#include "engine/rendering/mat/SimpleMaterial.h"

SimpleMaterial::SimpleMaterial(std::shared_ptr<Shader> shader, const glm::vec3 color, std::shared_ptr<Texture> texture)	:
	Material(shader),
	m_color(color),
	m_texture(texture) {}

SimpleMaterial::~SimpleMaterial() {}

bool SimpleMaterial::supportsPass(PassType passType) const {
	return passType == PassType::MainPass;
}

void SimpleMaterial::bindForPass(PassType passType, const RenderContext& context) const {
	if (passType == PassType::MainPass) {
		m_shader->bind();
		m_shader->setUniform("u_color", m_color);
		m_shader->setUniform("u_useTexture", m_texture != nullptr);
		m_shader->setUniform("u_mvp", context.modelViewProjection);
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