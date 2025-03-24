#include "UiUtil.h"
#include <imgui.h>

namespace UI::Util {
    void CenterPopup() {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 popupSize = ImGui::GetWindowSize();
        ImVec2 centerPos = ImVec2((io.DisplaySize.x - popupSize.x) * 0.5f, (io.DisplaySize.y - popupSize.y) * 0.5f);
        ImGui::SetWindowPos(centerPos);
    }
    
    void MakeNextWindowFullscreen() {
        ImGuiIO& io = ImGui::GetIO();        
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
    }
}