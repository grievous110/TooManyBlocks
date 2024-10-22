#include "Application.h"
#include "Ui.h"

namespace UI {
	WindowFactory& getWindowFactory() {
		static WindowFactory factory;
		return factory;
	}

	std::vector<std::string> listRegisteredWindows() {
		std::vector<std::string> list;
		for (const auto& pair : getWindowFactory()) {
			list.push_back(pair.first);
		}
		return list;
	}

	bool navigateToWindow(ApplicationContext& context, const std::string& windowName) {
		WindowFactory& factory = getWindowFactory();
		auto it = factory.find(windowName);
		if (it != factory.end()) {
			if (context.currentWindow) {
				delete context.currentWindow;
			}
			context.currentWindow = it->second();  // Call the stored factory function
			return true;
		}
		return false;
	}
}