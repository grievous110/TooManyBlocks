#ifndef TOOMANYBLOCKS_VERTEXBUFFER_H
#define TOOMANYBLOCKS_VERTEXBUFFER_H

#include <stddef.h>

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"

class VertexBuffer : public RenderApiObject {
private:
    static thread_local unsigned int currentlyBoundVBO;
    VertexBufferLayout m_layout;
    size_t m_size;

public:
    static void bindDefault();
    static void syncBinding();

    VertexBuffer(const void* data, size_t size);  // size is in bytes
    VertexBuffer(VertexBuffer&& other) noexcept;
    virtual ~VertexBuffer();

    void updateData(const void* data, size_t size, size_t offset = 0) const;

    void bind() const;

    inline void setLayout(const VertexBufferLayout& layout) { m_layout = layout; }

    inline const VertexBufferLayout& getLayout() const { return m_layout; }

    inline size_t getByteSize() const { return m_size; };

    inline size_t getVertexCount() const { return m_layout.stride() ? (m_size / m_layout.stride()) : 0; }

    VertexBuffer& operator=(VertexBuffer&& other) noexcept;
};

#endif