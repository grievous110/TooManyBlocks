#ifndef TOOMANYBLOCKS_GAMEOVERLAY_H
#define TOOMANYBLOCKS_GAMEOVERLAY_H

#include "engine/ui/Ui.h"
#include "engine/ui/StatScreen.h"

namespace UI {
    class GameOverlay : public Window {
    private:
        StatScreen m_statScreen;

        bool m_showMouse;
        bool m_showStats;

    public:
        GameOverlay() : m_showMouse(false), m_showStats(false) {}
        virtual ~GameOverlay() = default;

        void render(ApplicationContext& context) override;
    };
}  // namespace UI

#endif