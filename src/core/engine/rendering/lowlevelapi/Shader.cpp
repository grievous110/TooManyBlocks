#include "engine/rendering/GLUtils.h"
#include "Logger.h"
#include "Shader.h"
#include <fstream>
#include <gl/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stddef.h>
#include <stdexcept>

struct ShaderSource {
    std::string vertexSource;
    std::string fragmentSource;
};

enum CurrentlyReading {
    VERTEX,
    FRAGMENT,
    NOTHING
};

thread_local unsigned int Shader::currentlyBoundShader = 0;

static std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

static ShaderSource shaderSourceFromFile(const std::string& shaderPath) {
    std::string basename = shaderPath.substr(shaderPath.find_last_of("/\\") + 1);
    std::string vertFile = shaderPath + "/" + basename + ".vert";
    std::string fragFile = shaderPath + "/" + basename + ".frag";

    std::string vertexShaderCode = readFile(vertFile);
    std::string fragmentShaderCode = readFile(fragFile);

    return { vertexShaderCode, fragmentShaderCode };
}

static unsigned int compileShader(unsigned int type, const std::string& source, const ShaderDefines& definitions) {
    const char* src = nullptr;
    std::string processedSource;
    
    // Insert optional compile definitions
    if (!definitions.definitions().empty()) {
        size_t versionPos = source.find("#version");
        size_t insertPos = source.find('\n', versionPos);
        
        if (versionPos != std::string::npos && insertPos != std::string::npos) {
            std::string defineBlock;
            for (const auto& define : definitions.definitions()) {
                defineBlock += "#define " + define.first;
                if (!define.second.empty()) {
                    defineBlock += " " + define.second;
                }
                defineBlock += "\n";
            }

            size_t preLength = insertPos + 1; // include newline
            processedSource.reserve(defineBlock.size() + source.size()); // Preallocate

            // Build final string in-place
            processedSource.append(source, 0, preLength);
            processedSource.append(defineBlock);
            processedSource.append(source, preLength);

            src = processedSource.c_str();
        } else {
            // Fallback
            src = source.c_str();
        }
    } else {
        src = source.c_str();
    }

	unsigned int id = glCreateShader(type);
	GLCALL(glShaderSource(id, 1, &src, nullptr));
	GLCALL(glCompileShader(id));

	int compileSuccess;
	GLCALL(glGetShaderiv(id, GL_COMPILE_STATUS, &compileSuccess));
	if (compileSuccess == GL_FALSE) {
		int length;
		GLCALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = new char[length];
		GLCALL(glGetShaderInfoLog(id, length, &length, message));
        std::ostringstream ostream;
		ostream << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		ostream << message << std::endl;
        lgr::lout.error(ostream.str());
		delete[] message;
		GLCALL(glDeleteShader(id));
		return 0;
	}

	return id;
}

static unsigned int createShader(const ShaderSource& source, const ShaderDefines& definitions) {
    GLCALL(unsigned int program = glCreateProgram());
    unsigned int vs = compileShader(GL_VERTEX_SHADER, source.vertexSource, definitions);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, source.fragmentSource, definitions);

    GLCALL(glAttachShader(program, vs));
    GLCALL(glAttachShader(program, fs));
    GLCALL(glLinkProgram(program));

    int linkingSuccess;
    GLCALL(glGetProgramiv(program, GL_LINK_STATUS, &linkingSuccess));
    if (!linkingSuccess) {
        int logLength = 0;
        GLCALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength));
        if (logLength > 0) {
            std::ostringstream ostream;
            char* infoLog = new char[logLength];
            glGetProgramInfoLog(program, logLength, nullptr, infoLog);
            ostream << "Shader Linking Error:\n" << infoLog << std::endl;
            lgr::lout.error(ostream.str());
            delete[] infoLog;
        }
    }

    GLCALL(glValidateProgram(program));

    GLCALL(glDeleteShader(vs));
    GLCALL(glDeleteShader(fs));

    return program;
}

int Shader::getUniformLocation(const std::string& name) {
    auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }

    GLCALL(int location = glGetUniformLocation(m_rendererId, name.c_str()));
    if (location == -1) { // -1 is not found / can happen for unused uniforms also
        lgr::lout.warn("Warning: location of '" + std::string(name) + "' uniform was not found!");
    }

    m_uniformLocationCache[name] = location;
    return location;
}

