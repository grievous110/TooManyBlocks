#include "engine/rendering/GLUtils.h"
#include "Logger.h"
#include "ShaderStorageBuffer.h"
#include <gl/glew.h>
#include <stdexcept>

ShaderStorageBuffer::ShaderStorageBuffer(const void* data, size_t size) : m_size(size) {
    GLCALL(glGenBuffers(1, &m_rendererId));
    GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rendererId));
    GLCALL(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW));
    GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
    if (m_rendererId != 0) {
        try {
            GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
            GLCALL(glDeleteBuffers(1, &m_rendererId));
        } catch (const std::exception&) {
            lgr::lout.error("Error during ShaderStorageBuffer cleanup");
        }
    }
}

ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept : RenderApiObject(std::move(other)), m_size(other.m_size) {
    other.m_size = 0;
}

void ShaderStorageBuffer::updateData(const void* data, size_t size, size_t offset) const {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of ShaderStorageBuffer with id 0");

    if (offset + size > m_size)
        throw std::runtime_error("SSBO update exceeds buffer size");

    GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rendererId));
    GLCALL(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data));
    GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

void ShaderStorageBuffer::bind(unsigned int bindingPoint) const {
    if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of ShaderStorageBuffer with id 0");

    GLCALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_rendererId));
}

ShaderStorageBuffer& ShaderStorageBuffer::operator=(ShaderStorageBuffer&& other) noexcept {
    if (this != &other) {
        if (m_rendererId != 0) {
            try {
				GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                GLCALL(glDeleteBuffers(1, &m_rendererId));
			} catch (const std::exception&) {
				lgr::lout.error("Error during ShaderStorageBuffer cleanup");
			}
        }

        m_rendererId = other.m_rendererId;
        m_size = other.m_size;

        other.m_rendererId = 0;
        other.m_size = 0;
    }
    return *this;
}