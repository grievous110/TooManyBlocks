#ifndef GAMEOVERLAY_H
#define GAMEOVERLAY_H

#include "Ui.h"
#include "engine/GameInstance.h"

namespace UI {
	class GameOverlay : public Window {
		void render(ApplicationContext& context) override;
	};
}

#endif