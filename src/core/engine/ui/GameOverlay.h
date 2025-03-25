#ifndef GAMEOVERLAY_H
#define GAMEOVERLAY_H

#include "engine/GameInstance.h"
#include "engine/ui/Ui.h"

namespace UI {
	class GameOverlay : public Window {
	private:
		bool m_showMouse;
	public:
		GameOverlay() : m_showMouse(false) {}
		virtual ~GameOverlay() = default;

		void render(ApplicationContext& context) override;
	};
}

#endif