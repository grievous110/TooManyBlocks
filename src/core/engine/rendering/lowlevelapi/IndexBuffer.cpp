#include "engine/rendering/lowlevelapi/IndexBuffer.h"
#include "engine/rendering/Renderer.h"

unsigned int IndexBuffer::currentlyBoundIBO = 0;

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : m_count(count) {
	// Index Buffer Object (IBO)
	GLCALL(glGenBuffers(1, &m_rendererId));
	GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererId));
	GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer() {
	unbind();
	GLCALL(glDeleteBuffers(1, &m_rendererId));
}

void IndexBuffer::bind() const {
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
