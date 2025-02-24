#ifndef RENDERER_H
#define RENDERER_H

#include "compatability/Compatability.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/Texture.h"
#include "engine/rendering/SSAOProcessor.h"
#include "engine/rendering/mat/Material.h"
#include "engine/rendering/Mesh.h"
#include "engine/rendering/Scene.h"
#include <array>
#include <memory>
#include <string>
#include <unordered_map>

struct ApplicationContext;

class Renderer {
private:
	std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaderCache;
	std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
	std::unordered_map<std::string, std::pair<std::shared_ptr<MeshRenderData>, MeshBounds>> m_meshCache;

	RenderContext m_currentRenderContext;
	
	size_t m_totalSupportedLights;
	std::array<unsigned int, LightPriority::Count> m_maxShadowMapsPerPriority;
	RawBuffer<ShaderLightStruct> lightBuffer;
	RawBuffer<glm::mat4> lightViewProjectionBuffer;

	void beginShadowpass(const Scene& scene, const ApplicationContext& context);
	void endShadowpass(const Scene& scene, const ApplicationContext& context);
	void beginSSAOpass(const Scene& scene, const ApplicationContext& context);
	void endSSAOpass(const Scene& scene, const ApplicationContext& context);
	void beginMainpass(const Scene& scene, const ApplicationContext& context);
	void endMainpass(const Scene& scene, const ApplicationContext& context);

public:
	void initialize();

	void renderScene(const Scene& scene, const ApplicationContext& context);

	void drawMesh(const Mesh& mesh);

	std::shared_ptr<Shader> getShaderFromFile(const std::string& shaderPath);

	std::shared_ptr<Texture> getTextureFromFile(const std::string& texturePath);

	std::shared_ptr<Mesh> getMeshFromFile(const std::string& meshPath);
};

#endif