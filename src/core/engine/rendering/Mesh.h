#ifndef MESH_H
#define MESH_H

#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/Renderable.h"
#include "engine/rendering/RenderData.h"
#include <memory>

class Mesh : public Renderable {
private:
	std::shared_ptr<RenderData> m_data;
	std::shared_ptr<Material> m_material;
	BoundingBox m_bounds;
	
	void draw() const override;

public:
	Mesh(std::shared_ptr<RenderData> data, const BoundingBox& bounds, std::shared_ptr<Material> material = nullptr) : m_data(data), m_material(material), m_bounds(bounds) {}
	virtual ~Mesh() = default;

	inline void assignMaterial(std::shared_ptr<Material> material) { m_material = material; }

	std::shared_ptr<Material> getMaterial() const override { return m_material; }

	BoundingBox getBoundingBox() const override { return m_bounds; }
};

#endif