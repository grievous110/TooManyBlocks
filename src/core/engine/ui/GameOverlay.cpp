#include "GameOverlay.h"
#include "Application.h"
#include "datatypes/Transform.h"
#include "engine/entity/Entity.h"
#include <imgui/imgui.h>
#include <glm/vec3.hpp>

void UI::GameOverlay::render(ApplicationContext& context) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(displaySize.x, 200), ImGuiCond_Always);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Debug Info", nullptr, window_flags);

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

    ImGui::End();
}