#ifndef SHADER_H
#define SHADER_H

#include "RenderApiObject.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

#define DEPTH_SHADER "res/shaders/depth.shader"
#define SIMPLE_SHADER "res/shaders/simpleShader.shader"

class Shader : virtual public RenderApiObject {
private:
	static unsigned int currentlyBoundShader;
	const std::string m_filepath;
	std::unordered_map<std::string, int> m_uniformLocationCache;

public:
	Shader(const std::string& filepath);
	~Shader();

	void bind() const;
	void unbind() const;

	void setUniform(const std::string& name, const int& value);
	void setUniform(const std::string& name, const unsigned int& value);
	void setUniform(const std::string& name, const float& value);
	void setUniform(const std::string& name, const glm::vec2& vector);
	void setUniform(const std::string& name, const glm::vec3& vector);
	void setUniform(const std::string& name, const glm::vec4& vector);
	void setUniform(const std::string& name, const glm::ivec2& vector);
	void setUniform(const std::string& name, const glm::ivec3& vector);
	void setUniform(const std::string& name, const glm::ivec4& vector);
	void setUniform(const std::string& name, const glm::mat3& matrix);
	void setUniform(const std::string& name, const glm::mat4& matrix);
	void setUniform(const std::string& name, const bool& value);
	void setUniform(const std::string& name, const glm::vec2* vectors, int count);
	void setUniform(const std::string& name, const glm::vec3* vectors, int count);
	void setUniform(const std::string& name, const glm::vec4* vectors, int count);
	void setUniform(const std::string& name, const glm::mat4* matrices, int count);
	void setUniform(const std::string& name, const int* values, int count);
	void setUniform(const std::string& name, const unsigned int* values, int count);
	void setUniform(const std::string& name, const float* values, int count);

private:
	int getUniformLocation(const std::string& name);
};

#endif