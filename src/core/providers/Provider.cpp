#include "Provider.h"

std::shared_ptr<Material> Provider::getChachedMaterial(const std::string &name) const {
    auto it = m_materialCache.find(name);
    if (it != m_materialCache.end()) {
        if (std::shared_ptr<Material> ptr = it->second.lock()) {
            return ptr;
        }
    }

    return nullptr;
}

void Provider::putMaterial(const std::string &name, std::shared_ptr<Material> material) {
    m_materialCache[name] = material;
}
