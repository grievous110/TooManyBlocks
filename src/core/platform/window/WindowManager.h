#ifndef TOOMANYBLOCKS_WINDOWMANAGER_H
#define TOOMANYBLOCKS_WINDOWMANAGER_H

enum class CursorMode {
    Normal,
    HiddenAndCaptured
};

class WindowManager {
public:
    virtual ~WindowManager() = default;

    virtual void init() = 0;

    virtual void shutdown() = 0;

    virtual void createActiveWindow(unsigned int width, unsigned int height, const char* title) = 0;
    
    virtual bool shouldWindowClose() const = 0;
    
    virtual void swapBuffers() = 0;

    virtual void pollEvents() = 0;

    virtual void enableVSync(bool enabled) = 0;

    virtual void setCursorMode(CursorMode mode) = 0;
};

#endif
