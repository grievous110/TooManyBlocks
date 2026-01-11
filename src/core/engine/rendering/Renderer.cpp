#include "Renderer.h"

#include <GL/glew.h>

#include <chrono>
#include <memory>
#include <sstream>
#include <vector>

#include "Application.h"
#include "Logger.h"
#include "engine/GameInstance.h"
#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/Camera.h"
#include "engine/rendering/Frustum.h"
#include "engine/rendering/GLUtils.h"
#include "engine/rendering/SkeletalMesh.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/particles/ParticleSystem.h"

static constexpr float fullScreenQuadCCW[] = {
    // Position   // UV-Coords
    -1.0f, 1.0f,  0.0f, 1.0f,  // Top-Left
    -1.0f, -1.0f, 0.0f, 0.0f,  // Bottom-Left
    1.0f,  -1.0f, 1.0f, 0.0f,  // Bottom-Right

    -1.0f, 1.0f,  0.0f, 1.0f,  // Top-Left
    1.0f,  -1.0f, 1.0f, 0.0f,  // Bottom-Right
    1.0f,  1.0f,  1.0f, 1.0f   // Top-Right
};

static constexpr float fullScreenQuadCW[] = {
    // Position   // UV-Coords
    1.0f,  -1.0f, 1.0f, 0.0f,  // Bottom-Right
    -1.0f, -1.0f, 0.0f, 0.0f,  // Bottom-Left
    -1.0f, 1.0f,  0.0f, 1.0f,  // Top-Links

    1.0f,  -1.0f, 1.0f, 0.0f,  // Bottom-Right
    -1.0f, 1.0f,  0.0f, 1.0f,  // Top-Left
    1.0f,  1.0f,  1.0f, 1.0f   // Top-Right
};

static inline void batchByMaterialForPass(
    std::unordered_map<Material*, std::vector<Renderable*>>& materialBatches,
    const RawBuffer<Renderable*>& meshBuff,
    PassType type
) {
    materialBatches.clear();
    for (Renderable* mesh : meshBuff) {
        if (mesh->getMaterial()->supportsPass(type)) {
            materialBatches[mesh->getMaterial().get()].push_back(mesh);
        }
    }
}

void Renderer::beginTransformFeedbackPass(const ApplicationContext& context) {
    GLCALL(glEnable(GL_RASTERIZER_DISCARD));
    m_currentRenderContext.tInfo.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
    m_currentRenderContext.tInfo.viewportTransform = context.instance->m_player->getCamera()->getGlobalTransform();
}

void Renderer::endTransformFeedbackPass(const ApplicationContext& context) { GLCALL(glDisable(GL_RASTERIZER_DISCARD)); }

void Renderer::beginShadowpass(const ApplicationContext& context) {
    m_lightProcessor.clearShadowMaps();

    m_currentRenderContext.tInfo.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
    m_currentRenderContext.tInfo.viewportTransform = context.instance->m_player->getCamera()->getGlobalTransform();
    LightProcessor::prioritizeLights(
        m_lightsToRender, m_priodLigthsBuffer, m_lightProcessor.getShadowMapCounts(), m_currentRenderContext
    );
    m_currentRenderContext.lInfo.activeLightsCount = m_priodLigthsBuffer.size();
    m_lightProcessor.updateBuffers(m_priodLigthsBuffer);
}

void Renderer::endShadowpass(const ApplicationContext& context) {}

void Renderer::beginAmbientOcclusionPass(const ApplicationContext& context) {
    m_currentRenderContext.tInfo.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
    m_currentRenderContext.tInfo.projection = context.instance->m_player->getCamera()->getProjectionMatrix();
    m_currentRenderContext.tInfo.view = context.instance->m_player->getCamera()->getViewMatrix();
    m_currentRenderContext.tInfo.viewportTransform = context.instance->m_player->getCamera()->getGlobalTransform();
    m_ssaoProcessor.validateBuffers(context);  // Possible resize sssao textures if resize happened
    // Disable blending if enabled here cause rendering to textures that do not have 4 channels
    // (alpha) will will be discarded. Blending expects a valid alpha component
    // GLCALL(glDisable(GL_BLEND));
}

void Renderer::endAmbientOcclusionPass(const ApplicationContext& context) {
    // GLCALL(glEnable(GL_BLEND));
    m_currentRenderContext.ssaoInfo.output = m_ssaoProcessor.getOcclusionOutput();
}

void Renderer::beginMainpass(const ApplicationContext& context) {
    FrameBuffer::bindDefault();
    GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    glm::uvec2 screenRes = m_currentRenderContext.currScreenRes;
    GLCALL(glViewport(0, 0, screenRes.x, screenRes.y));
    m_currentRenderContext.tInfo.viewProjection = context.instance->m_player->getCamera()->getViewProjMatrix();
    m_currentRenderContext.tInfo.projection = context.instance->m_player->getCamera()->getProjectionMatrix();
    m_currentRenderContext.tInfo.view = context.instance->m_player->getCamera()->getViewMatrix();
    m_currentRenderContext.tInfo.viewportTransform = context.instance->m_player->getCamera()->getGlobalTransform();
}

