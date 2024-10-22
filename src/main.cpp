#include "Application.h"
#include "Logger.h"
#include <string>

#if defined(WIN_MAIN) && defined(NDEBUG) 
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	try {
		Application app;
		app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
	return 0;
}
#else
int main(void) {
	try {
		Application app;
		app.run();
	} catch (const std::exception& e) {
		lgr::lout.error("Unaught error occured: " + std::string(e.what()));
		return -1;
	}
	return 0;
}
#endif