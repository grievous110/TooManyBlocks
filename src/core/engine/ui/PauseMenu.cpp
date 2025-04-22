#include "PauseMenu.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Application.h"
#include "engine/GameInstance.h"
#include "engine/controllers/PlayerController.h"
#include "engine/ui/fonts/FontUtil.h"

namespace UI {
    void UI::PauseMenu::render(ApplicationContext& context) {
        ImGuiIO& io = ImGui::GetIO();

        if (!ImGui::IsKeyDown(ImGuiKey_Escape)) {
            m_escWasReleased = true;
        }

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 windowSize = ImVec2(300, 200);  // Customize as needed
        ImVec2 windowPos = ImVec2((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);

        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
        ImGui::Begin("Pause Menu", nullptr, window_flags);
        {
            ScopedFont font(context.fontPool->getFont(30));
            ImGui::Text("Game Paused");
            ImGui::Separator();
            if (ImGui::Button("Resume", ImVec2(-1, 0)) ||
                (ImGui::IsKeyPressed(ImGuiKey_Escape) && m_escWasReleased)) {  // -1 makes it fill width
                context.io->keyAdapter().attach(static_cast<PlayerController*>(context.instance->m_playerController));
                context.io->mouseAdapter().attach(static_cast<PlayerController*>(context.instance->m_playerController));
                glfwSetInputMode(context.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                context.instance->gameState.gamePaused = false;
                navigateToWindow(context, "GameOverlay");
            }
            if (ImGui::Button("Exit", ImVec2(-1, 0))) {
                context.instance->gameState.gamePaused = false;
                context.instance->deinitWorld();
                navigateToWindow(context, "MainMenu");
            }
        }
        ImGui::End();
    }
}  // namespace UI
