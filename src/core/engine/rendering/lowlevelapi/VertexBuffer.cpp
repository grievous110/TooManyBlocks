#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/Renderer.h"
#include "Logger.h"

unsigned int VertexBuffer::currentlyBoundVBO = 0;

VertexBuffer::VertexBuffer(const void* data, int size) {
	// Vertex Buffer Object (VBO)
	GLCALL(glGenBuffers(1, &m_rendererId));
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_rendererId));
	GLCALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept : RenderApiObject(std::move(other)) {}

VertexBuffer::~VertexBuffer() {
	if (m_rendererId != 0) {
		try {
			unbind();
			GLCALL(glDeleteBuffers(1, &m_rendererId));
		} catch (const std::exception& e) {
			lgr::lout.error("Error during VertexBuffer cleanup");
		}
	}
}

void VertexBuffer::bind() const {
	if (m_rendererId == 0)
		throw std::runtime_error("Invalid state of VertexBuffer with id 0");
	
	if (VertexBuffer::currentlyBoundVBO != m_rendererId) {
		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_rendererId));
		VertexBuffer::currentlyBoundVBO = m_rendererId;
	}
}

void VertexBuffer::unbind() const {
	if (VertexBuffer::currentlyBoundVBO == m_rendererId) {
		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		VertexBuffer::currentlyBoundVBO = 0;
	}
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
    if (this != &other) {
		if (m_rendererId != 0) {
			try {
				unbind();
				GLCALL(glDeleteBuffers(1, &m_rendererId));
			} catch (const std::exception& e) {
				lgr::lout.error("Error during VertexBuffer cleanup");
			}
		}
		RenderApiObject::operator=(std::move(other));
	}
    return *this;
}