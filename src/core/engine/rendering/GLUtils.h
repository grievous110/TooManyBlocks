#ifndef GLUTILS_H
#define GLUTILS_H

#include <stdexcept>

#define GLCALL(func) GLClearError(); func; if (!GLLogCall(#func, __FILE__, __LINE__)) throw std::runtime_error("Something went wrong in open gl")
//#define GLCALL(func) func

void GLClearError();

bool GLLogCall(const char* functionName, const char* file, int line);

void GLEnableDebugging();

void GLDisableDebugging();

#endif