#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/Renderer.h"
#include "Logger.h"

unsigned int IndexBuffer::currentlyBoundIBO = 0;

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : m_count(count) {
	// Index Buffer Object (IBO)
	GLCALL(glGenBuffers(1, &m_rendererId));
	GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererId));
	GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept : RenderApiObject(std::move(other)), m_count(other.m_count) {
	other.m_count = 0;
}

IndexBuffer::~IndexBuffer() {
	if (m_rendererId != 0) {
		try {
			unbind();
			GLCALL(glDeleteBuffers(1, &m_rendererId));
		} catch (const std::exception& e) {
			lgr::lout.error("Error during IndexBuffer cleanup");
		}
	}
}

void IndexBuffer::bind() const {
	if (m_rendererId == 0)
		throw std::runtime_error("Invalid state of IndexBuffer with id 0");

	if (IndexBuffer::currentlyBoundIBO != m_rendererId) {
		GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererId));
		IndexBuffer::currentlyBoundIBO = m_rendererId;
	}
}

void IndexBuffer::unbind() const {
	if (IndexBuffer::currentlyBoundIBO == m_rendererId) {
		GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		IndexBuffer::currentlyBoundIBO = 0;
	}
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
    if (this != &other) {
		if (m_rendererId != 0) {
		try {
			unbind();
			GLCALL(glDeleteBuffers(1, &m_rendererId));
		} catch (const std::exception& e) {
			lgr::lout.error("Error during IndexBuffer cleanup");
		}
	}
		RenderApiObject::operator=(std::move(other));
		m_count = other.m_count;
		
		other.m_count = 0;
	}
    return *this;
}
