#include "MainMenu.h"
#include "Application.h"
#include "engine/GameInstance.h"
#include <imgui/imgui.h>

namespace UI {
	void MainMenu::render(ApplicationContext& context) {
		ImGuiIO& io = ImGui::GetIO();
		ImVec2 displaySize = io.DisplaySize;

		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(displaySize);

		ImGui::Begin("Main Menu", NULL, window_flags);
		if (ImGui::Button("Start Game")) {
			context.instance->initialize();
			navigateToWindow(context, "GameOverlay");
		}
		ImGui::End();
	}
}
