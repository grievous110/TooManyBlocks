#ifndef TOOMANYBLOCKS_INPUTMANAGER_H
#define TOOMANYBLOCKS_INPUTMANAGER_H

#include "platform/input/KeyMouseIO.h"

class InputManager {
public:
    virtual void setup() = 0;

    virtual KeyObservable* keyAdapter() = 0;
    virtual MouseObservable* mouseAdapter() = 0;
};

#endif
