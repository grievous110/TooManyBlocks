#ifndef TOOMANYBLOCKS_SHADERSTORAGEBUFFER_H
#define TOOMANYBLOCKS_SHADERSTORAGEBUFFER_H

#include <stddef.h>  // For size_t

#include "engine/rendering/lowlevelapi/RenderApiObject.h"

/**
 * @brief Represents an OpenGL Shader Storage Buffer Object (SSBO).
 *
 * Encapsulates allocation, binding, and data updates for shader storage buffers.
 * Binds buffers via `glBindBufferBase` to a specific binding point.
 */
class ShaderStorageBuffer : public RenderApiObject {
private:
    static thread_local unsigned int currentlyBoundSSBO;
    size_t m_size;

    ShaderStorageBuffer(const void* data, size_t size);

public:
    /**
     * @brief Unbinds any currently bound shader storage buffer.
     */
    static void bindDefault();
    /**
     * @brief Syncs the internal binding state with the current OpenGL binding.
     *
     * Should be used if SSBO binding is changed manually.
     */
    static void syncBinding();

    /**
     * @brief Creates a new uniform buffer and uploads initial data.
     *
     * If `data` is `nullptr`, memory is allocated but not initialized.
     *
     * @param data Pointer to the initial buffer contents, or nullptr.
     * @param size Size of the buffer in bytes.
     */
    static ShaderStorageBuffer create(const void* data, size_t size);

    /**
     * @brief Constructs an uninitialized shader storage buffer with id 0.
     */
    ShaderStorageBuffer() noexcept : m_size(0) {}
    ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept;
    virtual ~ShaderStorageBuffer();

    /**
     * @brief Updates a portion of the shader storage buffer with new data.
     *
     * @param data Pointer to the source data.
     * @param size Size of the data in bytes.
     * @param offset Byte offset in the buffer where data should be written.
     *
     * @throws std::runtime_error if the offset + size exceeds the buffer’s size.
     * @throws std::runtime_error If the buffer ID is 0 (uninitialized or moved-from).
     */
    void updateData(const void* data, size_t size, size_t offset = 0) const;

    /**
     * @brief Binds the shade storage buffer to GL_SHADER_STORAGE_BUFFER if not already bound.
     *
     * @throws std::runtime_error If the buffer ID is 0 (uninitialized or moved-from).
     */
    void bind() const;

    /**
     * @brief Binds the buffer to a specific global ssbo binding point.
     *
     * @param bindingPoint Index in the block binding range.
     */
    void assignTo(unsigned int bindingPoint) const;

    /** @return The size of the buffer in bytes. */
    inline size_t getByteSize() const { return m_size; };

    ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept;
};

#endif