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
	RawBuffer<Light*> lights;
	std::shared_ptr<UniformBuffer> lightBuff;
	glm::mat4 viewProjection;
	Transform viewportTransform;
	Transform meshTransform;

	LightPriority currentLightPrio;
	unsigned int lightShadowAtlasIndex;
	std::array<unsigned int, LightPriority::Count> shadowMapSizes;
	std::array<std::shared_ptr<FrameBuffer>, LightPriority::Count> shadowMapAtlases;
};

enum PassType {
	ShadowPass,
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