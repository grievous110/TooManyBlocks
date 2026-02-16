#include "SSAORenderpass.h"

#include <GL/glew.h>

#include <glm/vec2.hpp>

#include "Application.h"
#include "datatypes/Transform.h"
#include "engine/GameInstance.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/Frustum.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/Renderer.h"

void SSAORenderpass::prepare(RenderContext& context, RenderResources& resources, const ApplicationContext& appContext) {
    context.tInfo.viewProjection = appContext.instance->m_player->getCamera()->getViewProjMatrix();
    context.tInfo.projection = appContext.instance->m_player->getCamera()->getProjectionMatrix();
    context.tInfo.view = appContext.instance->m_player->getCamera()->getViewMatrix();
    context.tInfo.viewportTransform = appContext.instance->m_player->getCamera()->getGlobalTransform();
    m_ssaoProcessor.validateBuffers(appContext);  // Possible resize sssao textures if resize happened
    // Disable blending if enabled here cause rendering to textures that do not have 4 channels
    // (alpha) will will be discarded. Blending expects a valid alpha component
    // GLCALL(glDisable(GL_BLEND));
}

void SSAORenderpass::execute(RenderContext& context, RenderResources& resources, const ApplicationContext& appContext) {
    cullObjectsOutOfView(*resources.objectsToRender, resources.culledObjectsBuffer, context.tInfo.viewProjection);
    batchByMaterialForPass(resources.culledObjectsBuffer, PassType::AmbientOcclusion);

    m_objectsProcessed = 0;
    if (!m_materialBatches.empty()) {
        m_ssaoProcessor.prepareSSAOGBufferPass(appContext);

        for (const auto& batch : m_materialBatches) {
            batch.first->bindForPass(PassType::AmbientOcclusion, context);

            for (const Renderable* obj : batch.second) {
                context.tInfo.meshTransform = obj->getRenderableTransform();
                batch.first->bindForObjectDraw(PassType::AmbientOcclusion, context);
                obj->draw();

                m_objectsProcessed++;
            }
        }

        m_ssaoProcessor.prepareSSAOPass(appContext);
        appContext.renderer->drawFullscreenQuad();

        m_ssaoProcessor.prepareSSAOBlurPass(appContext);
        appContext.renderer->drawFullscreenQuad();
    }

    m_materialBatches.clear();
}

void SSAORenderpass::cleanup(RenderContext& context, RenderResources& resources, const ApplicationContext& appContext) {
    // GLCALL(glEnable(GL_BLEND));
    context.ssaoInfo.output = m_ssaoProcessor.getOcclusionOutput();
}

const char* SSAORenderpass::name() { return "SSAO Renderpass"; }

void SSAORenderpass::putDebugInfo(DebugReport& report) {
    report.beginGroup(name());
    report.addTimeMs("Processing Time", m_lastRunTimeMs);
    report.addCounter("Objects processed", static_cast<int>(m_objectsProcessed));
    report.endGroup();
}
