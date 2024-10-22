#include <GL/glew.h>
#include "compatability/OpenGLTypeConversion.h"

template<>
unsigned int getOpenGLType<float>() {
    return GL_FLOAT;
}

template<>
unsigned int getOpenGLType<double>() {
    return GL_DOUBLE;
}

template<>
unsigned int getOpenGLType<int>() {
    return GL_INT;
}

template<>
unsigned int getOpenGLType<unsigned int>() {
    return GL_UNSIGNED_INT;
}

template<>
unsigned int getOpenGLType<short>() {
    return GL_SHORT;
}

template<>
unsigned int getOpenGLType<unsigned short>() {
    return GL_UNSIGNED_SHORT;
}

template<>
unsigned int getOpenGLType<char>() {
    return GL_BYTE;
}

template<>
unsigned int getOpenGLType<unsigned char>() {
    return GL_UNSIGNED_BYTE;
}