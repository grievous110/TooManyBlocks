#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include <stddef.h>

class UniformBuffer : public RenderApiObject {
private:
	size_t m_size;
	
public:
	static void bindDefault();

	UniformBuffer(const void* data, size_t size); // size is in bytes
	UniformBuffer(UniformBuffer&& other) noexcept;
	virtual ~UniformBuffer();

	void updateData(const void* data, size_t size, size_t offset = 0) const;

	void bind(unsigned int bindingPoint) const;

	inline size_t getByteSize() const { return m_size; };

	UniformBuffer& operator=(UniformBuffer&& other) noexcept;
};

#endif