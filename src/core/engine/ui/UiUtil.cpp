#include "UiUtil.h"

#include <imgui.h>

static ImVec2 projectVec3ToScreen(const glm::vec3& vec) { return ImVec2(vec.x, -vec.y); }

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

    void DrawCoordinateSystem(
        glm::vec3 x,
        glm::vec3 y,
        glm::vec3 z,
        float canvasSize,
        float axisLength,
        const char* xLabel,
        const char* yLabel,
        const char* zLabel
    ) {
        // Reserve space in the layout
        ImGui::Dummy(ImVec2(canvasSize, canvasSize));
        ImVec2 canvasPos = ImGui::GetItemRectMin();  // top-left of canvas
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasCenter = ImVec2(canvasPos.x + canvasSize / 2, canvasPos.y + canvasSize / 2);

        ImVec2 xProj = projectVec3ToScreen(x * axisLength);
        ImVec2 yProj = projectVec3ToScreen(y * axisLength);
        ImVec2 zProj = projectVec3ToScreen(z * axisLength);

        // Draw axes lines
        ImVec2 endX = ImVec2(canvasCenter.x + xProj.x, canvasCenter.y + xProj.y);
        ImVec2 endY = ImVec2(canvasCenter.x + yProj.x, canvasCenter.y + yProj.y);
        ImVec2 endZ = ImVec2(canvasCenter.x + zProj.x, canvasCenter.y + zProj.y);

        drawList->AddLine(canvasCenter, endX, IM_COL32(255, 0, 0, 255), 2.0f);
        drawList->AddLine(canvasCenter, endY, IM_COL32(0, 255, 0, 255), 2.0f);
        drawList->AddLine(canvasCenter, endZ, IM_COL32(0, 0, 255, 255), 2.0f);

        // Draw axis labels
        drawList->AddText(ImVec2(endX.x + 2, endX.y), IM_COL32(255, 0, 0, 255), xLabel);
        drawList->AddText(ImVec2(endY.x + 2, endY.y), IM_COL32(0, 255, 0, 255), yLabel);
        drawList->AddText(ImVec2(endZ.x + 2, endZ.y), IM_COL32(0, 0, 255, 255), zLabel);
    }
}  // namespace UI::Util