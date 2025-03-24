#include "UiDialog.h"
#include "engine/ui/UiUtil.h"
#include <imgui.h>

bool UI::Dialog::Confirm(const std::string& title, const std::string& message) {
    bool confirmed = false;
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        UI::Util::CenterPopup();
        ImGui::TextWrapped("%s", message.c_str());

        ImGui::Separator();
        
        if (ImGui::Button("Yes")) {
            confirmed = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    return confirmed;
}

bool UI::Dialog::Error(const std::string &title, const std::string &message) {
    if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", message.c_str());

        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
        return true;
    }
    return false;
}
