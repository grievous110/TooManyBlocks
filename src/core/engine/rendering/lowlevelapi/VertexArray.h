#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "engine/rendering/lowlevelapi/RenderApiObject.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"

class VertexArray : public RenderApiObject {
private:
    static thread_local unsigned int currentlyBoundVAO;
    unsigned int m_currAttribIndex;

public:
    static void bindDefault();
    static void syncBinding();

    VertexArray();
    VertexArray(VertexArray&& other) noexcept;
    virtual ~VertexArray();

    void addBuffer(const VertexBuffer& vb);

    void bind() const;

    void resetAttribIndex();

    VertexArray& operator=(VertexArray&& other) noexcept;
};

#endif