unsigned int Shader::getUniformBlockIndex(const std::string& name) {
    auto it = m_uniformBlockIndexCache.find(name);
    if (it != m_uniformBlockIndexCache.end()) {
        return it->second;
    }

    GLCALL(unsigned int blockIndex = glGetUniformBlockIndex(m_rendererId, name.c_str()));
    if (blockIndex == GL_INVALID_INDEX) {
        lgr::lout.warn("Warning: Index of '" + std::string(name) + "' uniform buffer was invalid!");
    }

    m_uniformBlockIndexCache[name] = blockIndex;
    return blockIndex;
}

void Shader::bindDefault() {
    if (Shader::currentlyBoundShader != 0) {
        GLCALL(glUseProgram(0));
        Shader::currentlyBoundShader = 0;
    }
}

void Shader::syncBinding() {
    int binding;
    GLCALL(glGetIntegerv(GL_CURRENT_PROGRAM, &binding));
    Shader::currentlyBoundShader = static_cast<unsigned int>(binding);
}

Shader::Shader(const std::string& shaderPath, const ShaderDefines& definitions) : m_shaderPath(shaderPath) {
    ShaderSource source = shaderSourceFromFile(shaderPath);
    m_rendererId = createShader(source, definitions);
}

Shader::Shader(Shader&& other) noexcept : RenderApiObject(std::move(other)), m_shaderPath(std::move(other.m_shaderPath)) {
    other.m_uniformLocationCache.clear();
}

Shader::~Shader() {
    if (m_rendererId != 0) {
        try {
            if (Shader::currentlyBoundShader == m_rendererId) {
                GLCALL(glUseProgram(0));
                Shader::currentlyBoundShader = 0;
            }
            GLCALL(glDeleteProgram(m_rendererId));
		} catch (const std::exception&) {
			lgr::lout.error("Error during Shader cleanup");
		}
    }
}

void Shader::bind() const {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    if (Shader::currentlyBoundShader != m_rendererId) {
        GLCALL(glUseProgram(m_rendererId));
        Shader::currentlyBoundShader = m_rendererId;
    }
}

void Shader::setAndBindUBO(const std::string& name, const UniformBuffer &ubo, unsigned int bindingPoint) {
    bind();
    ubo.bind(bindingPoint);
    GLCALL(glUniformBlockBinding(m_rendererId, getUniformBlockIndex(name), bindingPoint));
}

