#ifndef TOOMANYBLOCKS_APPLICATION_H
#define TOOMANYBLOCKS_APPLICATION_H

#include <stddef.h>
#include "datatypes/KeyMouseIO.h"
#include "util/SystemMetrics.h"

struct GLFWwindow;
class Provider;
class Application;
class Renderer;
class AudioEngine;
class GameInstance;
class FontPool;
class AppIO;
class ThreadPool;
namespace UI {
    class Window;
};

struct ApplicationContext {
    struct AppState {
        unsigned int screenWidth;
        unsigned int screenHeight;
        int lastMousepositionX;
        int lastMousepositionY;
    } state;

    struct Stats {
        float deltaAppTime;
        float elapsedAppTime;

        float cpuUsage;
        uint64_t processUsedBytes;
        MemoryInfo memInfo;
        ProcessIO processIo;
        CpuTimes cpuTimes;
    } stats;

    ThreadPool* workerPool;
    GLFWwindow* window;
    Provider* provider;
    Renderer* renderer;
    AudioEngine* audioEngine;
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