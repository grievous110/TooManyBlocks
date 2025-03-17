#include "Application.h"
#include "engine/GameInstance.h"
#include "engine/ui/fonts/FontUtil.h"
#include "MainMenu.h"
#include <imgui.h>

namespace UI {
	void MainMenu::render(ApplicationContext& context) {
		ImGuiIO& io = ImGui::GetIO();

		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);

		ImGui::Begin("Main Menu", NULL, window_flags);
		{
			ScopedFont font(context.fontPool->getFont(55));

			float buttonWidth = 400.0f;
			float buttonHeight = 75.0f;
			float padding = 10.0f;
			float cursorAlginX = (io.DisplaySize.x - buttonWidth) / 2.0f;
			float halfHeight = io.DisplaySize.y / 2;
			ImGui::SetCursorPos({cursorAlginX, halfHeight - buttonHeight - padding});
			if (ImGui::Button("Singleplayer", ImVec2(buttonWidth, buttonHeight))) {
				navigateToWindow(context, "WorldSelection");
			}
			ImGui::SetCursorPos({cursorAlginX, halfHeight + padding});
			if (ImGui::Button("Quit", ImVec2(buttonWidth, buttonHeight))) {
				context.instance->gameState.quitGame = true;
			}
		}
		ImGui::End();
	}
}
