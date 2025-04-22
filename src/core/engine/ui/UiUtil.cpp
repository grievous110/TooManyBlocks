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

    void DrawCrosshair(float crossSize, float thickness) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 center = ImVec2(windowPos.x + windowSize.x * 0.5f, windowPos.y + windowSize.y * 0.5f);

        // Horizontal line
        drawList->AddLine(
            ImVec2(center.x - crossSize / 2, center.y), ImVec2(center.x + crossSize / 2, center.y),
            IM_COL32(255, 255, 255, 255), thickness
        );

        // Vertical line
        drawList->AddLine(
            ImVec2(center.x, center.y - crossSize / 2), ImVec2(center.x, center.y + crossSize / 2),
            IM_COL32(255, 255, 255, 255), thickness
        );
    }
}  // namespace UI::Util