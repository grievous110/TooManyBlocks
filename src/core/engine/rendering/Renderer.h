#ifndef TOOMANYBLOCKS_RENDERER_H
#define TOOMANYBLOCKS_RENDERER_H

#include <memory>

#include "compatability/Compatability.h"
#include "engine/env/lights/Light.h"
#include "engine/rendering/LightProcessor.h"
#include "engine/rendering/Renderable.h"
#include "engine/rendering/SSAOProcessor.h"

struct ApplicationContext;

class Renderer {
private:
    std::vector<Light*> m_lightsToRender;
    std::vector<Renderable*> m_objectsToRender;

    std::unique_ptr<VertexArray> m_fullScreenQuad_vao;
    std::unique_ptr<VertexBuffer> m_fullScreenQuad_vbo;

    RenderContext m_currentRenderContext;
    LightProcessor m_lightProcessor;
    SSAOProcessor m_ssaoProcessor;

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