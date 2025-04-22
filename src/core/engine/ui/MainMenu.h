#ifndef MAINMENU_H
#define MAINMENU_H

#include "engine/ui/Ui.h"

namespace UI {
    class MainMenu : public Window {
    public:
        void render(ApplicationContext& context) override;
    };
}  // namespace UI

#endif