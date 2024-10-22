#include "engine/rendering/mat/Material.h"

Material::Material(std::shared_ptr<Shader> shader) : m_shader(shader) {}

const std::shared_ptr<Shader> Material::getShader() const {
	return m_shader;
}
