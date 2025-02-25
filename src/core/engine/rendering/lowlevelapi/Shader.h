#ifndef SHADER_H
#define SHADER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader : public RenderApiObject {
private:
	static thread_local unsigned int currentlyBoundShader;
	std::string m_shaderPath;
	std::unordered_map<std::string, int> m_uniformLocationCache;
	std::unordered_map<std::string, unsigned int> m_uniformBlockIndexCache;

	int getUniformLocation(const std::string& name);
	unsigned int getUniformBlockIndex(const std::string& name);

public:
	static void bindDefault();
	static void syncBinding();

	Shader(const std::string& shaderPath);
	Shader(Shader&& other) noexcept;
	virtual ~Shader();

	void bind() const;

	void setAndBindUBO(const std::string name, const UniformBuffer& ubo, unsigned int bindingPoint);

	void setUniform(const std::string& name, int value);
	void setUniform(const std::string& name, unsigned int value);
	void setUniform(const std::string& name, float value);
	void setUniform(const std::string& name, const glm::vec2& vector);
	void setUniform(const std::string& name, const glm::vec3& vector);
	void setUniform(const std::string& name, const glm::vec4& vector);
	void setUniform(const std::string& name, const glm::ivec2& vector);
	void setUniform(const std::string& name, const glm::ivec3& vector);
	void setUniform(const std::string& name, const glm::ivec4& vector);
	void setUniform(const std::string& name, const glm::uvec2& vector);
	void setUniform(const std::string& name, const glm::uvec3& vector);
	void setUniform(const std::string& name, const glm::uvec4& vector);
	void setUniform(const std::string& name, const glm::mat3& matrix);
	void setUniform(const std::string& name, const glm::mat4& matrix);
	void setUniform(const std::string& name, bool value);
	void setUniform(const std::string& name, const glm::vec2* vectors, size_t count);
	void setUniform(const std::string& name, const glm::vec3* vectors, size_t count);
	void setUniform(const std::string& name, const glm::vec4* vectors, size_t count);
	void setUniform(const std::string& name, const glm::mat4* matrices, size_t count);
	void setUniform(const std::string& name, const int* values, size_t count);
	void setUniform(const std::string& name, const unsigned int* values, size_t count);
	void setUniform(const std::string& name, const float* values, size_t count);

	Shader& operator=(Shader&& other) noexcept;
};

#endif