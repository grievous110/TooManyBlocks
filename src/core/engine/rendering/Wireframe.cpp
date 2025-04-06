#include "engine/rendering/GLUtils.h"
#include "Wireframe.h"
#include <GL/glew.h>
#include <vector>

void Wireframe::draw() const {
    m_data->vao.bind();
	m_data->ibo.bind();

	GLCALL(glDrawElements(GL_TRIANGLES, m_data->ibo.count(), GL_UNSIGNED_INT, nullptr));
}

Wireframe Wireframe::fromBoundigBox(const BoundingBox& bbox) {
    glm::vec3 corners[8] = {
        {bbox.min.x, bbox.min.y, bbox.min.z}, // 0
        {bbox.max.x, bbox.min.y, bbox.min.z}, // 1
        {bbox.max.x, bbox.max.y, bbox.min.z}, // 2
        {bbox.min.x, bbox.max.y, bbox.min.z}, // 3
        {bbox.min.x, bbox.min.y, bbox.max.z}, // 4
        {bbox.max.x, bbox.min.y, bbox.max.z}, // 5
        {bbox.max.x, bbox.max.y, bbox.max.z}, // 6
        {bbox.min.x, bbox.max.y, bbox.max.z}, // 7
    };

    const unsigned int indices[24] = {
        0, 1, 1, 2, 2, 3, 3, 0, // bottom
        4, 5, 5, 6, 6, 7, 7, 4, // top
        0, 4, 1, 5, 2, 6, 3, 7  // verticals
    };

    // VBO
    VertexBuffer vbo(&corners[0], 8 * sizeof(glm::vec3));
    // Layout
    VertexBufferLayout layout;
    layout.push(GL_FLOAT, sizeof(float), 3);
    // VAO
    VertexArray vao;
    vao.addBuffer(vbo, layout);
    // IBO
    IndexBuffer ibo(indices, 24);

    std::unique_ptr<WireframeRenderData> renderData = std::make_unique<WireframeRenderData>(std::move(vao), std::move(vbo), std::move(ibo));
    
    return Wireframe(std::move(renderData), bbox);
}
