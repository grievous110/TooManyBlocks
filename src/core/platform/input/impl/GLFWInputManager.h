#ifndef TOOMANYBLOCKS_GLFWINPUTMANAGER_H
#define TOOMANYBLOCKS_GLFWINPUTMANAGER_H

#include "platform/input/InputManager.h"

class GLFWInputManager : public InputManager {
private:
    KeyObservable m_keyObservable;
    MouseObservable m_mouseObservable;

public:
    virtual ~GLFWInputManager() = default;

    virtual void setup() override;

    inline virtual KeyObservable* keyAdapter() override { return &m_keyObservable; }

    inline virtual MouseObservable* mouseAdapter() override { return &m_mouseObservable; }
};

#endif
