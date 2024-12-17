#ifndef MATERIAL_H
#define MATERIAL_H

#include "engine/env/Light.h"
#include "engine/rendering/lowlevelapi/Shader.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct RenderContext {
	std::vector<Light> lights;
	glm::mat4 viewProjection;
	glm::mat4 modelMatrix;
	glm::vec3 meshPosition;
};

enum PassType {
	ShadowPass,
	MainPass
};

class Material {
public:
	std::shared_ptr<Shader> m_shader;

public:
	Material(std::shared_ptr<Shader> shader);
	virtual ~Material() = default;

	const std::shared_ptr<Shader> getShader() const;

	virtual bool supportsPass(PassType passType) const = 0;
	virtual void bindForPass(PassType passType, const RenderContext& context) const = 0;
	virtual void unbindForPass(PassType passType) const = 0;
};

#endif