#include "Application.h"
#include "Ui.h"

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
			// Delete current window if set (Remember to set to nullptr on initialization)
			if (context.currentWindow) {
				delete context.currentWindow;
			}
			// Call the stored factory function
			context.currentWindow = it->second();
			return true;
		}
		return false;
	}
}