void Shader::setUniform(const std::string& name, bool value) {
    bind();
    GLCALL(glUniform1i(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, int value) {
    bind();
    GLCALL(glUniform1i(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, unsigned int value) {
    bind();
    GLCALL(glUniform1ui(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, float value) {
    bind();
    GLCALL(glUniform1f(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, const glm::bvec2& vector) {
    bind();
    GLCALL(glUniform2i(getUniformLocation(name), vector.x, vector.y));
}

void Shader::setUniform(const std::string& name, const glm::bvec3& vector) {
    bind();
    GLCALL(glUniform3i(getUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::setUniform(const std::string& name, const glm::bvec4& vector) {
    bind();
    GLCALL(glUniform4i(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w));
}

void Shader::setUniform(const std::string& name, const glm::vec2& vector) {
    bind();
    GLCALL(glUniform2f(getUniformLocation(name), vector.x, vector.y));
}

void Shader::setUniform(const std::string& name, const glm::vec3& vector) {
    bind();
    GLCALL(glUniform3f(getUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::setUniform(const std::string& name, const glm::vec4& vector) {
    bind();
    GLCALL(glUniform4f(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w));
}

void Shader::setUniform(const std::string& name, const glm::ivec2& vector) {
    bind();
    GLCALL(glUniform2i(getUniformLocation(name), vector.x, vector.y));
}

void Shader::setUniform(const std::string& name, const glm::ivec3& vector) {
    bind();
    GLCALL(glUniform3i(getUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::setUniform(const std::string& name, const glm::ivec4& vector) {
    bind();
    GLCALL(glUniform4i(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w));
}

void Shader::setUniform(const std::string& name, const glm::uvec2& vector) {
    bind();
    GLCALL(glUniform2ui(getUniformLocation(name), vector.x, vector.y));
}

void Shader::setUniform(const std::string& name, const glm::uvec3& vector) {
    bind();
    GLCALL(glUniform3ui(getUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::setUniform(const std::string& name, const glm::uvec4& vector) {
    bind();
    GLCALL(glUniform4ui(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w));
}

void Shader::setUniform(const std::string& name, const glm::mat2& matrix) {
    bind();
    GLCALL(glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)));
}

void Shader::setUniform(const std::string& name, const glm::mat3& matrix) {
    bind();
    GLCALL(glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)));
}

void Shader::setUniform(const std::string& name, const glm::mat4& matrix) {
    bind();
    GLCALL(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)));
}

void Shader::setUniform(const std::string& name, const int* values, size_t count) {
    bind();
    GLCALL(glUniform1iv(getUniformLocation(name), count, values));
}

void Shader::setUniform(const std::string& name, const unsigned int* values, size_t count) {
    bind();
    GLCALL(glUniform1uiv(getUniformLocation(name), count, values));
}

void Shader::setUniform(const std::string& name, const float* values, size_t count) {
    bind();
    GLCALL(glUniform1fv(getUniformLocation(name), count, values));
}

void Shader::setUniform(const std::string& name, const glm::vec2* vectors, size_t count) {
    bind();
    GLCALL(glUniform2fv(getUniformLocation(name), count, glm::value_ptr(vectors[0])));
}

void Shader::setUniform(const std::string& name, const glm::vec3* vectors, size_t count) {
    bind();
    GLCALL(glUniform3fv(getUniformLocation(name), count, glm::value_ptr(vectors[0])));
}

void Shader::setUniform(const std::string& name, const glm::vec4* vectors, size_t count) {
    bind();
    GLCALL(glUniform4fv(getUniformLocation(name), count, glm::value_ptr(vectors[0])));
}

void Shader::setUniform(const std::string& name, const glm::ivec2* vectors, size_t count) {
    bind();
    GLCALL(glUniform2iv(getUniformLocation(name), count, glm::value_ptr(vectors[0])));
}

void Shader::setUniform(const std::string& name, const glm::ivec3* vectors, size_t count) {
    bind();
    GLCALL(glUniform3iv(getUniformLocation(name), count, glm::value_ptr(vectors[0])));
}

void Shader::setUniform(const std::string& name, const glm::ivec4* vectors, size_t count) {
    bind();
    GLCALL(glUniform4iv(getUniformLocation(name), count, glm::value_ptr(vectors[0])));
}

void Shader::setUniform(const std::string& name, const glm::uvec2* vectors, size_t count) {
    bind();
    GLCALL(glUniform2uiv(getUniformLocation(name), count, glm::value_ptr(vectors[0])));
}

void Shader::setUniform(const std::string& name, const glm::uvec3* vectors, size_t count) {
    bind();
    GLCALL(glUniform3uiv(getUniformLocation(name), count, glm::value_ptr(vectors[0])));
}

void Shader::setUniform(const std::string& name, const glm::uvec4* vectors, size_t count) {
    bind();
    GLCALL(glUniform4uiv(getUniformLocation(name), count, glm::value_ptr(vectors[0])));
}

void Shader::setUniform(const std::string& name, const glm::mat2* matrices, size_t count) {
    bind();
    GLCALL(glUniformMatrix2fv(getUniformLocation(name), count, GL_FALSE, glm::value_ptr(matrices[0])));
}

void Shader::setUniform(const std::string& name, const glm::mat3* matrices, size_t count) {
    bind();
    GLCALL(glUniformMatrix3fv(getUniformLocation(name), count, GL_FALSE, glm::value_ptr(matrices[0])));
}

void Shader::setUniform(const std::string& name, const glm::mat4* matrices, size_t count) {
    bind();
    GLCALL(glUniformMatrix4fv(getUniformLocation(name), count, GL_FALSE, glm::value_ptr(matrices[0])));
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_rendererId != 0) {
        try {
            if (Shader::currentlyBoundShader == m_rendererId) {
                GLCALL(glUseProgram(0));
                Shader::currentlyBoundShader = 0;
            }
            GLCALL(glDeleteProgram(m_rendererId));
		} catch (const std::exception&) {
			lgr::lout.error("Error during Shader cleanup");
		}
    }
        RenderApiObject::operator=(std::move(other));

        m_shaderPath = std::move(other.m_shaderPath);
        m_uniformLocationCache.clear();
        
        other.m_uniformLocationCache.clear();
    }
    return *this;
}