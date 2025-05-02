#include "StaticMeshBlueprint.h"

#include <GL/glew.h>

static std::shared_ptr<RenderData> packToRenderData(const CPURenderData<CompactChunkVertex>& data) {
    // Vertex Buffer Object (VBO)
    VertexBuffer vbo(data.vertices.data(), data.vertices.size() * sizeof(CompactChunkVertex));

    // Vertex Attribute Pointer
    VertexBufferLayout layout;
    // Compressed data
    layout.push(GL_UNSIGNED_INT, 1);
    layout.push(GL_UNSIGNED_INT, 1);
    vbo.setLayout(layout);

    // Vertex Array Object (VAO)
    VertexArray vao;
    vao.addBuffer(vbo);

    if (data.isIndexed()) {
        // Index Buffer Object (IBO)
        IndexBuffer ibo(data.indices.data(), data.indices.size());
        return std::make_shared<IndexedRenderData>(std::move(vao), std::move(vbo), std::move(ibo));
    } else {
        return std::make_shared<NonIndexedRenderData>(std::move(vao), std::move(vbo));
    }
}

static std::shared_ptr<RenderData> packToRenderData(const CPURenderData<Vertex>& data) {
    // Vertex Buffer Object (VBO)
    VertexBuffer vbo(data.vertices.data(), data.vertices.size() * sizeof(Vertex));

    // Vertex Attribute Pointer
    VertexBufferLayout layout;
    layout.push(GL_FLOAT, 3);  // Position
    layout.push(GL_FLOAT, 2);  // UV
    layout.push(GL_FLOAT, 3);  // Normal
    vbo.setLayout(layout);

    // Vertex Array Object (VAO)
    VertexArray vao;
    vao.addBuffer(vbo);

    if (data.isIndexed()) {
        // Index Buffer Object (IBO)
        IndexBuffer ibo(data.indices.data(), data.indices.size());
        return std::make_shared<IndexedRenderData>(std::move(vao), std::move(vbo), std::move(ibo));
    } else {
        return std::make_shared<NonIndexedRenderData>(std::move(vao), std::move(vbo));
    }
}

void StaticMeshBlueprint::bake() {
    if (m_raw) {
        m_baked = std::make_unique<Baked>(Baked{packToRenderData(*m_raw), m_raw->bounds});
        m_raw.reset();
    }
}

std::shared_ptr<void> StaticMeshBlueprint::createInstance() const {
    if (m_baked) {
        return std::make_shared<StaticMesh::Internal>(StaticMesh::Internal{m_baked->renderData, m_baked->bounds});
    }
    return nullptr;
}

void StaticChunkMeshBlueprint::bake() {
    if (m_raw) {
        m_baked = std::make_unique<Baked>(Baked{packToRenderData(*m_raw), m_raw->bounds});
        m_raw.reset();
    }
}

std::shared_ptr<void> StaticChunkMeshBlueprint::createInstance() const {
    if (m_baked) {
        return std::make_shared<StaticMesh::Internal>(StaticMesh::Internal{m_baked->renderData, m_baked->bounds});
    }
    return nullptr;
}