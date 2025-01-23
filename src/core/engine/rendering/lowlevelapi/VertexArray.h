#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"

class VertexArray : public RenderApiObject {
private:
	static thread_local unsigned int currentlyBoundVAO;

public:
	static void bindDefault();
	static void syncBinding();

	VertexArray();
	VertexArray(VertexArray&& other) noexcept;
	virtual ~VertexArray();

	void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	void bind() const;

    VertexArray& operator=(VertexArray&& other) noexcept;
};

#endif