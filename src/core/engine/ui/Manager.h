#ifndef TOOMANYBLOCKS_MANAGER_H
#define TOOMANYBLOCKS_MANAGER_H

#include <functional>
#include <string>

#include "engine/ui/Widget.h"
#include "engine/ui/fonts/FontUtil.h"

namespace UI {
    class Manager {
        std::unordered_map<std::string, std::function<Widget*()>> m_widgetFactory;

        FontPool m_fontPool;

        Widget* m_currentWidget;
        Widget* m_nextWidget;

    public:
        Manager();
        ~Manager();

        void init();

        void shutdown();

        void renderFrame();

        void registerWidget(const std::string& widgetName, std::function<Widget*()> createFn);

        bool navigateToWidget(const std::string& widgetName);

        FontData getFont(float requestedSize);
    };
}  // namespace UI

#endif