void Renderer::endMainpass(const ApplicationContext& context) {
    m_lightsToRender.clear();
    m_objectsToRender.clear();
}

void Renderer::initialize() {
    GLEnableDebugging();

    // GLCALL(glPolygonMode(GL_FRONT, GL_LINE)); // Grid View mode
    // GLCALL(glEnable(GL_BLEND)); // Leave disabled for now (Blending will receive dedicated system)
    GLCALL(glEnable(GL_DEPTH_TEST));
    GLCALL(glEnable(GL_CULL_FACE));    // Enable face culling
    GLCALL(glCullFace(GL_BACK));       // Specify that back faces should be culled (not rendered)
    GLCALL(glFrontFace(GL_CW));        // Specify frontfaces as faces with clockwise winding
    GLCALL(glEnable(GL_MULTISAMPLE));  // Enable MSAA
    GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));  // Blending
    GLCALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

    m_lightProcessor.initialize();
    m_priodLigthsBuffer = RawBuffer<Light*>(m_lightProcessor.totalSupportedLights());
    m_currentRenderContext.lInfo.shadowMapAtlases = m_lightProcessor.getShadowMapAtlases();
    m_currentRenderContext.lInfo.shadowMapSizes = m_lightProcessor.getShadowMapSizes();
    m_currentRenderContext.lInfo.lightBuff = m_lightProcessor.getShaderLightUniformBuffer();
    m_currentRenderContext.lInfo.lightViewProjectionBuff = m_lightProcessor.getLightViewProjectionUniformBuffer();

    // Initialize SSAO renderer
    m_ssaoProcessor.initialize();

    // Create vertex array / buffer for fullscreen quad
    m_fullScreenQuad_vbo = VertexBuffer::create(fullScreenQuadCW, sizeof(fullScreenQuadCW));
    VertexBufferLayout layout;
    layout.push(GL_FLOAT, 2);  // Position
    layout.push(GL_FLOAT, 2);  // Screen UV
    m_fullScreenQuad_vbo.setLayout(layout);
    m_fullScreenQuad_vao = VertexArray::create();
    m_fullScreenQuad_vao.addBuffer(m_fullScreenQuad_vbo);

    FrameBuffer::bindDefault();
}

void Renderer::submitLight(Light* light) { m_lightsToRender.push_back(light); }

void Renderer::submitRenderable(Renderable* obj) { m_objectsToRender.push_back(obj); }

