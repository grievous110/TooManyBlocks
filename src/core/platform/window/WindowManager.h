#ifndef TOOMANYBLOCKS_WINDOWMANAGER_H
#define TOOMANYBLOCKS_WINDOWMANAGER_H

#include <string>
#include <vector>

enum class CursorMode {
    Normal,
    HiddenAndCaptured
};

enum class WindowMode {
    Windowed,
    Borderless,
    Fullscreen
};

struct MonitorInfo {
    int id;
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    int refreshRate;
    std::string name;
};

class WindowManager {
public:
    virtual ~WindowManager() = default;

    virtual void init() = 0;

    virtual void shutdown() = 0;

    virtual void createActiveWindow(unsigned int width, unsigned int height, const char* title) = 0;

    virtual bool shouldWindowClose() const = 0;

    virtual void setWindowSize(unsigned int width, unsigned int height) = 0;

    virtual void setWindowPosition(int x, int y) = 0;

    virtual void swapBuffers() = 0;

    virtual void pollEvents() = 0;

    virtual void enableVSync(bool enabled) = 0;

    virtual void setCursorMode(CursorMode mode) = 0;

    virtual std::vector<MonitorInfo> getAvailableMonitors() const = 0;

    virtual void setWindowMode(WindowMode mode, int monitorId = -1) = 0;
};

#endif
