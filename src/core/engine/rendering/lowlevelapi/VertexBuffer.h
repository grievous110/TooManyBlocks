#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "RenderApiObject.h"

class VertexBuffer : virtual public RenderApiObject {
private:
	static unsigned int currentlyBoundVBO;
	
public:
	VertexBuffer(const void* data, int size); // size is in bytes
	VertexBuffer(VertexBuffer&& other) noexcept;
	~VertexBuffer();

	void bind() const;
	void unbind() const;

	VertexBuffer& operator=(VertexBuffer&& other) noexcept;
};

#endif