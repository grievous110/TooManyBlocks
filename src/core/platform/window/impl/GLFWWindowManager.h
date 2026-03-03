#ifndef TOOMANYBLOCKS_GLFWWINDOWMANAGER_H
#define TOOMANYBLOCKS_GLFWWINDOWMANAGER_H

#include "platform/window/WindowManager.h"

struct GLFWwindow;

class GLFWWindowManager : public WindowManager {
private:
    GLFWwindow* m_currentWindow;
    bool m_isInitialized;

public:
    GLFWWindowManager();
    virtual ~GLFWWindowManager();

    virtual void init() override;

    virtual void shutdown() override;

    virtual void createActiveWindow(unsigned int width, unsigned int height, const char* title) override;
    
    virtual bool shouldWindowClose() const override;
    
    virtual void swapBuffers() override;

    virtual void pollEvents() override;

    virtual void enableVSync(bool enabled) override;

    virtual void setCursorMode(CursorMode mode) override;
};

#endif
