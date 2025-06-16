#ifndef TOOMANYBLOCKS_RENDERER_H
#define TOOMANYBLOCKS_RENDERER_H

#include "compatability/Compatability.h"
#include "engine/env/lights/Light.h"
#include "engine/rendering/LightProcessor.h"
#include "engine/rendering/Renderable.h"
#include "engine/rendering/SSAOProcessor.h"

struct ApplicationContext;

struct LightingInfo {
    unsigned int activeLightsCount;
    const UniformBuffer* lightBuff;
    const UniformBuffer* lightViewProjectionBuff;
    LightPriority currentLightPrio;
    unsigned int lightShadowAtlasIndex;
    std::array<unsigned int, LightPriority::Count> shadowMapSizes;
    std::array<Texture*, LightPriority::Count> shadowMapAtlases;
};

struct TransformInfo {
    glm::mat4 viewProjection;
    glm::mat4 projection;
    glm::mat4 view;
    Transform viewportTransform;
    Transform meshTransform;
};

struct ParticleInfo {
    const UniformBuffer* pModulesBuff;
    unsigned int spawnCount;
    unsigned int particleSpawnOffset;
    unsigned int maxParticleCount;
    uint32_t flags;
};

struct SSAOInfo {
    const Texture* output;
};

struct SkeletalMeshInfo {
    const UniformBuffer* jointMatrices;
};

struct RenderContext {
    glm::uvec2 currScreenRes;
    float deltaTime;
    float elapsedTime;
    
    TransformInfo tInfo;
    SkeletalMeshInfo skInfo;
    LightingInfo lInfo;
    ParticleInfo pInfo;
    SSAOInfo ssaoInfo;
};

class Renderer {
private:
    std::vector<Light*> m_lightsToRender;
    std::vector<Renderable*> m_objectsToRender;

    RawBuffer<Light*> m_priodLigthsBuffer;

    VertexArray m_fullScreenQuad_vao;
    VertexBuffer m_fullScreenQuad_vbo;

    RenderContext m_currentRenderContext;
    LightProcessor m_lightProcessor;
    SSAOProcessor m_ssaoProcessor;

    void beginTransformFeedbackPass(const ApplicationContext& context);
    void endTransformFeedbackPass(const ApplicationContext& context);
    void beginShadowpass(const ApplicationContext& context);
    void endShadowpass(const ApplicationContext& context);
    void beginAmbientOcclusionPass(const ApplicationContext& context);
    void endAmbientOcclusionPass(const ApplicationContext& context);
    void beginMainpass(const ApplicationContext& context);
    void endMainpass(const ApplicationContext& context);

public:
    void initialize();

    void submitLight(Light* light);

    void submitRenderable(Renderable* obj);

    void render(const ApplicationContext& context);

    void drawFullscreenQuad();
};

#endif