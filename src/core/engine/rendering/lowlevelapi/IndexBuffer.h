#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "RenderApiObject.h"

class IndexBuffer : virtual public RenderApiObject {
private:
	static unsigned int currentlyBoundIBO;
	unsigned int m_count;
	
public:
	IndexBuffer(const unsigned int* data, unsigned int count);
	IndexBuffer(IndexBuffer&& other) noexcept;
	virtual ~IndexBuffer();

	void bind() const;
	void unbind() const;

	inline unsigned int count() const { return m_count; }

	IndexBuffer& operator=(IndexBuffer&& other) noexcept;
};

#endif