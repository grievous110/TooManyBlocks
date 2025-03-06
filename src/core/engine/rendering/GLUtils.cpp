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
			" line " << line;
		lgr::lout.error(stream.str());

		if (++errorCount >= maxErrorChecks) {
			lgr::lout.error("GLLogCall() reached maximum error checks. Possible infinite error generation.");
            break;
        }
	}
	return errorCount == 0;
}