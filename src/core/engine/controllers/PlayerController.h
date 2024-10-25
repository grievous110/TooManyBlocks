#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "engine/controllers/Controller.h"
#include "engine/KeyObserver.h"
#include "engine/MouseObserver.h"
#include <unordered_map>

class PlayerController : public Controller, public KeyObserver, public MouseObserver {
private:
    std::unordered_map<int, bool> keyStates;
    
public:
    PlayerController();
    ~PlayerController();

    void notify(const KeyEvent& event, const KeyEventData& data) override;

	void notify(const MousEvent& event, const MouseEventData& data) override;
    
    void update(float msDelta) override;
};

#endif