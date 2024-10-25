#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "engine/Updatable.h"

class Entity;

class Controller : public Updatable {
protected:
    Entity* m_possessedEntity;

public:
    Controller();
    ~Controller();

    void possess(Entity* entity);
    void unpossess();
};

#endif