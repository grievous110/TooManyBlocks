#include "ShadowRenderpass.h"

#include <GL/glew.h>

#include <glm/vec2.hpp>

#include "Application.h"
#include "datatypes/Transform.h"
#include "engine/GameInstance.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/Frustum.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/Renderer.h"

void ShadowRenderpass::prepare(RenderContext& context, RenderResources& resources, const ApplicationContext& appContext) {
    m_lightProcessor.clearShadowMaps();

    context.tInfo.viewProjection = appContext.instance->m_player->getCamera()->getViewProjMatrix();
    context.tInfo.viewportTransform = appContext.instance->m_player->getCamera()->getGlobalTransform();
    m_lastLightCountPerPrio = LightProcessor::prioritizeLights(
        *resources.lightsToRender, resources.priodLightsBuffer, m_lightProcessor.getShadowMapCounts(), context
    );
    context.lInfo.activeLightsCount = resources.priodLightsBuffer.size();
    m_lightProcessor.updateBuffers(resources.priodLightsBuffer);
}

void ShadowRenderpass::execute(RenderContext& context, RenderResources& resources, const ApplicationContext& appContext) {
    m_objectsProcessed = 0;

    for (int i = 0; i < resources.priodLightsBuffer.size(); i++) {
        const Light* light = resources.priodLightsBuffer[i];
        context.lInfo.currentLightPrio = light->getPriotity();
        context.lInfo.lightShadowAtlasIndex = light->getShadowAtlasIndex();
        context.tInfo.viewProjection = light->getViewProjMatrix();
        context.tInfo.viewportTransform = light->getGlobalTransform();

        m_lightProcessor.prepareShadowPass(light);

        cullObjectsOutOfView(*resources.objectsToRender, resources.culledObjectsBuffer, context.tInfo.viewProjection);
        batchByMaterialForPass(resources.culledObjectsBuffer, PassType::ShadowPass);

        for (const auto& batch : m_materialBatches) {
            batch.first->bindForPass(PassType::ShadowPass, context);

            for (const Renderable* obj : batch.second) {
                context.tInfo.meshTransform = obj->getRenderableTransform();
                batch.first->bindForObjectDraw(PassType::ShadowPass, context);
                obj->draw();

                m_objectsProcessed++;
            }
        }
        m_materialBatches.clear();
    }
}

void ShadowRenderpass::cleanup(RenderContext& context, RenderResources& resources, const ApplicationContext& appContext) {

}

const char* ShadowRenderpass::name() { return "Shadow Pass"; }

void ShadowRenderpass::putDebugInfo(DebugReport& report) {
    report.beginGroup(name());
    report.addTimeMs("Processing Time", m_lastRunTimeMs);
    report.addCounter("Objects processed", static_cast<int>(m_objectsProcessed));

    int totalLightCount = 0;
    for (int prio = LightPriority::High; prio <= LightPriority::Count; prio++) {
        totalLightCount += static_cast<int>(m_lastLightCountPerPrio[prio]);
    }

    report.addCounter("Lights processed", static_cast<int>(totalLightCount));
    report.addCounter("- [HIGH] Prio Lights", static_cast<int>(m_lastLightCountPerPrio[LightPriority::High]));
    report.addCounter("- [MEDIUM] Prio Lights", static_cast<int>(m_lastLightCountPerPrio[LightPriority::Medium]));
    report.addCounter("- [LOW] Prio Lights", static_cast<int>(m_lastLightCountPerPrio[LightPriority::Low]));
    report.endGroup();
}
