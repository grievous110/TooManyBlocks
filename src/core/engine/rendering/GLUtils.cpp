#include "GLUtils.h"
#include "Logger.h"
#include <gl/glew.h>
#include <sstream>

static constexpr int maxErrorChecks = 10;

static constexpr const char* toGLErrorString(GLenum error) {
	switch (error) {
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
		default: return "Unknown OpenGL Error";
	}
}

static void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,	GLsizei length, const GLchar* message, const void* userParam) {
	std::ostringstream logStream;
    logStream << "[OpenGL] ";

    switch (source) {
        case GL_DEBUG_SOURCE_API: logStream << "(API) "; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: logStream << "(Window System) "; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: logStream << "(Shader Compiler) "; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: logStream << "(Third Party) "; break;
        case GL_DEBUG_SOURCE_APPLICATION: logStream << "(Application) "; break;
        default: logStream << "(Other) "; break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR: logStream << "[Error] "; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: logStream << "[Deprecated] "; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: logStream << "[Undefined Behavior] "; break;
        case GL_DEBUG_TYPE_PORTABILITY: logStream << "[Portability] "; break;
        case GL_DEBUG_TYPE_PERFORMANCE: logStream << "[Performance] "; break;
        case GL_DEBUG_TYPE_MARKER: logStream << "[Marker] "; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: logStream << "[Push] "; break;
        case GL_DEBUG_TYPE_POP_GROUP: logStream << "[Pop] "; break;
        default: logStream << "[Other] "; break;
    }

	logStream << "(ID: " << id << ") " << message;

	switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            lgr::lout.error(logStream.str()); break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            lgr::lout.warn(logStream.str()); break;
        case GL_DEBUG_SEVERITY_LOW:
            lgr::lout.info(logStream.str()); break;
        default:
            lgr::lout.debug(logStream.str()); break;
    }
}

void GLClearError() {
    int errorCount = 0;
    while (glGetError() != GL_NO_ERROR) {
        if (++errorCount >= maxErrorChecks) {
            lgr::lout.error("GLClearError() reached maximum error checks. Possible infinite error generation.");
            break;
        }
    }
}

bool GLLogCall(const char* functionName, const char* file, int line) {
    int errorCount = 0;
	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		std::ostringstream stream;
		stream << "[OpenGL Error " << toGLErrorString(error) << " 0x" << std::hex << error <<
			"] caused by " << functionName <<
			" in " << file <<
			" line " << std::dec << line;
		lgr::lout.error(stream.str());

		if (++errorCount >= maxErrorChecks) {
			lgr::lout.error("GLLogCall() reached maximum error checks. Possible infinite error generation.");
            break;
        }
	}
	return errorCount == 0;
}

void GLEnableDebugging() {
	int flags;
	GLCALL(glGetIntegerv(GL_CONTEXT_FLAGS, &flags));
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		GLCALL(glEnable(GL_DEBUG_OUTPUT));
		GLCALL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
		GLCALL(glDebugMessageCallback(OpenGLDebugCallback, nullptr));
		// Enable debug calls for all occasions
		GLCALL(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE));

		lgr::lout.info("OpenGL debugging enabled");
	} else {
		lgr::lout.warn("OpenGL debugging not supported in this context");
	}
}

void GLDisableDebugging() {
	GLCALL(glDisable(GL_DEBUG_OUTPUT));
	lgr::lout.info("OpenGL debugging disabled");
}

size_t GLsizeof(unsigned int type) {
    switch (type) {
		case GL_BYTE: return sizeof(GLbyte);               // 1
		case GL_UNSIGNED_BYTE: return sizeof(GLubyte);     // 1
		case GL_SHORT: return sizeof(GLshort);             // 2
		case GL_UNSIGNED_SHORT: return sizeof(GLushort);   // 2
		case GL_INT: return sizeof(GLint);                 // 4
		case GL_UNSIGNED_INT: return sizeof(GLuint);       // 4
		case GL_FLOAT: return sizeof(GLfloat);             // 4
		case GL_DOUBLE: return sizeof(GLdouble);           // 8
		default: 
			throw std::runtime_error("Unknown component type in GLTF accessor.");
	}
}