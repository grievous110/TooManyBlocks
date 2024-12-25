#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "RenderApiObject.h"
#include "VertexBufferLayout.h"

class VertexArray : public RenderApiObject {
private:
	static unsigned int currentlyBoundVAO;

public:
	VertexArray();
	VertexArray(VertexArray&& other) noexcept;
	virtual ~VertexArray();

	void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	void bind() const;
	void unbind() const;

    VertexArray& operator=(VertexArray&& other) noexcept;
};

#endif