#ifndef TOOMANYBLOCKS_RENDERABLE_H
#define TOOMANYBLOCKS_RENDERABLE_H

#include <memory>

#include "engine/comp/SceneComponent.h"
#include "engine/geometry/BoundingVolume.h"
#include "engine/rendering/mat/Material.h"

class Renderable : public SceneComponent {
    friend class Renderer;

private:
    virtual void draw() const = 0;

protected:
    std::shared_ptr<Material> m_material;

public:
    Renderable(std::shared_ptr<Material> material = nullptr) : m_material(material) {}
    virtual ~Renderable() = default;

    virtual void assignMaterial(std::shared_ptr<Material> material) { m_material = material; }

    virtual std::shared_ptr<Material> getMaterial() const { return m_material; };

    virtual BoundingBox getBoundingBox() const { return BoundingBox::invalid(); };

};

#endif