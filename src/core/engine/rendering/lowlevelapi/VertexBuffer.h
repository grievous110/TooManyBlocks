#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"

class VertexBuffer : public RenderApiObject {
private:
	static thread_local unsigned int currentlyBoundVBO;
	size_t m_size;
	
public:
	static void bindDefault();
	static void syncBinding();

	VertexBuffer(const void* data, size_t size); // size is in bytes
	VertexBuffer(VertexBuffer&& other) noexcept;
	virtual ~VertexBuffer();

	inline size_t getByteSize() const { return m_size; };

	void bind() const;

	VertexBuffer& operator=(VertexBuffer&& other) noexcept;
};

#endif