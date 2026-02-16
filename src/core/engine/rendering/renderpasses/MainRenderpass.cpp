#include "MainRenderpass.h"

#include <GL/glew.h>

#include <glm/vec2.hpp>

#include "Application.h"
#include "datatypes/Transform.h"
#include "engine/GameInstance.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/Frustum.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/Renderer.h"
#include "engine/rendering/lowlevelapi/FrameBuffer.h"

void MainRenderpass::prepare(RenderContext& context, RenderResources& resources, const ApplicationContext& appContext) {
    FrameBuffer::bindDefault();
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    glm::uvec2 screenRes = context.currScreenRes;
    GLCALL(glViewport(0, 0, screenRes.x, screenRes.y));
    context.tInfo.viewProjection = appContext.instance->m_player->getCamera()->getViewProjMatrix();
    context.tInfo.projection = appContext.instance->m_player->getCamera()->getProjectionMatrix();
    context.tInfo.view = appContext.instance->m_player->getCamera()->getViewMatrix();
    context.tInfo.viewportTransform = appContext.instance->m_player->getCamera()->getGlobalTransform();

    if (m_debugPolygonModeEnabled) {
        GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }
}

void MainRenderpass::execute(RenderContext& context, RenderResources& resources, const ApplicationContext& appContext) {
    m_objectsProcessed = 0;

    cullObjectsOutOfView(*resources.objectsToRender, resources.culledObjectsBuffer, context.tInfo.viewProjection);
    batchByMaterialForPass(resources.culledObjectsBuffer, PassType::MainPass);

    for (const auto& batch : m_materialBatches) {
        batch.first->bindForPass(PassType::MainPass, context);

        for (const Renderable* obj : batch.second) {
            if (const SkeletalMesh* sMesh = dynamic_cast<const SkeletalMesh*>(obj)) {
                context.skInfo.jointMatrices = sMesh->getJointMatrices();
            } else if (const ParticleSystem* pSys = dynamic_cast<const ParticleSystem*>(obj)) {
                context.pInfo.flags = pSys->getFlags();
            }
            context.tInfo.meshTransform = obj->getRenderableTransform();
            batch.first->bindForObjectDraw(PassType::MainPass, context);
            obj->draw();

            m_objectsProcessed++;
        }
    }

    m_materialBatches.clear();
}

void MainRenderpass::cleanup(RenderContext& context, RenderResources& resources, const ApplicationContext& appContext) {
    if (m_debugPolygonModeEnabled) {
        GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }
}

const char* MainRenderpass::name() { return "Main Renderpass"; }

void MainRenderpass::putDebugInfo(DebugReport& report) {
    report.beginGroup(name());
    report.addTimeMs("Processing Time", m_lastRunTimeMs);
    report.addCounter("Objects processed", static_cast<int>(m_objectsProcessed));
    report.endGroup();
}
