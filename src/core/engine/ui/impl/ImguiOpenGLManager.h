#ifndef TOOMANYBLOCKS_IMGUIOPENGLMANAGER_H
#define TOOMANYBLOCKS_IMGUIOPENGLMANAGER_H

#include "engine/ui/Ui.h"
#include <unordered_map>

namespace UI {
    class ImguiOpenGLManager : public Manager {
    private:
        std::unordered_map<std::string, std::function<Widget*()>> m_widgetFactory;

        Widget* m_currentWidget;
        Widget* m_nextWidget;

    public:
        ImguiOpenGLManager();
        virtual ~ImguiOpenGLManager();

        virtual void init() override;

        virtual void shutdown() override;

        virtual void renderFrame() override;

        virtual void registerWidget(const std::string& widgetName, std::function<Widget*()> createFn) override;

        virtual bool navigateToWidget(const std::string& widgetName) override;
    };
}  // namespace UI

#endif
