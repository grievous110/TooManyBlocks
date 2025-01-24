#ifndef PROVIDER_H
#define PROVIDER_H

#include "engine/rendering/mat/Material.h"
#include <memory>
#include <string>
#include <unordered_map>

class Provider {
private:
    std::unordered_map<std::string, std::weak_ptr<Material>> m_materialCache;

public:
    std::shared_ptr<Material> getChachedMaterial(const std::string& name) const;

    void putMaterial(const std::string& name, std::shared_ptr<Material> material);
};

#endif