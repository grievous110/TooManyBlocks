#include "LineMaterial.h"
#include "Logger.h"

bool LineMaterial::supportsPass(PassType passType) const {
	return passType == PassType::MainPass;
}

void LineMaterial::bindForPass(PassType passType, const RenderContext& context) const {
	if (passType == PassType::MainPass) {
		m_shader->bind();
		m_shader->setUniform("u_color", m_color);
	} else {
		lgr::lout.error("Material bound for unsupported pass");
	}
}

void LineMaterial::bindForObjectDraw(PassType passType, const RenderContext &context) const {
	if (passType == PassType::MainPass) {
		m_shader->bind();
		m_shader->setUniform("u_mvp", context.viewProjection * context.meshTransform.getModelMatrix());
	} else {
		lgr::lout.error("Material bound for unsupported pass");
	}
}