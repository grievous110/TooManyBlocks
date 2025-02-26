#ifndef BLOCKTOTEXTUREMAPPING_H
#define BLOCKTOTEXTUREMAPPING_H

#include "datatypes/DatatypeDefs.h"
#include "datatypes/BlockTypes.h"
#include <cstdint>
#include <array>
#include <unordered_map>

class BlockToTextureMap {
private:
    std::unordered_map<uint16_t, std::array<uint16_t, 6>> map;
    
public:
    BlockToTextureMap();

    uint16_t getTexIndex(uint16_t blockType, AxisDirection dir) const;
};

#endif