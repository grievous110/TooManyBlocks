#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "RenderApiObject.h"

class VertexBuffer : virtual public RenderApiObject {
private:
	static unsigned int currentlyBoundVBO;
public:
	VertexBuffer(const void* data, int size);
	~VertexBuffer();

	void bind() const;
	void unbind() const;
};

#endif