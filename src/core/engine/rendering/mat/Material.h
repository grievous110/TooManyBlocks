#ifndef MATERIAL_H
#define MATERIAL_H

#include "datatypes/Transform.h"
#include "engine/env/lights/Light.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct RenderContext {
	std::vector<Light> lights;
	glm::mat4 viewProjection;
	Transform cameraTransform;
	Transform meshTransform;
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
	virtual void unbindForPass(PassType passType) const = 0;
};

#endif