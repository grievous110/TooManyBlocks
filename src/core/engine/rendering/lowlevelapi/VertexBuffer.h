#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"

class VertexBuffer : public RenderApiObject {
private:
	static unsigned int currentlyBoundVBO;
	int m_size;
	
public:
	VertexBuffer(const void* data, int size); // size is in bytes
	VertexBuffer(VertexBuffer&& other) noexcept;
	virtual ~VertexBuffer();

	inline int getByteSize() const { return m_size; };

	void bind() const;
	void unbind() const;

	VertexBuffer& operator=(VertexBuffer&& other) noexcept;
};

#endif