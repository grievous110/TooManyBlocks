#ifndef RENDERER_H
#define RENDERER_H

#include "Application.h"
#include "compatability/Compatability.h"
#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/mat/Material.h"
#include "engine/rendering/Mesh.h"
#include "engine/rendering/Scene.h"
#include <gl/glew.h>
#include <exception>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

#define ASSERT(x, msg) if (!(x)) throw std::runtime_error(msg)
#define GLCALL(func) GLClearError(); func; if (!GLLogCall(#func, __FILE__, __LINE__)) throw std::runtime_error("Something went wrong in open gl")

void GLClearError();

bool GLLogCall(const char* functionName, const char* file, int line);

class Renderer {
private:
	std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaderCache;
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;

	RenderContext currentRenderContext;

	void beginShadowpass(const Scene& scene, const ApplicationContext& context);
	void endShadowpass(const Scene& scene, const ApplicationContext& context);
	void beginMainpass(const Scene& scene, const ApplicationContext& context);
	void endMainpass(const Scene& scene, const ApplicationContext& context);

public:
	void renderScene(const Scene& scene, const ApplicationContext& context);

	void drawMesh(const Mesh& mesh, const RenderContext& context);

	std::shared_ptr<Shader> getShaderFromFile(const std::string& shaderPath);

	std::shared_ptr<Texture> getTextureFromFile(const std::string& texturePath);
};

#endif