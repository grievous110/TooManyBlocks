#include "ShaderStorageBuffer.h"

#include <GL/glew.h>

#include <stdexcept>

#include "Logger.h"
#include "engine/rendering/GLUtils.h"

thread_local unsigned int ShaderStorageBuffer::currentlyBoundSSBO = 0;

ShaderStorageBuffer::ShaderStorageBuffer(const void* data, size_t size) : m_size(size) {
    GLCALL(glGenBuffers(1, &m_rendererId));
    bind();
    GLCALL(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW));
}

void ShaderStorageBuffer::bindDefault() {
    if (ShaderStorageBuffer::currentlyBoundSSBO != 0) {
        GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
        ShaderStorageBuffer::currentlyBoundSSBO = 0;
    }
}

ShaderStorageBuffer ShaderStorageBuffer::create(const void* data, size_t size) {
    return ShaderStorageBuffer(data, size);
}

ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept
    : RenderApiObject(std::move(other)), m_size(other.m_size) {}

ShaderStorageBuffer::~ShaderStorageBuffer() {
    if (isValid()) {
        try {
            if (ShaderStorageBuffer::currentlyBoundSSBO == m_rendererId) {
                GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                ShaderStorageBuffer::currentlyBoundSSBO = 0;
            }
            GLCALL(glDeleteBuffers(1, &m_rendererId));
        } catch (const std::exception&) {
            lgr::lout.error("Error during ShaderStorageBuffer cleanup");
        }
    }
}

void ShaderStorageBuffer::updateData(const void* data, size_t size, size_t offset) const {
    bind();

    if (offset + size > m_size) throw std::runtime_error("SSBO update exceeds buffer size");

    GLCALL(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data));
}

void ShaderStorageBuffer::bind() const {
    if (ShaderStorageBuffer::currentlyBoundSSBO != m_rendererId) {
        GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_rendererId));
        ShaderStorageBuffer::currentlyBoundSSBO = m_rendererId;
    }
}

void ShaderStorageBuffer::assignTo(unsigned int bindingPoint) const {
    if (!isValid()) throw std::runtime_error("Invalid state of ShaderStorageBuffer with id 0");

    GLCALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_rendererId));
}

ShaderStorageBuffer& ShaderStorageBuffer::operator=(ShaderStorageBuffer&& other) noexcept {
    if (this != &other) {
        if (isValid()) {
            try {
                if (ShaderStorageBuffer::currentlyBoundSSBO == m_rendererId) {
                    GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
                    ShaderStorageBuffer::currentlyBoundSSBO = 0;
                }
                GLCALL(glDeleteBuffers(1, &m_rendererId));
            } catch (const std::exception&) {
                lgr::lout.error("Error during ShaderStorageBuffer cleanup");
            }
        }

        m_rendererId = other.m_rendererId;
        m_size = other.m_size;
    }
    return *this;
}