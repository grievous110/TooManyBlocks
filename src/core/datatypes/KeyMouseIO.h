#ifndef TOOMANYBLOCKS_KEYMOUSEIO_H
#define TOOMANYBLOCKS_KEYMOUSEIO_H

#include <vector>

#include "engine/Observer.h"

enum class MousEvent {
    Move,
    ButtonDown,
    ButtonUp,
    Scroll
};

struct MouseEventData {
    union {
        struct {
            double x;
            double y;
        } delta;
        struct {
            int code;
        } key;
    };
};

using MouseObserver = Observer<MousEvent, MouseEventData>;
using MouseObservable = Observable<MousEvent, MouseEventData>;

enum class KeyEvent {
    ButtonDown,
    ButtonUp
};

struct KeyEventData {
    int keycode;
    int mods;
};

using KeyObserver = Observer<KeyEvent, KeyEventData>;
using KeyObservable = Observable<KeyEvent, KeyEventData>;

#endif