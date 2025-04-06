#include "Application.h"
#include "datatypes/Transform.h"
#include "engine/controllers/PlayerController.h"
#include "engine/entity/Entity.h"
#include "engine/ui/fonts/FontUtil.h"
#include "GameOverlay.h"
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <imgui.h>

void UI::GameOverlay::render(ApplicationContext& context) {
    ImGuiIO& io = ImGui::GetIO();
    
    ImGuiWindowFlags window_flags =
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoTitleBar;
    
    UI::Util::MakeNextWindowFullscreen();
    ImGui::Begin("Game Overlay", nullptr, window_flags);
    {
        ScopedFont font(context.fontPool->getFont(20));

        Transform& cameraTransform = context.instance->m_player->getCamera()->getLocalTransform();
        glm::vec3 camRotation = cameraTransform.getRotationEuler();
        glm::vec3 camForward = cameraTransform.getForward();
        glm::vec3 playerPos = context.instance->m_player->getTransform().getPosition();
        glm::vec3 velocity = context.instance->m_player->getVelocity();

        ImGui::Text("Application average: %.1f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Player Position: x=%.1f, y=%.1f, z=%.1f", playerPos.x, playerPos.y, playerPos.z);
        ImGui::Text("Cam Rotation: x=%.1f, y=%.1f, z=%.1f", camRotation.x, camRotation.y, camRotation.z);
        ImGui::Text("Cam Forward Vec: x=%.1f, y=%.1f, z=%.1f", camForward.x, camForward.y, camForward.z);
        ImGui::Text("Velocity: x=%.1f, y=%.1f, z=%.1f", velocity.x, velocity.y, velocity.z);
        
        UI::Util::DrawCrosshair(35.0f, 3.0f);

        bool newShowMouse = ImGui::IsKeyDown(ImGuiKey_LeftAlt);
        if (newShowMouse != m_showMouse) {
            m_showMouse = newShowMouse;
            glfwSetInputMode(context.window, GLFW_CURSOR, m_showMouse ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            context.instance->gameState.gamePaused = true;
            context.io->keyAdapter().detach(static_cast<PlayerController*>(context.instance->m_player->getController()));
            context.io->mouseAdapter().detach(static_cast<PlayerController*>(context.instance->m_player->getController()));
            glfwSetInputMode(context.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            navigateToWindow(context, "PauseMenu");
        }
    }
    ImGui::End();
}