#ifndef LINE_H
#define LINE_H

#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/lowlevelapi/VertexArray.h"
#include "engine/rendering/lowlevelapi/VertexBuffer.h"
#include "engine/rendering/Renderable.h"
#include <memory>

class Line : public Renderable {
private:
    std::unique_ptr<VertexArray> m_vao;
    std::unique_ptr<VertexBuffer> m_vbo;
    std::shared_ptr<Material> m_material;
    BoundingBox m_bounds;
    float m_lineWidth;

    void draw() const override;

public:
    Line(const glm::vec3& start, const glm::vec3& end, float lineWidth = 2.0f, std::shared_ptr<Material> material = nullptr);
    virtual ~Line() = default;

    inline void setLineWidth(float lineWidth) { m_lineWidth = lineWidth; }

    inline float getLineWidht() const { return m_lineWidth; }

    inline void assignMaterial(std::shared_ptr<Material> material) { m_material = material; }

    std::shared_ptr<Material> getMaterial() const override { return m_material; }

    BoundingBox getBoundingBox() const override { return m_bounds; }
};

#endif