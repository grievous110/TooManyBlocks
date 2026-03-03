#include "GLFWInputManager.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

#include "Application.h"

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
        KeyEventData data;
        data.keycode = key;
        data.mods = mods;

        ApplicationContext* context = Application::getContext();
        context->inputManager->keyAdapter()->notifyObservers(
            action == GLFW_PRESS ? KeyEvent::ButtonDown : KeyEvent::ButtonUp, data
        );
    }
}

static void mouseKeyCallback(GLFWwindow* window, int button, int action, int mods) {
    MouseEventData data;
    data.key.code = button;

    ApplicationContext* context = Application::getContext();
    context->inputManager->mouseAdapter()->notifyObservers(
        action == GLFW_PRESS ? MousEvent::ButtonDown : MousEvent::ButtonUp, data
    );
}

static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    MouseEventData data;
    data.delta.x = xoffset;
    data.delta.y = yoffset;

    ApplicationContext* context = Application::getContext();
    context->inputManager->mouseAdapter()->notifyObservers(MousEvent::Scroll, data);
}

static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    ApplicationContext* context = Application::getContext();
    MouseEventData data;
    data.delta.x = xpos - context->state.lastMousepositionX;
    data.delta.y = ypos - context->state.lastMousepositionY;

    context->inputManager->mouseAdapter()->notifyObservers(MousEvent::Move, data);
    context->state.lastMousepositionX = xpos;
    context->state.lastMousepositionY = ypos;
}

void GLFWInputManager::setup() {
    GLFWwindow* window = glfwGetCurrentContext();
    if (window == nullptr) {
        throw std::runtime_error("No current glfw window was set");
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseKeyCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
}
