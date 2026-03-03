#ifndef TOOMANYBLOCKS_UIWINDOW_H
#define TOOMANYBLOCKS_UIWINDOW_H

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "engine/ui/Manager.h"
#include "engine/ui/UiDialog.h"
#include "engine/ui/UiUtil.h"
#include "engine/ui/Widget.h"

namespace UI {
    extern Manager* _manager;

    template <typename T>
    void init() {
        _manager = new T;
        _manager->init();
    }

    inline void shutdown() {
        if (_manager) {
            _manager->shutdown();
            delete _manager;
            _manager = nullptr;
        }
    }

    inline void render() { _manager->renderFrame(); }

    template <typename T>
    void registerWidget(const std::string& widgetName) {
        static_assert(std::is_base_of<Widget, T>::value, "T must derive from Widget");
        _manager->registerWidget(widgetName, []() { return new T; });
    }

    inline bool navigateToWidget(const std::string& widgetName) { return _manager->navigateToWidget(widgetName); }
}  // namespace UI

#endif