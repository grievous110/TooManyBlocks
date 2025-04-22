#ifndef APPLICATION_H
#define APPLICATION_H

#include "datatypes/KeyMouseIO.h"
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
    int lastMousepositionX;
    int lastMousepositionY;

    ThreadPool* workerPool;
    GLFWwindow* window;
    Provider* provider;
    Renderer* renderer;
    GameInstance* instance;
    UI::Window* currentWindow;
    UI::Window* nextWindow;
    FontPool* fontPool;
    AppIO* io;
};

class AppIO {
private:
    KeyObservable m_keyObs;
    MouseObservable m_mouseObs;

    AppIO() = default;
    virtual ~AppIO() = default;

    friend Application;

public:
    inline KeyObservable& keyAdapter() { return m_keyObs; };
    inline MouseObservable& mouseAdapter() { return m_mouseObs; };
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