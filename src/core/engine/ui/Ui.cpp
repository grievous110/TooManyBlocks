#include "Ui.h"

#include "Application.h"

namespace UI {
    WindowFactory& getWindowFactory() {
        // Singleton factory function mapping
        static WindowFactory factory;
        return factory;
    }

    std::vector<std::string> listRegisteredWindows() {
        const WindowFactory& factory = getWindowFactory();
        std::vector<std::string> list;
        list.reserve(factory.size());

        for (const auto& pair : factory) {
            list.push_back(pair.first);
        }
        return list;
    }

    bool navigateToWindow(ApplicationContext& context, const std::string& windowName) {
        WindowFactory& factory = getWindowFactory();
        auto it = factory.find(windowName);
        if (it != factory.end()) {
            // Set create as next window
            if (!context.nextWindow) {
                context.nextWindow = it->second();
                return true;
            }
        }
        return false;
    }
}  // namespace UI