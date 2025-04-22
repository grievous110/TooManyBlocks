#ifndef PAUSEMENU_H
#define PAUSEMENU_H

#include "engine/ui/Ui.h"

namespace UI {
    class PauseMenu : public Window {
    private:
        bool m_escWasReleased;

    public:
        PauseMenu() : m_escWasReleased(false) {}
        virtual ~PauseMenu() = default;

        void render(ApplicationContext& context) override;
    };
}  // namespace UI

#endif