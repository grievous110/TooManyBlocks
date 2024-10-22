#ifndef RENDERER_H
#define RENDERER_H

#include "Application.h"
#include "engine/rendering/cache/ShaderCache.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/mat/Material.h"
#include "engine/rendering/Mesh.h"
#include "Scene.h"
#include <gl/glew.h>
#include <exception>
#include <string>
#include <memory>
#include <iostream>

#define ASSERT(x, msg) if (!(x)) throw std::runtime_error(msg)
#ifdef _DEBUG
#define GLCALL(func) GLClearError(); func; if (!GLLogCall(#func, __FILE__, __LINE__)) __debugbreak()
#else
#define GLCALL(func) func;
#endif

void GLClearError();

bool GLLogCall(const char* functionName, const char* file, int line);

class Renderer {
private:
	ShaderCache m_shaderCache;
	RenderContext currentRenderContext;

	void beginShadowpass(const Scene& scene);
	void endShadowpass(const Scene& scene);
	void beginMainpass(const Scene& scene);
	void endMainpass(const Scene& scene);
	void clear() const;

public:
	Renderer();

	void renderScene(const Scene& scene, const ApplicationContext& context);

	void drawMesh(const Mesh& mesh, const RenderContext& context);

	void draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;

	std::shared_ptr<Shader> getShader(const std::string& shader);
};

#endif