void Renderer::render(const ApplicationContext& context) {
    static auto lastLogTime = std::chrono::high_resolution_clock::now();
    static std::chrono::duration<double> totalTime(0);
    static std::chrono::duration<double> testTime(0);
    static int frameCount = 0;

    auto totalTimerStart = std::chrono::high_resolution_clock::now();

    // Update render context
    m_currentRenderContext.currScreenRes = glm::uvec2(context.state.screenWidth, context.state.screenHeight);
    m_currentRenderContext.deltaTime = context.instance->gameState.deltaTime;
    m_currentRenderContext.elapsedTime = context.instance->gameState.elapsedGameTime;
    // Update camera aspect ratio just in case it changed via resize of screen.
    context.instance->m_player->getCamera()->setAspectRatio(static_cast<float>(context.state.screenWidth) / static_cast<float>(context.state.screenHeight));

    RawBuffer<Renderable*> culledObjectBuffer = RawBuffer<Renderable*>(m_objectsToRender.size());
    std::unordered_map<Material*, std::vector<Renderable*>> materialBatches;

    beginTransformFeedbackPass(context);

    cullObjectsOutOfView(m_objectsToRender, culledObjectBuffer, m_currentRenderContext.tInfo.viewProjection);
    batchByMaterialForPass(materialBatches, culledObjectBuffer, PassType::TransformFeedback);

    for (auto& batch : materialBatches) {
        batch.first->bindForPass(PassType::TransformFeedback, m_currentRenderContext);

        for (Renderable* obj : batch.second) {
            if (ParticleSystem* ps = dynamic_cast<ParticleSystem*>(obj)) {
                ps->switchBuffers();
                m_currentRenderContext.tInfo.meshTransform = ps->getRenderableTransform();
                m_currentRenderContext.pInfo.pModulesBuff = ps->getModulesUBO();
                m_currentRenderContext.pInfo.spawnCount = ps->getSpawnCount();
                m_currentRenderContext.pInfo.particleSpawnOffset = ps->getParticleSpawnOffset();
                m_currentRenderContext.pInfo.allocatedParticleCount = ps->getAllocatedParticleCount();
                m_currentRenderContext.pInfo.flags = ps->getFlags();
                batch.first->bindForObjectDraw(PassType::TransformFeedback, m_currentRenderContext);
                ps->compute();
            }
        }
    }

    endTransformFeedbackPass(context);

    beginShadowpass(context);

    for (int i = 0; i < m_priodLigthsBuffer.size(); i++) {
        const Light* light = m_priodLigthsBuffer[i];
        m_currentRenderContext.lInfo.currentLightPrio = light->getPriotity();
        m_currentRenderContext.lInfo.lightShadowAtlasIndex = light->getShadowAtlasIndex();
        m_currentRenderContext.tInfo.viewProjection = light->getViewProjMatrix();
        m_currentRenderContext.tInfo.viewportTransform = light->getGlobalTransform();

        m_lightProcessor.prepareShadowPass(light);

        cullObjectsOutOfView(m_objectsToRender, culledObjectBuffer, m_currentRenderContext.tInfo.viewProjection);
        batchByMaterialForPass(materialBatches, culledObjectBuffer, PassType::ShadowPass);

        for (const auto& batch : materialBatches) {
            batch.first->bindForPass(PassType::ShadowPass, m_currentRenderContext);

            for (const Renderable* obj : batch.second) {
                m_currentRenderContext.tInfo.meshTransform = obj->getRenderableTransform();
                batch.first->bindForObjectDraw(PassType::ShadowPass, m_currentRenderContext);
                obj->draw();
            }
        }
    }

    endShadowpass(context);

    auto testTimerStart = std::chrono::high_resolution_clock::now();
    beginAmbientOcclusionPass(context);

    cullObjectsOutOfView(m_objectsToRender, culledObjectBuffer, m_currentRenderContext.tInfo.viewProjection);
    batchByMaterialForPass(materialBatches, culledObjectBuffer, PassType::AmbientOcclusion);

    if (!materialBatches.empty()) {
        m_ssaoProcessor.prepareSSAOGBufferPass(context);

        for (const auto& batch : materialBatches) {
            batch.first->bindForPass(PassType::AmbientOcclusion, m_currentRenderContext);

            for (const Renderable* obj : batch.second) {
                m_currentRenderContext.tInfo.meshTransform = obj->getRenderableTransform();
                batch.first->bindForObjectDraw(PassType::AmbientOcclusion, m_currentRenderContext);
                obj->draw();
            }
        }

        m_ssaoProcessor.prepareSSAOPass(context);
        drawFullscreenQuad();

        m_ssaoProcessor.prepareSSAOBlurPass(context);
        drawFullscreenQuad();
    }

    endAmbientOcclusionPass(context);
    testTime += std::chrono::high_resolution_clock::now() - testTimerStart;

    beginMainpass(context);

    // No culling since main pass uses same view
    // cullMeshesOutOfView(scene.meshes, culledMeshBuffer,
    // m_currentRenderContext.viewProjection);
    batchByMaterialForPass(materialBatches, culledObjectBuffer, PassType::MainPass);

    for (const auto& batch : materialBatches) {
        batch.first->bindForPass(PassType::MainPass, m_currentRenderContext);

        for (const Renderable* obj : batch.second) {
            if (const SkeletalMesh* sMesh = dynamic_cast<const SkeletalMesh*>(obj)) {
                m_currentRenderContext.skInfo.jointMatrices = sMesh->getJointMatrices();
            } else if (const ParticleSystem* pSys = dynamic_cast<const ParticleSystem*>(obj)) {
                m_currentRenderContext.pInfo.flags = pSys->getFlags();
            }
            m_currentRenderContext.tInfo.meshTransform = obj->getRenderableTransform();
            batch.first->bindForObjectDraw(PassType::MainPass, m_currentRenderContext);
            obj->draw();
        }
    }

    endMainpass(context);
    totalTime += std::chrono::high_resolution_clock::now() - totalTimerStart;

    // Logging
    frameCount++;
    auto now = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastLogTime).count() >= 1) {
        std::ostringstream msg;
        msg << "Rendering pipeline monitor:" << std::endl;
        msg << "Tested time: " << testTime.count() * 1000.0 / frameCount << "ms (average per frame)" << std::endl;
        msg << "Tested part took " << testTime.count() / totalTime.count() * 100.0 << "% of "
            << totalTime.count() * 1000.0 / frameCount << "ms excution time" << std::endl;
        msg << "Lights processed: " << m_priodLigthsBuffer.size() << std::endl;
        msg << "Objects drawn: " << culledObjectBuffer.size() << std::endl;
        msg << "Batches: " << materialBatches.size();
        lgr::lout.debug(msg.str());

        // Reset timers and counters
        totalTime = std::chrono::duration<double>(0);
        testTime = std::chrono::duration<double>(0);
        frameCount = 0;
        lastLogTime = now;
    }
}

void Renderer::drawFullscreenQuad() {
    m_fullScreenQuad_vao.bind();
    GLCALL(glDrawArrays(GL_TRIANGLES, 0, 6));
}