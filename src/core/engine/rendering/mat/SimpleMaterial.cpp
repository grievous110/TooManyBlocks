#include "SimpleMaterial.h"
#include "Logger.h"

bool SimpleMaterial::supportsPass(PassType passType) const {
	return passType == PassType::MainPass;
}

void SimpleMaterial::bindForPass(PassType passType, const RenderContext& context) const {
	if (passType == PassType::MainPass) {
		m_shader->bind();
		m_shader->setUniform("u_color", m_color);
		m_shader->setUniform("u_useTexture", m_texture != nullptr);
		if (m_texture) {
			m_texture->bind(0);
			m_shader->setUniform("u_texture", 0);
		}
	} else {
		lgr::lout.error("Material bound for unsupported pass");
	}
}

void SimpleMaterial::bindForMeshDraw(PassType passType, const RenderContext &context) const {
	if (passType == PassType::MainPass) {
		m_shader->bind();
		m_shader->setUniform("u_mvp", context.viewProjection * context.meshTransform.getModelMatrix());
	} else {
		lgr::lout.error("Material bound for unsupported pass");
	}
}
