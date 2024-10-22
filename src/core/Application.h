#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine/KeyObserver.h"
#include "engine/MouseObserver.h"
#include "engine/ui/Ui.h"

struct GLFWwindow;
class Application;
class Renderer;
class GameInstance;
class AppIO;

struct ApplicationContext {
	GLFWwindow* window;
	Renderer* renderer;
	GameInstance* instance;
	UI::Window* currentWindow;
	AppIO* io;
};

class AppIO : public MouseObservable, public KeyObservable {
private:
	AppIO() {};

	friend Application;

public:
	using MouseObservable::notifyObservers;
	using MouseObservable::attach;
	using MouseObservable::detach;
	using KeyObservable::notifyObservers;
	using KeyObservable::attach;
	using KeyObservable::detach;
};

class Application {
private:
	static ApplicationContext* currentContext;

public:
	static void setCurrentContext(ApplicationContext* currentContext);
	static ApplicationContext* createContext();
	static void deleteCurrentContext();
	static ApplicationContext* getContext();

	virtual void run();
};

#endif