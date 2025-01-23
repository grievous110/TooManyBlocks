#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"

class IndexBuffer : public RenderApiObject {
private:
	static thread_local unsigned int currentlyBoundIBO;
	unsigned int m_count;
	
public:
	static void bindDefault();
	static void syncBinding();

	IndexBuffer(const unsigned int* data, unsigned int count);
	IndexBuffer(IndexBuffer&& other) noexcept;
	virtual ~IndexBuffer();

	void bind() const;

	inline unsigned int count() const { return m_count; }

	IndexBuffer& operator=(IndexBuffer&& other) noexcept;
};

#endif