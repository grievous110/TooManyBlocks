#include "GLUtils.h"
#include "Logger.h"
#include <gl/glew.h>
#include <sstream>

static constexpr int maxErrorChecks = 10;

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
		std::stringstream stream;
		stream << "[OpenGL Error 0x" << std::hex << error <<
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