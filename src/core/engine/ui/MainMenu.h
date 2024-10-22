#ifndef MAINMENU_H
#define MAINMENU_H

#include "Ui.h"

namespace UI {
	class MainMenu : public Window {
	private:

	public:
		void render(ApplicationContext& context) override;
	};
}

#endif