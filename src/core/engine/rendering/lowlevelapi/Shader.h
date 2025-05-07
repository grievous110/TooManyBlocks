#ifndef TOOMANYBLOCKS_SHADER_H
#define TOOMANYBLOCKS_SHADER_H

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include "engine/rendering/lowlevelapi/UniformBuffer.h"

class ShaderDefines {
private:
    std::unordered_map<std::string, std::string> m_defines;

public:
    inline void add(const std::string& key, const std::string& value = "") { m_defines[key] = value; }

    inline const std::unordered_map<std::string, std::string>& definitions() const { return m_defines; }
};

/**
 * @brief Wrapper for OpenGL Shader programs.
 *
 * Exposes setters for uniform variables.
 */
class Shader : public RenderApiObject {
private:
    static thread_local unsigned int currentlyUsedShader;
    std::string m_shaderPath;
    std::unordered_map<std::string, int> m_uniformLocationCache;
    std::unordered_map<std::string, unsigned int> m_uniformBlockIndexCache;

    /**
     * @brief Retrieves the location of a uniform variable by name.
     *
     * @param name The name of the uniform variable.
     * @return The location of the uniform, or -1 if not found.
     */
    int getUniformLocation(const std::string& name);

    /**
     * @brief Retrieves the index of a uniform block (UBO) by name.
     *
     * @param name The name of the uniform block.
     * @return The OpenGL index of the uniform block, or GL_INVALID_INDEX if not found.
     */
    unsigned int getUniformBlockIndex(const std::string& name);

    Shader(const std::string& shaderPath, const ShaderDefines& defines);

public:
    /**
     * @brief Sets the currently used shader on the current thread to 0.
     */
    static void useDefault();
    /**
     * @brief Synchronizes the wrapper's internal usage state with OpenGL.
     *
     * Should be used if the Shader usage is changed manually.
     */
    static void syncUsage();

    /**
     * @brief Constructs a new Shader from a file path and optional preprocessor definitions.
     *
     * @param shaderPath Path to the shader files directory.
     * @param defines Optional list of preprocessor macro definitions.
     */
    static Shader create(const std::string& shaderPath, const ShaderDefines& defines = ShaderDefines());

    Shader() noexcept = default;
    Shader(Shader&& other) noexcept;
    virtual ~Shader();

    /**
     * @brief Sets this shader as the currently used program.
     *
     * @throws std::runtime_error If the buffer ID is 0 (uninitialized or moved-from).
     */
    void use() const;

    /**
     * @brief Associates and binds a Uniform Buffer Object (UBO) to a named uniform block in the shader.
     *
     * @param name The name of the uniform block in the shader.
     * @param ubo The UniformBuffer instance to bind.
     * @param bindingPoint The binding point to associate with the UBO.
     */
    void setAndBindUBO(const std::string& name, const UniformBuffer& ubo, unsigned int bindingPoint);

    void setUniform(const std::string& name, bool value);
    void setUniform(const std::string& name, int value);
    void setUniform(const std::string& name, unsigned int value);
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, const glm::bvec2& vector);
    void setUniform(const std::string& name, const glm::bvec3& vector);
    void setUniform(const std::string& name, const glm::bvec4& vector);
    void setUniform(const std::string& name, const glm::vec2& vector);
    void setUniform(const std::string& name, const glm::vec3& vector);
    void setUniform(const std::string& name, const glm::vec4& vector);
    void setUniform(const std::string& name, const glm::ivec2& vector);
    void setUniform(const std::string& name, const glm::ivec3& vector);
    void setUniform(const std::string& name, const glm::ivec4& vector);
    void setUniform(const std::string& name, const glm::uvec2& vector);
    void setUniform(const std::string& name, const glm::uvec3& vector);
    void setUniform(const std::string& name, const glm::uvec4& vector);
    void setUniform(const std::string& name, const glm::mat2& matrix);
    void setUniform(const std::string& name, const glm::mat3& matrix);
    void setUniform(const std::string& name, const glm::mat4& matrix);
    // No bool vec arrays cause the would have to be converted to int array (to have int pointer, wich opengl expects)
    void setUniform(const std::string& name, const int* values, size_t count);
    void setUniform(const std::string& name, const unsigned int* values, size_t count);
    void setUniform(const std::string& name, const float* values, size_t count);
    void setUniform(const std::string& name, const glm::vec2* vectors, size_t count);
    void setUniform(const std::string& name, const glm::vec3* vectors, size_t count);
    void setUniform(const std::string& name, const glm::vec4* vectors, size_t count);
    void setUniform(const std::string& name, const glm::ivec2* vectors, size_t count);
    void setUniform(const std::string& name, const glm::ivec3* vectors, size_t count);
    void setUniform(const std::string& name, const glm::ivec4* vectors, size_t count);
    void setUniform(const std::string& name, const glm::uvec2* vectors, size_t count);
    void setUniform(const std::string& name, const glm::uvec3* vectors, size_t count);
    void setUniform(const std::string& name, const glm::uvec4* vectors, size_t count);
    void setUniform(const std::string& name, const glm::mat2* matrices, size_t count);
    void setUniform(const std::string& name, const glm::mat3* matrices, size_t count);
    void setUniform(const std::string& name, const glm::mat4* matrices, size_t count);

    Shader& operator=(Shader&& other) noexcept;
};

#endif