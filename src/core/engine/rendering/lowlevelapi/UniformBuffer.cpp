#include "engine/rendering/GLUtils.h"
#include "Logger.h"
#include "UniformBuffer.h"
#include <gl/glew.h>
#include <stdexcept>

void UniformBuffer::bindDefault() {
    GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

UniformBuffer::UniformBuffer(const void* data, size_t size) : m_size(size) {	
	// Uniform Buffer Object (UBO)
	GLCALL(glGenBuffers(1, &m_rendererId));
	GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, m_rendererId));
	GLCALL(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW));

	GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept : RenderApiObject(std::move(other)), m_size(other.m_size) {
	other.m_size = 0;
}

UniformBuffer::~UniformBuffer() {
	if (m_rendererId != 0) {
		try {
			GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
			GLCALL(glDeleteBuffers(1, &m_rendererId));
		} catch (const std::exception&) {
			lgr::lout.error("Error during UniformBuffer cleanup");
		}
	}
}

void UniformBuffer::updateData(const void *data, size_t size, size_t offset) const {
	 if (m_rendererId == 0)
        throw std::runtime_error("Invalid state of UniformBuffer with id 0");

    if (offset + size > m_size)
        throw std::runtime_error("UBO update exceeds buffer size");

    GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, m_rendererId));
    GLCALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
}

void UniformBuffer::bind(unsigned int bindingPoint) const {
	if (m_rendererId == 0)
		throw std::runtime_error("Invalid state of UniformBuffer with id 0");

    GLCALL(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_rendererId));
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
	if (this != &other) {
		if (m_rendererId != 0) {
			try {
				GLCALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
				GLCALL(glDeleteBuffers(1, &m_rendererId));
			} catch (const std::exception&) {
				lgr::lout.error("Error during UniformBuffer cleanup");
			}
		}
		RenderApiObject::operator=(std::move(other));

		m_size = other.m_size;

		other.m_size = 0;
	}
	return *this;
}