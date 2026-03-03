#ifndef TOOMANYBLOCKS_MANAGER_H
#define TOOMANYBLOCKS_MANAGER_H

#include <string>
#include <functional>

#include "engine/ui/Widget.h"

namespace UI {
    class Manager {
    public:
        virtual ~Manager() = default;

        virtual void init() = 0;

        virtual void shutdown() = 0;

        virtual void renderFrame() = 0;

        virtual void registerWidget(const std::string& widgetName, std::function<Widget*()> createFn) = 0;

        virtual bool navigateToWidget(const std::string& widgetName) = 0;
    };
}  // namespace UI

#endif
