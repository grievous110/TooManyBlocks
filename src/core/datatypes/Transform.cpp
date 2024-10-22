#include "datatypes/Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

void Transform::recalculateModelMatrix() const {
    m_modelMatrix = glm::mat4(1.0f);

    m_modelMatrix = glm::translate(m_modelMatrix, m_position) *
                    glm::mat4_cast(m_rotationQuat) *
                    glm::scale(m_modelMatrix, m_scale);

    m_dirty = false;
}

void Transform::updateEulerFromQuat() {
    m_eulerAngles = glm::degrees(glm::eulerAngles(m_rotationQuat));
    normalizeEuler();
}

void Transform::normalizeEuler() {
    m_eulerAngles.x = normalizeAngle(m_eulerAngles.x);
    m_eulerAngles.y = normalizeAngle(m_eulerAngles.y);
    m_eulerAngles.z = normalizeAngle(m_eulerAngles.z);
}

float Transform::normalizeAngle(float degrees) {
    degrees = fmod(degrees, 360.0f);
    if (degrees < -180.0f) degrees += 360.0f;
    if (degrees >= 180.0f) degrees -= 360.0f;
    return degrees;
}

void Transform::updateQuatFromEuler() {
    normalizeEuler();
    m_rotationQuat = glm::quat(glm::radians(m_eulerAngles));
    m_rotationQuat = glm::normalize(m_rotationQuat);
}

Transform::Transform()
    : m_modelMatrix(1.0f),
    m_position(0.0f),
    m_eulerAngles(0.0f),
    m_rotationQuat(glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f))),
    m_scale(1.0f),
    m_dirty(true) {}

Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
    : m_modelMatrix(1.0f),
    m_position(position),
    m_eulerAngles(rotation),
    m_rotationQuat(glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f))),
    m_scale(scale),
    m_dirty(true) {

    updateQuatFromEuler();
}

Transform::Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
    : m_modelMatrix(1.0f),
    m_position(position),
    m_eulerAngles(0.0f),
    m_rotationQuat(rotation),
    m_scale(scale),
    m_dirty(true) {

    updateEulerFromQuat();
}

void Transform::rotate(const float& angle, const glm::vec3& axis) {
    glm::quat rotationQuat = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
    m_rotationQuat = rotationQuat * m_rotationQuat;
    updateEulerFromQuat();  // Keep Euler angles in sync
    m_dirty = true;
}

void Transform::rotate(const glm::vec3& eulerAngles) {
    glm::quat rotationQuat = glm::quat(glm::radians(eulerAngles));
    m_rotationQuat = rotationQuat * m_rotationQuat;
    updateEulerFromQuat();  // Keep Euler angles in sync
    m_dirty = true;
}

void Transform::rotate(const glm::quat& quaternion) {
    m_rotationQuat = quaternion * m_rotationQuat;
    updateEulerFromQuat();  // Keep Euler angles in sync
    m_dirty = true;
}

void Transform::translate(const glm::vec3& offset) {
    m_position += offset;
    m_dirty = true;
}

void Transform::scale(float uniformScale) {
    m_scale *= glm::vec3(uniformScale);
    m_dirty = true;
}

void Transform::lookAt(const glm::vec3& target, const glm::vec3& up) {
    glm::mat4 lookAtMatrix = glm::lookAt(m_position, target, up);
    m_rotationQuat = glm::quat_cast(lookAtMatrix);
    m_dirty = true;
}

void Transform::reset() {
    m_position = glm::vec3(0.0f);
    m_eulerAngles = glm::vec3(0.0f);
    m_rotationQuat = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
    m_scale = glm::vec3(1.0f);
    m_dirty = true;
}

void Transform::setPosition(const glm::vec3& position) {
    m_position = position;
    m_dirty = true;
}

void Transform::setRotation(const glm::quat& rotation) {
    m_rotationQuat = rotation;
    updateEulerFromQuat();
    m_dirty = true;
}

void Transform::setRotation(const glm::vec3& eulerAngles) {
    m_eulerAngles = eulerAngles;
    updateQuatFromEuler();
    m_dirty = true;
}

void Transform::setScale(const glm::vec3& scale) {
    m_scale = scale;
    m_dirty = true;
}

glm::vec3 Transform::getPosition() const {
    return m_position;
}

glm::quat Transform::getRotationQuat() const {
    return m_rotationQuat;
}

glm::vec3 Transform::getRotationEuler() const {
    return m_eulerAngles;
}

glm::vec3 Transform::getScale() const {
    return m_scale;
}

glm::vec3 Transform::getForward() const {
    return glm::normalize(m_rotationQuat * glm::vec3(0.0f, 0.0f, -1.0f)); // Forward is -Z in local space
}

glm::vec3 Transform::getRight() const {
    return glm::normalize(m_rotationQuat * glm::vec3(1.0f, 0.0f, 0.0f)); // Right is +X in local space
}

glm::vec3 Transform::getUp() const {
    return glm::normalize(m_rotationQuat * glm::vec3(0.0f, 1.0f, 0.0f));  // Up is +Y in local space
}

Transform Transform::getInverse() const {
    Transform result;
    result.setPosition(-m_position);
    result.setRotation(glm::inverse(m_rotationQuat));
    result.setScale(1.0f / m_scale);
    return result;
}

glm::mat4 Transform::getModelMatrix() const {
    if (m_dirty) {
        recalculateModelMatrix();
    }
    return m_modelMatrix;
}

Transform Transform::lerp(const Transform& other, float t) const {
    Transform result;
    result.setPosition(glm::mix(this->getPosition(), other.getPosition(), t));
    result.setScale(glm::mix(this->getScale(), other.getScale(), t));
    result.setRotation(glm::slerp(this->getRotationQuat(), other.getRotationQuat(), t)); // Spherical linear interpolation for quaternions
    return result;
}

Transform Transform::operator*(const Transform& other) const {
    Transform result;
    result.setPosition(this->getPosition() + other.getPosition());
    result.setRotation(this->getRotationQuat() * other.getRotationQuat());
    result.setScale(this->getScale() * other.getScale());
    return result;
}

Transform& Transform::operator*=(const Transform& other) {
    this->translate(other.getPosition());
    this->rotate(other.getRotationQuat());
    this->setScale(this->getScale() * other.getScale());
    return *this;
}

bool Transform::operator==(const Transform& other) const {
    return (m_position == other.m_position &&
        m_rotationQuat == other.m_rotationQuat &&
        m_scale == other.m_scale);
}

bool Transform::operator!=(const Transform& other) const {
    return !(*this == other);
}