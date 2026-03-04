#ifndef TOOMANYBLOCKS_APPLICATION_H
#define TOOMANYBLOCKS_APPLICATION_H

#include <stddef.h>
#include "platform/input/InputManager.h"
#include "platform/window/WindowManager.h"
#include "foundation/util/SystemMetrics.h"

#ifndef APP_NAME
#define APP_NAME "Unspecified"
#endif

class CPUAssetProvider;
class Application;
class Renderer;
class AudioEngine;
class GameInstance;
class ThreadPool;

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
    CPUAssetProvider* provider;
    Renderer* renderer;
    AudioEngine* audioEngine;
    GameInstance* instance;
    WindowManager* windowManager;
    InputManager* inputManager;
};

class Application {
private:
    void updateStats(float deltaTime);
    
    void createContext();
    void deleteContext();

protected:
    static ApplicationContext* currentContext;

    virtual void init();
    virtual void execute();
    virtual void shutdown();

public:
    static ApplicationContext* getContext();

    void run();
};

#endif
