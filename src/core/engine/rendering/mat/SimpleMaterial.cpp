#include "SimpleMaterial.h"

#include "Logger.h"
#include "engine/rendering/Renderer.h"

bool SimpleMaterial::isReady() const { return m_mainShader.isReady(); }

bool SimpleMaterial::supportsPass(PassType passType) const { return passType == PassType::MainPass; }

void SimpleMaterial::bindForPass(PassType passType, const RenderContext& context) {
    if (passType == PassType::MainPass) {
        Shader& mainShader = m_mainShader.value();

        mainShader.use();
        mainShader.setUniform("u_color", m_color);
        mainShader.setUniform("u_useTexture", m_texture.isReady());
        if (m_texture.isReady()) {
            m_texture.value().bindToUnit(0);
            mainShader.setUniform("u_texture", 0);
        }
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}

void SimpleMaterial::bindForObjectDraw(PassType passType, const RenderContext& context) {
    if (passType == PassType::MainPass) {
        Shader& mainShader = m_mainShader.value();

        mainShader.use();
        mainShader.setUniform("u_mvp", context.tInfo.viewProjection * context.tInfo.meshTransform.getModelMatrix());
    } else {
        lgr::lout.error("Material bound for unsupported pass");
    }
}
