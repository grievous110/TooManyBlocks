#ifndef TOOMANYBLOCKS_VERTEXBUFFER_H
#define TOOMANYBLOCKS_VERTEXBUFFER_H

#include <stddef.h>  // For size_t

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include "engine/rendering/lowlevelapi/VertexBufferLayout.h"

/**
 * @brief Represents an OpenGL Vertex Buffer Object (VBO).
 *
 * Manages GPU memory for vertex data. Supports data uploads and layout specification.
 */
class VertexBuffer : public RenderApiObject {
private:
    static thread_local unsigned int currentlyBoundVBO;
    VertexBufferLayout m_layout;
    size_t m_size;

    VertexBuffer(const void* data, size_t size);

public:
    /**
     * @brief Unbinds any currently bound vertex buffer.
     */
    static void bindDefault();
    /**
     * @brief Syncs the internal binding state with the current OpenGL binding.
     *
     * Should be used if VBO binding is changed manually.
     */
    static void syncBinding();

    /**
     * @brief Creates a vertex buffer with optional initial data.
     *
     * If `data` is `nullptr`, memory is allocated but not initialized.
     *
     * @param data Pointer to the vertex data, or nullptr to allocate only.
     * @param size Size of the buffer in bytes.
     */
    static VertexBuffer create(const void* data, size_t size);

    /**
     * @brief Constructs an uninitialized vertex buffer with id 0.
     */
    VertexBuffer() noexcept : m_size(0) {}
    VertexBuffer(VertexBuffer&& other) noexcept;
    virtual ~VertexBuffer();

    /**
     * @brief Updates a portion of the vertex buffer with new data.
     *
     * @param data Pointer to the source data.
     * @param size Size of the data in bytes.
     * @param offset Byte offset in the buffer where data should be written.
     *
     * @throws std::runtime_error If the update exceeds the allocated buffer size.
     * @throws std::runtime_error If the buffer ID is 0 (uninitialized or moved-from).
     */
    void updateData(const void* data, size_t size, size_t offset = 0) const;

    /**
     * @brief Binds the vertex buffer to GL_ELEMENT_ARRAY_BUFFER if not already bound.
     *
     * @throws std::runtime_error If the buffer ID is 0 (uninitialized or moved-from).
     */
    void bind() const;

    /**
     * @brief Sets the layout describing the structure of the vertex data.
     */
    inline void setLayout(const VertexBufferLayout& layout) { m_layout = layout; }

    /**
     * @brief Returns the currently assigned vertex buffer layout.
     */
    inline const VertexBufferLayout& getLayout() const { return m_layout; }

    /** @return The total size of the buffer in bytes. */
    inline size_t getByteSize() const { return m_size; };

    /**
     * @return The number of vertices stored in the buffer. This is derived from the set layout
     * and is 0 if no layout is set.
     */
    inline size_t getVertexCount() const { return m_layout.stride() ? (m_size / m_layout.stride()) : 0; }

    VertexBuffer& operator=(VertexBuffer&& other) noexcept;
};

#endif