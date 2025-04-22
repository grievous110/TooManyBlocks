#include "IndexBuffer.h"

#include <gl/glew.h>

#include "Logger.h"
#include "engine/rendering/GLUtils.h"

thread_local unsigned int IndexBuffer::currentlyBoundIBO = 0;

void IndexBuffer::bindDefault() {
    if (IndexBuffer::currentlyBoundIBO != 0) {
        GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        IndexBuffer::currentlyBoundIBO = 0;
    }
}

void IndexBuffer::syncBinding() {
    int binding;
    GLCALL(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &binding));
    IndexBuffer::currentlyBoundIBO = static_cast<unsigned int>(binding);
}

IndexBuffer::IndexBuffer(const unsigned int* data, size_t count) : m_count(count) {
    // Index Buffer Object (IBO)
    GLCALL(glGenBuffers(1, &m_rendererId));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererId));
    GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(unsigned int), data, GL_STATIC_DRAW));
    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer::currentlyBoundIBO));
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept : RenderApiObject(std::move(other)), m_count(other.m_count) {
    other.m_count = 0;
}

IndexBuffer::~IndexBuffer() {
    if (m_rendererId != 0) {
        try {
            if (IndexBuffer::currentlyBoundIBO == m_rendererId) {
                GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                IndexBuffer::currentlyBoundIBO = 0;
            }
            GLCALL(glDeleteBuffers(1, &m_rendererId));
        } catch (const std::exception&) {
            lgr::lout.error("Error during IndexBuffer cleanup");
        }
    }
}

void IndexBuffer::bind() const {
    if (m_rendererId == 0) throw std::runtime_error("Invalid state of IndexBuffer with id 0");

    if (IndexBuffer::currentlyBoundIBO != m_rendererId) {
        GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererId));
        IndexBuffer::currentlyBoundIBO = m_rendererId;
    }
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
    if (this != &other) {
        if (m_rendererId != 0) {
            try {
                if (IndexBuffer::currentlyBoundIBO == m_rendererId) {
                    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                    IndexBuffer::currentlyBoundIBO = 0;
                }
                GLCALL(glDeleteBuffers(1, &m_rendererId));
            } catch (const std::exception&) {
                lgr::lout.error("Error during IndexBuffer cleanup");
            }
        }
        RenderApiObject::operator=(std::move(other));
        m_count = other.m_count;

        other.m_count = 0;
    }
    return *this;
}
