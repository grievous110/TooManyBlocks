#ifndef RENDERER_H
#define RENDERER_H

#include "compatability/Compatability.h"
#include "engine/rendering/LightProcessor.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/mat/Material.h"
#include "engine/rendering/Mesh.h"
#include "engine/rendering/Scene.h"
#include "engine/rendering/SSAOProcessor.h"
#include <array>
#include <memory>
#include <string>
#include <unordered_map>

struct ApplicationContext;

class Renderer {
private:
	std::unique_ptr<VertexArray> m_fullScreenQuad_vao;
	std::unique_ptr<VertexBuffer> m_fullScreenQuad_vbo;

	RenderContext m_currentRenderContext;
	LightProcessor m_lightProcessor;
	SSAOProcessor m_ssaoProcessor;

	void beginShadowpass(const Scene& scene, const ApplicationContext& context);
	void endShadowpass(const Scene& scene, const ApplicationContext& context);
	void beginAmbientOcclusionPass(const Scene& scene, const ApplicationContext& context);
	void endAmbientOcclusionPass(const Scene& scene, const ApplicationContext& context);
	void beginMainpass(const Scene& scene, const ApplicationContext& context);
	void endMainpass(const Scene& scene, const ApplicationContext& context);

public:
	void initialize();

	void renderScene(const Scene& scene, const ApplicationContext& context);

	void drawMesh(const Mesh& mesh);

	void drawFullscreenQuad();
};

#endif