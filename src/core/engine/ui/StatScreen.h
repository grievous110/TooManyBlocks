#ifndef TOOMANYBLOCKS_STATSCREEN_H
#define TOOMANYBLOCKS_STATSCREEN_H

#include "engine/ui/Ui.h"

namespace UI {
    class StatScreen : public Window {
    public:
        StatScreen() = default;
        virtual ~StatScreen() = default;

        void render(ApplicationContext& context) override;
    };
}  // namespace UI

#endif