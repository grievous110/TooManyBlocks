#ifndef MATERIAL_H
#define MATERIAL_H

#include "datatypes/Transform.h"
#include "datatypes/RawBuffer.h"
#include "engine/env/lights/Light.h"
#include "engine/rendering/lowlevelapi/FrameBuffer.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"
#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct RenderContext {
	glm::uvec2 currentScreenResolution;
	RawBuffer<Light*> lights;
	std::weak_ptr<UniformBuffer> lightBuff;
	std::weak_ptr<UniformBuffer> lightViewProjectionBuff;
	glm::mat4 viewProjection;
	glm::mat4 projection;
	glm::mat4 view;
	Transform viewportTransform;
	Transform meshTransform;

	LightPriority currentLightPrio;
	unsigned int lightShadowAtlasIndex;
	std::array<unsigned int, LightPriority::Count> shadowMapSizes;
	std::array<std::weak_ptr<Texture>, LightPriority::Count> shadowMapAtlases;

	std::weak_ptr<Texture> ssaoOutput;
};

enum PassType {
	ShadowPass,
	AmbientOcclusion,
	MainPass
};

class Material {
public:
	std::shared_ptr<Shader> m_shader;

public:
	Material(std::shared_ptr<Shader> shader = nullptr) : m_shader(shader) {}
	virtual ~Material() = default;

	inline std::shared_ptr<Shader> getShader() const { return m_shader; }

	virtual bool supportsPass(PassType passType) const = 0;
	virtual void bindForPass(PassType passType, const RenderContext& context) const = 0;
	virtual void bindForMeshDraw(PassType passType, const RenderContext& context) const = 0;
};

#endif