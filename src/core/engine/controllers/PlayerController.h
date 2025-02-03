#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "engine/controllers/Controller.h"
#include "engine/KeyObserver.h"
#include "engine/MouseObserver.h"
#include <unordered_map>

class PlayerController : public Controller, public KeyObserver, public MouseObserver {
private:
    std::unordered_map<int, bool> keyStates;
    float m_cameraPitch = 0.0f;
    
public:
    PlayerController();
    virtual ~PlayerController();

    void notify(KeyEvent event, KeyEventData data) override;

	void notify(MousEvent event, MouseEventData data) override;
    
    void update(float msDelta) override;
};

#endif