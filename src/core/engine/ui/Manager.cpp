#include "Manager.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "AppConstants.h"

UI::Manager::Manager() : m_currentWidget(nullptr), m_nextWidget(nullptr) {}

UI::Manager::~Manager() {
    if (m_currentWidget) {
        delete m_currentWidget;
    }
    if (m_nextWidget) {
        delete m_nextWidget;
    }
}

void UI::Manager::init() {
    // ImGui setup
    ImGui::CreateContext();
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;  // Weird 1px border otherwise
    ImGui::StyleColorsDark();

    // Load font with sizes used for interpolation
    m_fontPool.loadFontSizes(Res::Font::PROGGY_CLEAN, {16.0f, 32.0f, 48.0f, 64.0f});
}

void UI::Manager::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UI::Manager::renderFrame() {
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

void UI::Manager::registerWidget(const std::string& widgetName, std::function<Widget*()> createFn) {
    m_widgetFactory.emplace(widgetName, createFn);
}

bool UI::Manager::navigateToWidget(const std::string& widgetName) {
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

FontData UI::Manager::getFont(float requestedSize) { return m_fontPool.getFont(requestedSize); }
