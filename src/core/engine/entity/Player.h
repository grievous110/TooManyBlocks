#ifndef PLAYER_H
#define PLAYER_H

#include "engine/comp/SceneComponent.h"
#include "engine/entity/Entity.h"
#include "engine/rendering/Camera.h"
#include <glm/vec3.hpp>
#include <memory>

class Player : public Entity {
private:
	std::shared_ptr<Camera> m_camera;
	glm::ivec3 m_focusedBlock;
	bool m_isFocusingBlock;
	float m_reach;

public:
	Player() : m_camera(std::make_shared<Camera>(45.0f, 950.0f / 540.0f)), m_reach(6.0f) { m_sceneRoot.attachChild(m_camera.get()); m_camera->getLocalTransform().translate(glm::vec3(0, 1.5, 0)); }
	virtual ~Player() = default;

	inline std::shared_ptr<Camera> getCamera() const { return m_camera; }

	inline void setReachDistance(float reach) { m_reach = reach; }

	inline float getReachDistance() const { return m_reach; }

	inline void setFocusedBlock(const glm::ivec3& focusedBlock) { m_focusedBlock = focusedBlock; }

	inline glm::ivec3 getFocusedBlock() const { return m_focusedBlock; }

	inline void setIsFocusingBlock(bool isBlockFocused) { m_isFocusingBlock = isBlockFocused; }

	inline bool isFocusingBlock() const { return m_isFocusingBlock; }

	void update(float msDelta) override;
};

#endif