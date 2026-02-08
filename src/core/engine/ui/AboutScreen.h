#ifndef TOOMANYBLOCKS_ABOUTSCREEN_H
#define TOOMANYBLOCKS_ABOUTSCREEN_H

#include "engine/ui/Ui.h"
#include "threading/Future.h"

namespace UI {
    class AboutScreen : public Window {
    private:
        Future<std::string> m_content;

        void startLoadingContent();

        void pullErrorFromContentLoad();

    public:
        AboutScreen() = default;
        virtual ~AboutScreen() = default;

        void render(ApplicationContext& context) override;
    };
}  // namespace UI

#endif