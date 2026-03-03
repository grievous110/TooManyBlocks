#include "ImguiOpenGLManager.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

UI::ImguiOpenGLManager::ImguiOpenGLManager() : m_currentWidget(nullptr), m_nextWidget(nullptr) {}

UI::ImguiOpenGLManager::~ImguiOpenGLManager() {
    if (m_currentWidget) {
        delete m_currentWidget;
    }
    if (m_nextWidget) {
        delete m_nextWidget;
    }
}

void UI::ImguiOpenGLManager::init() {
    // ImGui setup
    ImGui::CreateContext();
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f; // Weird 1px border otherwise
    ImGui::StyleColorsDark();
}

void UI::ImguiOpenGLManager::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UI::ImguiOpenGLManager::renderFrame() {
    if (m_nextWidget) {
        // Navigate safely to new window
        if (m_currentWidget) {
            delete m_currentWidget;
        }
        m_currentWidget = m_nextWidget;
        m_nextWidget = nullptr;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (m_currentWidget) {
        m_currentWidget->render();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::ImguiOpenGLManager::registerWidget(const std::string& widgetName, std::function<Widget*()> createFn) {
    m_widgetFactory.emplace(widgetName, createFn);
}

bool UI::ImguiOpenGLManager::navigateToWidget(const std::string& widgetName) {
    auto it = m_widgetFactory.find(widgetName);
    if (it != m_widgetFactory.end()) {
        // Set create as next window
        if (!m_nextWidget) {
            m_nextWidget = it->second();
            return true;
        }
    }
    return false;
}
