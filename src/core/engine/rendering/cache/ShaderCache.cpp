#include "engine/rendering/cache/ShaderCache.h"

std::shared_ptr<Shader> ShaderCache::getShader(const std::string& name) {
    if (m_shaderMap.find(name) != m_shaderMap.end()) {
        return m_shaderMap[name];
    }

    m_shaderMap[name] = std::make_shared<Shader>(name);
    return m_shaderMap[name];
}