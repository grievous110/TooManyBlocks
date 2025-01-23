#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"

class VertexBuffer : public RenderApiObject {
private:
	static thread_local unsigned int currentlyBoundVBO;
	int m_size;
	
public:
	static void bindDefault();
	static void syncBinding();

	VertexBuffer(const void* data, int size); // size is in bytes
	VertexBuffer(VertexBuffer&& other) noexcept;
	virtual ~VertexBuffer();

	inline int getByteSize() const { return m_size; };

	void bind() const;

	VertexBuffer& operator=(VertexBuffer&& other) noexcept;
};

#endif