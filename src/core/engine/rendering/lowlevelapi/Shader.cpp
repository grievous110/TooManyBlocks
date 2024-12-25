#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/Renderer.h"
#include "Logger.h"
#include <sstream>
#include <stdexcept>
#include <fstream>
#include "Shader.h"

using namespace std;

struct ShaderSource {
    const string vertexSource;
    const string fragmentSource;
};

enum CurrentlyReading {
    VERTEX,
    FRAGMENT,
    NOTHING
};

unsigned int Shader::currentlyBoundShader = 0;

string readFile(const string& filepath) {
    ifstream file(filepath, ios::in);
    if (!file.is_open()) {
        throw runtime_error("Failed to open file: " + filepath);
    }
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

ShaderSource shaderSourceFromFile(const string& shaderPath) {
    string basename = shaderPath.substr(shaderPath.find_last_of("/\\") + 1);
    string vertFile = shaderPath + "/" + basename + ".vert";
    string fragFile = shaderPath + "/" + basename + ".frag";

    string vertexShaderCode = readFile(vertFile);
    string fragmentShaderCode = readFile(fragFile);

    return { vertexShaderCode, fragmentShaderCode };
}

unsigned int compileShader(const unsigned int& type, const string& source) {
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	GLCALL(glShaderSource(id, 1, &src, nullptr));
	GLCALL(glCompileShader(id));

	int result;
	GLCALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

	if (result == GL_FALSE) {
		int length;
		GLCALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = new char[length];
		GLCALL(glGetShaderInfoLog(id, length, &length, message));
        stringstream stream;
		stream << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << endl;
		stream << message << endl;
        lgr::lout.error(stream.str());
		delete[] message;
		GLCALL(glDeleteShader(id));
		return 0;
	}

	return id;
}

unsigned int createShader(const string& vertexShader, const string& fragmentShader) {
    GLCALL(unsigned int program = glCreateProgram());
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCALL(glAttachShader(program, vs));
    GLCALL(glAttachShader(program, fs));
    GLCALL(glLinkProgram(program));
    GLCALL(glValidateProgram(program));

    GLCALL(glDeleteShader(vs));
    GLCALL(glDeleteShader(fs));

    return program;
}


Shader::Shader(const string& shaderPath) : m_shaderPath(shaderPath) {
    ShaderSource source = shaderSourceFromFile(shaderPath);
    m_rendererId = createShader(source.vertexSource, source.fragmentSource);
}

Shader::Shader(Shader&& other) noexcept : RenderApiObject(std::move(other)), m_shaderPath(std::move(other.m_shaderPath)) {
    other.m_uniformLocationCache.clear();
}

Shader::~Shader() {
    if (m_rendererId != 0) {
        unbind();
        GLCALL(glDeleteProgram(m_rendererId));
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

void Shader::unbind() const {
    if (Shader::currentlyBoundShader == m_rendererId) {
        GLCALL(glUseProgram(0));
        Shader::currentlyBoundShader = 0;
    }
}

void Shader::setUniform(const string& name, int value) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform1i(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, unsigned int value) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform1ui(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, float value) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform1f(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, const glm::vec2& vector) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform2f(getUniformLocation(name), vector.x, vector.y));
}

void Shader::setUniform(const std::string& name, const glm::vec3& vector) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform3f(getUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::setUniform(const string& name, const glm::vec4& vector) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform4f(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w));
}

void Shader::setUniform(const std::string& name, const glm::ivec2& vector) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform2i(getUniformLocation(name), vector.x, vector.y));
}

void Shader::setUniform(const std::string& name, const glm::ivec3& vector) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform3i(getUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::setUniform(const std::string& name, const glm::ivec4& vector) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform4i(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w));
}

void Shader::setUniform(const std::string& name, const glm::mat3& matrix) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::setUniform(const string& name, const glm::mat4& matrix) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::setUniform(const std::string& name, bool value) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform1i(getUniformLocation(name), static_cast<int>(value)));
}

void Shader::setUniform(const std::string& name, const glm::vec2* vectors, int count) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform2fv(getUniformLocation(name), count, &vectors[0][0]));
}

void Shader::setUniform(const std::string& name, const glm::vec3* vectors, int count) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform3fv(getUniformLocation(name), count, &vectors[0][0]));
}

void Shader::setUniform(const std::string& name, const glm::vec4* vectors, int count) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform4fv(getUniformLocation(name), count, &vectors[0][0]));
}

void Shader::setUniform(const std::string& name, const glm::mat4* matrices, int count) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniformMatrix4fv(getUniformLocation(name), count, GL_FALSE, &matrices[0][0][0]));
}

void Shader::setUniform(const std::string& name, const int* values, int count) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform1iv(getUniformLocation(name), count, values));
}

void Shader::setUniform(const std::string& name, const unsigned int* values, int count) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform1uiv(getUniformLocation(name), count, values));
}

void Shader::setUniform(const std::string& name, const float* values, int count) {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of Shader with id 0");

    GLCALL(glUniform1fv(getUniformLocation(name), count, values));
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_rendererId != 0) {
            GLCALL(glDeleteProgram(m_rendererId));
        }
        RenderApiObject::operator=(std::move(other));

        m_shaderPath = std::move(other.m_shaderPath);
        m_uniformLocationCache.clear();
        
        other.m_uniformLocationCache.clear();
    }
    return *this;
}

int Shader::getUniformLocation(const string& name) {
    if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) {
        return m_uniformLocationCache[name];
    }

    GLCALL(int location = glGetUniformLocation(m_rendererId, name.c_str()));
    if (location == -1) { // -1 is not found / can happen for unused uniforms also
        lgr::lout.warn("Warning: location of '" + string(name) + "' uniform was not found!");
    }

    m_uniformLocationCache[name] = location;
    return location;
}
