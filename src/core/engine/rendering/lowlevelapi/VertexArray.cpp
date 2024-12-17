#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "VertexBufferLayout.h"
#include "engine/rendering/Renderer.h"

unsigned int VertexArray::currentlyBoundVAO = 0;

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
	const auto& elements = layout.elements();
	size_t offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++) {
		const BufferLayoutElement& element = elements[i];
		GLCALL(glEnableVertexAttribArray(i));
		// TODO: Generalize for all
		//GLCALL(glVertexAttribPointer(i, element.count, element.type, element.normalized ? GL_TRUE : GL_FALSE, layout.stride(), (const void*)(uintptr_t) offset));
		GLCALL(glVertexAttribIPointer(i, element.count, element.type, layout.stride(), (const void*)(uintptr_t) offset));
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
