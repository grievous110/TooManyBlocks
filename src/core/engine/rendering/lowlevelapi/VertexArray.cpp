#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "VertexBufferLayout.h"
#include "engine/rendering/Renderer.h"

unsigned int VertexArray::currentlyBoundVAO = 0;

constexpr bool isIntegerBased(unsigned int type) {
	return type == GL_INT || type == GL_UNSIGNED_INT || type == GL_SHORT || type == GL_UNSIGNED_SHORT || type == GL_BYTE || type == GL_UNSIGNED_BYTE;
}

VertexArray::VertexArray() {
	GLCALL(glGenVertexArrays(1, &m_rendererId));
}

VertexArray::~VertexArray() {
	unbind();
	GLCALL(glDeleteVertexArrays(1, &m_rendererId));
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
			GLCALL(glVertexAttribPointer(i, element.count, element.type, element.normalized ? GL_TRUE : GL_FALSE, layout.stride(), (const void*)(uintptr_t) offset));
		}
		offset += element.count * element.typeSize;
	}
}

void VertexArray::bind() const {
	if (VertexArray::currentlyBoundVAO != m_rendererId) {
		GLCALL(glBindVertexArray(m_rendererId));
		VertexArray::currentlyBoundVAO = m_rendererId;
	}
}

void VertexArray::unbind() const {
	if (VertexArray::currentlyBoundVAO == m_rendererId) {
		GLCALL(glBindVertexArray(0));
		VertexArray::currentlyBoundVAO = 0;
	}
}
