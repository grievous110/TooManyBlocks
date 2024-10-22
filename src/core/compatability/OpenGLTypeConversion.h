#ifndef OPENGLTYPECONVERSION_H
#define OPENGLTYPECONVERSION_H

#include <type_traits>

template<typename T>
unsigned int getOpenGLType() {
    static_assert(std::is_same<T, void>::value, "Unsupported type for OpenGL.");
    return 0; // This line will never be reached
}

template<>
unsigned int getOpenGLType<float>();

template<>
unsigned int getOpenGLType<double>();

template<>
unsigned int getOpenGLType<int>();

template<>
unsigned int getOpenGLType<unsigned int>();

template<>
unsigned int getOpenGLType<short>();

template<>
unsigned int getOpenGLType<unsigned short>();

template<>
unsigned int getOpenGLType<char>();

template<>
unsigned int getOpenGLType<unsigned char>();

#endif
