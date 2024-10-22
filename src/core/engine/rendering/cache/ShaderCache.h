#ifndef SHADERCACHE_H
#define SHADERCACHE_H

#include "engine/rendering/lowlevelapi/Shader.h"
#include <memory>
#include <string>
#include <unordered_map>

class ShaderCache {
private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaderMap;

public:
    std::shared_ptr<Shader> getShader(const std::string& name);
};

#endif

