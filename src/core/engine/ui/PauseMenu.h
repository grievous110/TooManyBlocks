#ifndef PAUSEMENU_H
#define PAUSEMENU_H

#include "engine/ui/Ui.h"

namespace UI {
	class PauseMenu : public Window {
	public:
        virtual ~PauseMenu() = default;

		void render(ApplicationContext& context) override;
	};
}

#endif