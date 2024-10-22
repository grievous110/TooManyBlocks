#ifndef UIWINDOW_H
#define UIWINDOW_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

struct ApplicationContext;

namespace UI {
	class Window;
	using WindowFactory = std::unordered_map<std::string, std::function<Window*()>>;

	WindowFactory& getWindowFactory();

	template<typename T>
	void registerWindow(const std::string& windowName) {
		getWindowFactory()[windowName] = []() {
			return new T;
		};
	}

	std::vector<std::string> listRegisteredWindows();

	bool navigateToWindow(ApplicationContext& context, const std::string& windowName);

	class Window {
	public:
		virtual void render(ApplicationContext& context) = 0;
		virtual ~Window() = default;
	};
}

#endif