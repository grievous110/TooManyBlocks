#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "RenderApiObject.h"

class VertexBufferLayout;

class VertexArray : virtual public RenderApiObject {
private:
	static unsigned int currentlyBoundVAO;

public:
	VertexArray();
	~VertexArray();

	void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	void bind() const;
	void unbind() const;
};

#endif