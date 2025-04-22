#ifndef ABOUTSCREEN_H
#define ABOUTSCREEN_H

#include <mutex>

#include "engine/ui/Ui.h"

namespace UI {
    class AboutScreen : public Window {
    private:
        std::mutex m_mtx;
        std::string m_content;
        bool m_shouldLoadContent;

        void loadContent();

    public:
        AboutScreen() : m_shouldLoadContent(true) {}
        virtual ~AboutScreen() = default;

        void render(ApplicationContext& context) override;
    };
}  // namespace UI

#endif