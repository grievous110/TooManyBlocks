#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include <stddef.h>

class IndexBuffer : public RenderApiObject {
private:
	static thread_local unsigned int currentlyBoundIBO;
	size_t m_count;
	
public:
	static void bindDefault();
	static void syncBinding();

	IndexBuffer(const unsigned int* data, size_t count);
	IndexBuffer(IndexBuffer&& other) noexcept;
	virtual ~IndexBuffer();

	void bind() const;

	inline size_t count() const { return m_count; }

	IndexBuffer& operator=(IndexBuffer&& other) noexcept;
};

#endif