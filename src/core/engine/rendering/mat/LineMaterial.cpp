#include "LineMaterial.h"

#include "Logger.h"
#include "engine/rendering/Renderer.h"

bool LineMaterial::supportsPass(PassType passType) const { return passType == PassType::MainPass; }

void LineMaterial::bindForPass(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        m_mainShader->bind();
        m_mainShader->setUniform("u_color", m_color);
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}

void LineMaterial::bindForObjectDraw(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        m_mainShader->bind();
        m_mainShader->setUniform("u_mvp", context.tInfo.viewProjection * context.tInfo.meshTransform.getModelMatrix());
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}