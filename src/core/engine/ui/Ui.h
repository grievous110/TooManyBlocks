#ifndef UIWINDOW_H
#define UIWINDOW_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/ui/UiDialog.h"
#include "engine/ui/UiUtil.h"

struct ApplicationContext;

namespace UI {
    class Window;
    using WindowFactory = std::unordered_map<std::string, std::function<Window*()>>;

    WindowFactory& getWindowFactory();

    template <typename T>
    void registerWindow(const std::string& windowName) {
        // Register creator function in singelton factory
        getWindowFactory()[windowName] = []() { return new T; };
    }

    std::vector<std::string> listRegisteredWindows();

    bool navigateToWindow(ApplicationContext& context, const std::string& windowName);

    class Window {
    private:
        std::string m_errorString;

    public:
        virtual ~Window() = default;

        virtual void render(ApplicationContext& context) = 0;

        inline void setError(const std::string& error) { m_errorString = error; }

        inline const char* getError() const { return m_errorString.c_str(); }

        inline bool hasError() const { return !m_errorString.empty(); }

        inline void clearError() { m_errorString.clear(); }
    };
}  // namespace UI

#endif