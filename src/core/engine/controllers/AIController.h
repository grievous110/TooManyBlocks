#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include "engine/controllers/Controller.h"

class AIController : public Controller {
public:
    AIController() = default;
    virtual ~AIController() = default;
};

#endif