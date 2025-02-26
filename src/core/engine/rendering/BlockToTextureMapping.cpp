#include "BlockToTextureMapping.h"
#include <stdexcept>

#define ADD_MAPPING(blockType, mappingdef) map.emplace(blockType, std::array<uint16_t, 6>{mappingdef})

#define ALL_FACES(val) val, val, val, val, val, val
#define AXIS_BASED(x_axis, y_axis, z_axis) x_axis, x_axis, y_axis, y_axis, z_axis, z_axis
#define INDIVIDUAL_FACES(xp_face, xn_face, yp_face, yn_face, zp_face, zn_face) xp_face, xn_face, yp_face, yn_face, zp_face, zn_face
#define TOP_BOTTOM_SIDES(top, bottom, sides) sides, sides, top, bottom, sides, sides
#define TOP_BOTTOM_WITH_SIDES(top, bottom, xp_face, xn_face, zp_face, zn_face) xp_face, xn_face, top, bottom, zp_face, zn_face

BlockToTextureMap::BlockToTextureMap() {
    ADD_MAPPING(STONE, ALL_FACES(0U));
    ADD_MAPPING(GRASS, TOP_BOTTOM_SIDES(4U, 2U, 3U));
    ADD_MAPPING(DIRT, ALL_FACES(2U));
}

uint16_t BlockToTextureMap::getTexIndex(uint16_t blockType, AxisDirection dir) const {
    auto it = map.find(blockType);
    if (it == map.end()) {
        throw std::out_of_range("No texture index mapping for this block type");
    }
    return it->second[static_cast<size_t>(dir)];
}
