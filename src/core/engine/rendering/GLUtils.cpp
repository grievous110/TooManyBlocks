#include "GLUtils.h"
#include "Logger.h"
#include <gl/glew.h>
#include <sstream>
#include <vector>

void GLClearError() {
	const int maxChecks = 10;  // Limit number of error checks
    int errorCount = 0;

    while (glGetError() != GL_NO_ERROR) {
        errorCount++;
        if (errorCount >= maxChecks) {
            lgr::lout.error("GLClearError() reached maximum error checks. Possible infinite error generation.");
            break;
        }
    }
}

bool GLLogCall(const char* functionName, const char* file, int line) {
	const int maxChecks = 10;  // Limit number of error checks
    int errorCount = 0;
	std::vector<GLenum> errors;
	while (GLenum error = glGetError()) {
		errors.push_back(error);
		errorCount++;
		if (errorCount >= maxChecks) {
            lgr::lout.error("GLClearError() reached maximum error checks. Possible infinite error generation.");
            break;
        }
	}
	if (!errors.empty()) {
		for (const GLenum& e : errors) {
			std::stringstream stream;
			stream << "[OpenGL Error 0x" << std::hex << e <<
				"] caused by " << functionName <<
				" in " << file <<
				" line " << line;
			lgr::lout.error(stream.str());
		}
		return false;
	}
	return true;
}