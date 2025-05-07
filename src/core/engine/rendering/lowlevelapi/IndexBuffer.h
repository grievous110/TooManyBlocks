#ifndef TOOMANYBLOCKS_INDEXBUFFER_H
#define TOOMANYBLOCKS_INDEXBUFFER_H

#include <stddef.h>  // For size_t

#include "engine/rendering/lowlevelapi/RenderApiObject.h"

/**
 * @brief Wrapper for OpenGL Index Buffer Object (IBO).
 *
 * Stores and manages element indices for indexed drawing.
 */
class IndexBuffer : public RenderApiObject {
private:
    static thread_local unsigned int currentlyBoundIBO;
    size_t m_count;

    IndexBuffer(const unsigned int* data, size_t count);

public:
    /**
     * @brief Unbinds any currently bound index buffer on the current thread.
     */
    static void bindDefault();
    /**
     * @brief Synchronizes the wrapper's internal binding state with OpenGL.
     *
     * Should be used if the IBO binding is changed manually.
     */
    static void syncBinding();
    /**
     * @brief Creates a new index buffer and uploads index data.
     *
     *  If `data` is `nullptr`, memory is allocated but not initialized.
     *
     * @param data Pointer to an array of unsigned int indices, or nullptr to allocate uninitialized storage.
     * @param count Number of indices to allocate space for.
     */
    static IndexBuffer create(const unsigned int* data, size_t count);

    /**
     * @brief Constructs an uninitialized index buffer with id 0.
     */
    IndexBuffer() noexcept : m_count(0) {}
    IndexBuffer(IndexBuffer&& other) noexcept;
    virtual ~IndexBuffer();

    /**
     * @brief Updates a portion of the index buffer with new data.
     *
     * @param data Pointer to the index data.
     * @param count Amount of indices to write.
     * @param offset element offset in the buffer where data should be written.
     *
     * @throws std::runtime_error If the update exceeds the allocated buffer length.
     * @throws std::runtime_error If the buffer ID is 0 (uninitialized or moved-from).
     */
    void updateData(const unsigned int* data, size_t count, size_t offset = 0) const;

    /**
     * @brief Binds the index buffer to GL_ELEMENT_ARRAY_BUFFER if not already bound.
     *
     * @throws std::runtime_error If the buffer ID is 0 (uninitialized or moved-from).
     */
    void bind() const;

    /** @return The number of indices stored in the buffer. */
    inline size_t count() const { return m_count; }

    IndexBuffer& operator=(IndexBuffer&& other) noexcept;
};

#endif