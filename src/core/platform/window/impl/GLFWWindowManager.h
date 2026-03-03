#ifndef TOOMANYBLOCKS_GLFWWINDOWMANAGER_H
#define TOOMANYBLOCKS_GLFWWINDOWMANAGER_H

#include "platform/window/WindowManager.h"

struct GLFWwindow;

class GLFWWindowManager : public WindowManager {
private:
    GLFWwindow* m_currentWindow;
    WindowMode m_currentMode;
    bool m_isInitialized;

    // State before exiting windowed mode
    int m_windowedX;
    int m_windowedY;
    int m_windowedWidth;
    int m_windowedHeight;

public:
    GLFWWindowManager();
    virtual ~GLFWWindowManager();

    virtual void init() override;

    virtual void shutdown() override;

    virtual void createActiveWindow(unsigned int width, unsigned int height, const char* title) override;
    
    virtual bool shouldWindowClose() const override;
    
    virtual void setWindowSize(unsigned int width, unsigned int height) override;

    virtual void setWindowPosition(int x, int y) override;

    virtual void swapBuffers() override;

    virtual void pollEvents() override;

    virtual void enableVSync(bool enabled) override;

    virtual void setCursorMode(CursorMode mode) override;

    virtual std::vector<MonitorInfo> getAvailableMonitors() const override;

    virtual void setWindowMode(WindowMode mode, int monitorId = -1) override;
};

#endif
