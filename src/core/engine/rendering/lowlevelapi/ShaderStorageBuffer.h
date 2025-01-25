#ifndef SHADERSTORAGEBUFFER_H
#define SHADERSTORAGEBUFFER_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"

class ShaderStorageBuffer : public RenderApiObject {
private:
	size_t m_size;
	
public:
	static void bindDefault();

	ShaderStorageBuffer(const void* data, size_t size); // size is in bytes
	ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept;
	virtual ~ShaderStorageBuffer();

	inline size_t getByteSize() const { return m_size; };

	void bind(unsigned int bindingPoint) const;

	ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept;
};

#endif