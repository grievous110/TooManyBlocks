#ifndef GAMEOVERLAY_H
#define GAMEOVERLAY_H

#include "engine/GameInstance.h"
#include "engine/ui/Ui.h"

namespace UI {
	class GameOverlay : public Window {
	public:
		void render(ApplicationContext& context) override;
	};
}

#endif