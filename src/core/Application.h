#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine/KeyObserver.h"
#include "engine/MouseObserver.h"
#include "engine/ui/Ui.h"

struct GLFWwindow;
class Provider;
class Application;
class Renderer;
class GameInstance;
class FontPool;
class AppIO;
class ThreadPool;

struct ApplicationContext {
	unsigned int screenWidth;
	unsigned int screenHeight;
	
	ThreadPool* workerPool;
	GLFWwindow* window;
	Provider* provider;
	Renderer* renderer;
	GameInstance* instance;
	UI::Window* currentWindow;
	FontPool* fontPool;
	AppIO* io;
};

class AppIO : public MouseObservable, public KeyObservable {
private:
	virtual ~AppIO() = default;

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
	static void setCurrentContext(ApplicationContext* context);
	static ApplicationContext* createContext();
	static void deleteCurrentContext();
	static ApplicationContext* getContext();

	virtual void run();
};

#endif