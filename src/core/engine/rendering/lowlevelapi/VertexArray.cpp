#include "engine/rendering/GLUtils.h"
#include "Logger.h"
#include "VertexArray.h"
#include <gl/glew.h>

thread_local unsigned int VertexArray::currentlyBoundVAO = 0;

constexpr bool isIntegerBased(unsigned int type) {
	return type == GL_INT || type == GL_UNSIGNED_INT || type == GL_SHORT || type == GL_UNSIGNED_SHORT || type == GL_BYTE || type == GL_UNSIGNED_BYTE;
}

void VertexArray::bindDefault() {
	if (VertexArray::currentlyBoundVAO != 0) {
		GLCALL(glBindVertexArray(0));
		VertexArray::currentlyBoundVAO = 0;
	}
}

void VertexArray::syncBinding() {
	int binding;
	GLCALL(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &binding));
	VertexArray::currentlyBoundVAO = static_cast<unsigned int>(binding);
}

VertexArray::VertexArray() {
    GLCALL(glGenVertexArrays(1, &m_rendererId));
}

VertexArray::VertexArray(VertexArray&& other) noexcept : RenderApiObject(std::move(other)) {}

VertexArray::~VertexArray() {
	if (m_rendererId != 0) {
		try {
			if (VertexArray::currentlyBoundVAO == m_rendererId) {
				GLCALL(glBindVertexArray(0));
				VertexArray::currentlyBoundVAO = 0;
			}
			GLCALL(glDeleteVertexArrays(1, &m_rendererId));
		} catch (const std::exception&) {
			lgr::lout.error("Error during VertexArray cleanup");
		}
	}
}

void VertexArray::addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) {
	bind();
	vb.bind();
	const std::vector<BufferLayoutElement>& elements = layout.elements();
	size_t offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++) {
		const BufferLayoutElement& element = elements[i];
		GLCALL(glEnableVertexAttribArray(i));
		if (isIntegerBased(element.type)) {
			GLCALL(glVertexAttribIPointer(i, element.count, element.type, layout.stride(), (const void*) offset));
		} else {
			GLCALL(glVertexAttribPointer(i, element.count, element.type, element.normalized ? GL_TRUE : GL_FALSE, layout.stride(), (const void*) offset));
		}
		offset += element.count * element.typeSize;
	}
}

void VertexArray::bind() const {
	if (m_rendererId == 0)
		throw std::runtime_error("Invalid state of VertexArray with id 0");

	if (VertexArray::currentlyBoundVAO != m_rendererId) {
		GLCALL(glBindVertexArray(m_rendererId));
		VertexArray::currentlyBoundVAO = m_rendererId;
	}
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
	if (this != &other) {
		if (m_rendererId != 0) {
			try {
				if (VertexArray::currentlyBoundVAO == m_rendererId) {
					GLCALL(glBindVertexArray(0));
					VertexArray::currentlyBoundVAO = 0;
				}
				GLCALL(glDeleteVertexArrays(1, &m_rendererId));
			} catch (const std::exception&) {
				lgr::lout.error("Error during VertexArray cleanup");
			}
		}
		RenderApiObject::operator=(std::move(other));
	}
    return *this;
}