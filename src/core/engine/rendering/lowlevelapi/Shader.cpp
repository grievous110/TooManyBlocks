#include "engine/rendering/lowlevelapi/Shader.h"
#include "engine/rendering/Renderer.h"
#include "Logger.h"
#include <sstream>
#include <stdexcept>
#include <fstream>

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

static ShaderSource shaderSourceFromFile(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        throw runtime_error(string("Error opening file " + path).c_str());
    }

    const string vertexShaderIdentifier = "#vertex shader";
    const string fragmentShaderIdentifier = "#fragment shader";

    string line;
    stringstream vertexCodeBuffer;
    stringstream fragmentCodeBuffer;
    CurrentlyReading cReading = CurrentlyReading::NOTHING;

    while (getline(file, line)) {
        if (line.find(vertexShaderIdentifier) != string::npos) {
            cReading = CurrentlyReading::VERTEX;
            continue;
        } else if (line.find(fragmentShaderIdentifier) != string::npos) {
            cReading = CurrentlyReading::FRAGMENT;
            continue;
        } else {
            if (cReading == CurrentlyReading::VERTEX) {
                vertexCodeBuffer << line << endl;
            } else if (cReading == CurrentlyReading::FRAGMENT) {
                fragmentCodeBuffer << line << endl;
            }
        }
    }

    ShaderSource source = { vertexCodeBuffer.str(), fragmentCodeBuffer.str() };
    if (source.vertexSource.empty() || source.fragmentSource.empty()) {
        throw runtime_error("Vertex or shader source code was not found");
    }

    return { vertexCodeBuffer.str(), fragmentCodeBuffer.str() };
}

static unsigned int CompileShader(const unsigned int& type, const string& source) {
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

static unsigned int CreateShader(const string& vertexShader, const string& fragmentShader) {
    GLCALL(unsigned int program = glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCALL(glAttachShader(program, vs));
    GLCALL(glAttachShader(program, fs));
    GLCALL(glLinkProgram(program));
    GLCALL(glValidateProgram(program));

    GLCALL(glDeleteShader(vs));
    GLCALL(glDeleteShader(fs));

    return program;
}


Shader::Shader(const string& filepath) : m_filepath(filepath) {
    ShaderSource source = shaderSourceFromFile(filepath);
    m_rendererId = CreateShader(source.vertexSource, source.fragmentSource);
}

Shader::~Shader() {
    unbind();
    GLCALL(glDeleteProgram(m_rendererId));
}

void Shader::bind() const {
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

void Shader::setUniform(const string& name, const int& value) {
    GLCALL(glUniform1i(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, const unsigned int& value) {
    GLCALL(glUniform1ui(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, const float& value) {
    GLCALL(glUniform1f(getUniformLocation(name), value));
}

void Shader::setUniform(const std::string& name, const glm::vec2& vector) {
    GLCALL(glUniform2f(getUniformLocation(name), vector.x, vector.y));
}

void Shader::setUniform(const std::string& name, const glm::vec3& vector) {
    GLCALL(glUniform3f(getUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::setUniform(const string& name, const glm::vec4& vector) {
    GLCALL(glUniform4f(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w));
}

void Shader::setUniform(const std::string& name, const glm::ivec2& vector) {
    GLCALL(glUniform2i(getUniformLocation(name), vector.x, vector.y));
}

void Shader::setUniform(const std::string& name, const glm::ivec3& vector) {
    GLCALL(glUniform3i(getUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::setUniform(const std::string& name, const glm::ivec4& vector) {
    GLCALL(glUniform4i(getUniformLocation(name), vector.x, vector.y, vector.z, vector.w));
}

void Shader::setUniform(const std::string& name, const glm::mat3& matrix) {
    GLCALL(glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::setUniform(const string& name, const glm::mat4& matrix) {
    GLCALL(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::setUniform(const std::string& name, const bool& value) {
    GLCALL(glUniform1i(getUniformLocation(name), static_cast<int>(value)));
}

void Shader::setUniform(const std::string& name, const glm::vec2* vectors, int count) {
    GLCALL(glUniform2fv(getUniformLocation(name), count, &vectors[0][0]));
}

void Shader::setUniform(const std::string& name, const glm::vec3* vectors, int count) {
    GLCALL(glUniform3fv(getUniformLocation(name), count, &vectors[0][0]));
}

void Shader::setUniform(const std::string& name, const glm::vec4* vectors, int count) {
    GLCALL(glUniform4fv(getUniformLocation(name), count, &vectors[0][0]));
}

void Shader::setUniform(const std::string& name, const glm::mat4* matrices, int count) {
    GLCALL(glUniformMatrix4fv(getUniformLocation(name), count, GL_FALSE, &matrices[0][0][0]));
}

void Shader::setUniform(const std::string& name, const int* values, int count) {
    GLCALL(glUniform1iv(getUniformLocation(name), count, values));
}

void Shader::setUniform(const std::string& name, const unsigned int* values, int count) {
    GLCALL(glUniform1uiv(getUniformLocation(name), count, values));
}

void Shader::setUniform(const std::string& name, const float* values, int count) {
    GLCALL(glUniform1fv(getUniformLocation(name), count, values));
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
