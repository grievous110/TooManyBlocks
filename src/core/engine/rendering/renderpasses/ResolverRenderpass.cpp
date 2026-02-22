#include "ResolverRenderpass.h"

#include <GL/glew.h>

#include "AppConstants.h"
#include "Application.h"
#include "engine/rendering/Frustum.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/Renderer.h"
#include "engine/resource/loaders/ShaderLoader.h"
#include "engine/rendering/lowlevelapi/FrameBuffer.h"

void ResolverRenderpass::prepare(
    RenderContext& context,
    RenderResources& resources,
    const ApplicationContext& appContext
) {
    FrameBuffer::bindDefault();
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    glm::uvec2 screenRes = context.currScreenRes;
    GLCALL(glViewport(0, 0, screenRes.x, screenRes.y));
}

void ResolverRenderpass::execute(
    RenderContext& context,
    RenderResources& resources,
    const ApplicationContext& appContext
) {
    m_resolverShader.use();
    context.opaqueInfo.output->bindToUnit(0);
    m_resolverShader.setUniform("u_opaquePassResult", 0);

    context.transparencyInfo.accumOutput->bindToUnit(1);
    m_resolverShader.setUniform("u_transparencyAccumTexture", 1);
    context.transparencyInfo.revealOutput->bindToUnit(2);
    m_resolverShader.setUniform("u_transparencyRevealTexture", 2);

    appContext.renderer->drawFullscreenQuad();
}

void ResolverRenderpass::cleanup(
    RenderContext& context,
    RenderResources& resources,
    const ApplicationContext& appContext
) {}

ResolverRenderpass::ResolverRenderpass() {
    ApplicationContext* context = Application::getContext();
    CPUShader cpuShader = loadShaderFromFile(Res::Shader::RESOLVER, ShaderLoadOption::VertexAndFragment);

    m_resolverShader = Shader::create(cpuShader.vertexShader, cpuShader.fragmentShader);

}

const char* ResolverRenderpass::name() { return "Resolver Renderpass"; }

void ResolverRenderpass::putDebugInfo(DebugReport& report) {
    report.beginGroup(name());
    report.addTimeMs("Processing Time", m_lastRunTimeMs);
    report.endGroup();
}
