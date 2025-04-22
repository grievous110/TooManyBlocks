#ifndef TOOMANYBLOCKS_MAINMENU_H
#define TOOMANYBLOCKS_MAINMENU_H

#include "engine/ui/Ui.h"

namespace UI {
    class MainMenu : public Window {
    public:
        void render(ApplicationContext& context) override;
    };
}  // namespace UI

#endif