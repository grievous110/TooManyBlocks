#include "engine/controllers/Controller.h"
#include "engine/entity/Entity.h"
#include "Controller.h"

Controller::~Controller() {
    unpossess();
}

void Controller::possess(Entity *entity) {
    unpossess();
    m_possessedEntity = entity;
    m_possessedEntity->m_controller = this;
}

void Controller::unpossess() {
    if(m_possessedEntity) {
        m_possessedEntity->m_controller = nullptr;
        m_possessedEntity = nullptr;
    }
}