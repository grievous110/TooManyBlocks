#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include <stddef.h>

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

	void updateData(const void* data, size_t size, size_t offset = 0) const;

	void bind() const;

	inline size_t getByteSize() const { return m_size; };

	VertexBuffer& operator=(VertexBuffer&& other) noexcept;
};

#endif