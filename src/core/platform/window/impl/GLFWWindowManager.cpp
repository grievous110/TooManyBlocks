#include "GLFWWindowManager.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

#include "Application.h"

static void windowResizeCallback(GLFWwindow* window, int width, int height) {
    ApplicationContext* context = Application::getContext();
    context->state.screenWidth = static_cast<unsigned int>(width);
    context->state.screenHeight = static_cast<unsigned int>(height);
}

GLFWWindowManager::GLFWWindowManager() : m_currentWindow(nullptr), m_isInitialized(false) {}

GLFWWindowManager::~GLFWWindowManager() { shutdown(); }

void GLFWWindowManager::init() {
    if (m_isInitialized) {
        throw std::runtime_error("Duplicate initialization!");
    }

    if (glfwInit() != GLFW_TRUE) throw std::runtime_error("Error initializing glfw!");

    // Specifiy OpenGl Version to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Core does not auto create vertex array objects, compatibitly does
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef DEBUG_MODE  // Create debug open gl context (Some graphics cards do not enable debugging by default)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
}

void GLFWWindowManager::shutdown() {
    if (m_isInitialized) {
        // This implicitly destroys open gl context -> gl calls afterwards will cause error
        if (m_currentWindow) {
            glfwDestroyWindow(m_currentWindow);
        }
        glfwTerminate();
    }
}

void GLFWWindowManager::createActiveWindow(unsigned int width, unsigned int height, const char* title) {
    m_currentWindow = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwSetFramebufferSizeCallback(m_currentWindow, windowResizeCallback);
    glfwMakeContextCurrent(m_currentWindow);  // Set active (Also activates OpenGL context on this thread)
}

bool GLFWWindowManager::shouldWindowClose() const { return glfwWindowShouldClose(m_currentWindow); }

void GLFWWindowManager::swapBuffers() { glfwSwapBuffers(m_currentWindow); }

void GLFWWindowManager::pollEvents() { glfwPollEvents(); }

void GLFWWindowManager::enableVSync(bool enabled) { glfwSwapInterval(enabled ? 1 : 0); }

void GLFWWindowManager::setCursorMode(CursorMode mode) {
    switch (mode) {
        case CursorMode::Normal: glfwSetInputMode(m_currentWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL); break;
        case CursorMode::HiddenAndCaptured: glfwSetInputMode(m_currentWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); break;
        default: throw std::runtime_error("Unspecified cursor mode");
    }
}
