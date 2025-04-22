#include "PlayerController.h"

#include <GLFW/glfw3.h>

#include "Application.h"
#include "Logger.h"
#include "datatypes/DatatypeDefs.h"
#include "datatypes/Transform.h"
#include "engine/GameInstance.h"
#include "engine/entity/Player.h"
#include "engine/geometry/Linetrace.h"
#include "engine/rendering/Camera.h"

PlayerController::PlayerController() : m_cameraPitch(0.0f), m_playerNeedsReadjustment(true) {
    if (ApplicationContext* context = Application::getContext()) {
        Application::getContext()->io->keyAdapter().attach(this);
        Application::getContext()->io->mouseAdapter().attach(this);
    }
}

PlayerController::~PlayerController() {
    if (ApplicationContext* context = Application::getContext()) {
        Application::getContext()->io->keyAdapter().detach(this);
        Application::getContext()->io->mouseAdapter().detach(this);
    }
}

void PlayerController::notify(KeyEvent event, KeyEventData data) {
    keyStates[data.keycode] = event == KeyEvent::ButtonDown;
}

void PlayerController::notify(MousEvent event, MouseEventData data) {
    if (!keyStates[GLFW_KEY_LEFT_ALT] && event == MousEvent::Move) {
        if (Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
            Transform& tr = pl->getCamera()->getLocalTransform();

            float pitchDelta = static_cast<float>(-data.delta.y) * 0.25f;
            float yawDelta = static_cast<float>(-data.delta.x) * 0.25f;

            float newPitch = glm::clamp(m_cameraPitch + pitchDelta, -89.0f, 89.0f);
            float allowedPitchDelta = newPitch - m_cameraPitch;
            m_cameraPitch += allowedPitchDelta;

            tr.rotate(yawDelta, WorldUp);
            tr.rotate(allowedPitchDelta, tr.getRight());
        }
    } else if (event == MousEvent::ButtonDown || event == MousEvent::ButtonUp) {
        if (!keyStates[GLFW_KEY_LEFT_ALT]) {
            if (!keyStates[GLFW_MOUSE_BUTTON_LEFT] && data.key.code == GLFW_MOUSE_BUTTON_LEFT &&
                event == MousEvent::ButtonDown) {
                // Block breaking logic
                if (ApplicationContext* context = Application::getContext()) {
                    if (Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
                        if (pl->isFocusingBlock()) {
                            context->instance->m_world->setBlock(pl->getFocusedBlock(), AIR);
                        }
                    }
                }
            }
        }

        keyStates[data.key.code] = event == MousEvent::ButtonDown;
    }
}

void PlayerController::update(float deltaTime) {
    if (ApplicationContext* context = Application::getContext()) {
        if (Player* pl = dynamic_cast<Player*>(m_possessedEntity)) {
            // Check if player needs to moved up when beeing in unloeded terrain
            glm::ivec3 chunkPos = Chunk::worldToChunkOrigin(pl->getTransform().getPosition());
            Chunk* chunk = context->instance->m_world->getChunk(chunkPos);
            if (!m_playerNeedsReadjustment) {
                m_playerNeedsReadjustment = chunk == nullptr;
            }
            if (m_playerNeedsReadjustment) {
                if (chunk) {
                    for (int y = 0; y < CHUNK_HEIGHT - 1; y++) {
                        if (chunk->blocks()[chunkBlockIndex(0, y, 0)].type == AIR &&
                            chunk->blocks()[chunkBlockIndex(0, y + 1, 0)].type == AIR) {
                            pl->getTransform().setPosition(glm::vec3(0.5f, chunkPos.y + y + 0.1f, 0.5f));
                            m_playerNeedsReadjustment = false;
                            break;
                        }
                    }
                    if (m_playerNeedsReadjustment) {
                        // Move up into next chunk
                        pl->getTransform().setPosition(glm::vec3(0.5f, chunkPos.y + CHUNK_HEIGHT + 0.1f, 0.5f));
                    }
                }
            } else {
                // Movement logic
                if (!keyStates[GLFW_KEY_LEFT_ALT]) {
                    glm::vec3 dir(0.0f);
                    const Transform camGlobTransform = pl->getCamera()->getGlobalTransform();
                    const Transform& camTransform = pl->getCamera()->getLocalTransform();
                    float speed = 4.0f;
                    MovementMode currMode = pl->getMovementComponent()->getMovementMode();
                    if (keyStates[GLFW_KEY_LEFT_SHIFT] && currMode != MovementMode::Fly) {
                        speed *= 2.0f;
                    }

                    // Adjust velocity based on key states
                    if (keyStates[GLFW_KEY_W]) dir += camTransform.getForward();
                    if (keyStates[GLFW_KEY_S]) dir -= camTransform.getForward();
                    if (keyStates[GLFW_KEY_D]) dir += camTransform.getRight();
                    if (keyStates[GLFW_KEY_A]) dir -= camTransform.getRight();

                    if (currMode == MovementMode::Spectator) {
                        // Additional controls as spectator
                        if (keyStates[GLFW_KEY_SPACE]) dir += WorldUp;
                        if (keyStates[GLFW_KEY_LEFT_CONTROL]) dir -= WorldUp;
                    } else if (currMode == MovementMode::Walk) {
                        // Jump logic
                        if (keyStates[GLFW_KEY_SPACE] && pl->getMovementComponent()->isGrounded()) {
                            pl->getMovementComponent()->addImpulse(WorldUp * 7.0f);
                        }
                    }
                    if (glm::length(dir) <= 0.0f) {
                        speed = 0.0f;
                    }

                    pl->getMovementComponent()->setInputDirection(dir);
                    pl->getMovementComponent()->setMovementSpeed(speed);

                    // Block focus logic
                    HitResult result = linetraceByChannel(
                        camGlobTransform.getPosition(),
                        camGlobTransform.getPosition() + (camGlobTransform.getForward() * pl->getReachDistance()),
                        Channel::BlockTrace
                    );
                    pl->setIsFocusingBlock(result.hitSuccess);
                    if (result.hitSuccess) {
                        pl->setFocusedBlock(glm::ivec3(result.objectPosition));
                    }
                }
            }
        }
    }
}
