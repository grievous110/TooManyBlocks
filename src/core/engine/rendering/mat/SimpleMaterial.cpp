#include "SimpleMaterial.h"

#include "Logger.h"
#include "engine/rendering/Renderer.h"

bool SimpleMaterial::supportsPass(PassType passType) const { return passType == PassType::MainPass; }

void SimpleMaterial::bindForPass(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        m_mainShader->bind();
        m_mainShader->setUniform("u_color", m_color);
        m_mainShader->setUniform("u_useTexture", m_texture != nullptr);
        if (m_texture) {
            m_texture->bind(0);
            m_mainShader->setUniform("u_texture", 0);
        }
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}

void SimpleMaterial::bindForObjectDraw(PassType passType, const RenderContext& context) const {
    if (passType == PassType::MainPass) {
        m_mainShader->bind();
        m_mainShader->setUniform("u_mvp", context.tInfo.viewProjection * context.tInfo.meshTransform.getModelMatrix());
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}
