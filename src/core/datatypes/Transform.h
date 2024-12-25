#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
private:
    glm::vec3 m_position;
    glm::vec3 m_eulerAngles;
    glm::quat m_rotationQuat;
    glm::vec3 m_scale;

    mutable bool m_dirty; // Tracks whether the model matrix needs recalculation
    mutable glm::mat4 m_modelMatrix;

    void recalculateModelMatrix() const;
    void updateEulerFromQuat();
    void updateQuatFromEuler();
    void normalizeEuler();
    float normalizeAngle(float degrees);

public:
    Transform();
    Transform(const glm::vec3& position, const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f));
    Transform(const glm::vec3& position, const glm::quat& rotation = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f)), const glm::vec3& scale = glm::vec3(1.0f));

    void rotate(const float& angle, const glm::vec3& axis);
    void rotate(const glm::vec3& eulerAngles);
    void rotate(const glm::quat& quaternion);
    void translate(const glm::vec3& offset);
    void scale(float uniformScale);
    void lookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));
    void reset();

    void setPosition(const glm::vec3& position);
    void setRotation(const glm::quat& rotation);
    void setRotation(const glm::vec3& eulerAngles);
    void setScale(const glm::vec3& scale);

    glm::vec3 getPosition() const;
    glm::quat getRotationQuat() const;
    glm::vec3 getRotationEuler() const;
    glm::vec3 getScale() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;
    Transform getInverse() const;

    glm::mat4 getModelMatrix() const;

    Transform lerp(const Transform& other, float t) const;

    Transform operator*(const Transform& other) const;
    Transform& operator*=(const Transform& other);
    bool operator==(const Transform& other) const;
    bool operator!=(const Transform& other) const;
};

#endif