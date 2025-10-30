#version 430 core

layout(location = 0) in uint compressedData1;

uniform mat4 u_viewProjection;
uniform vec3 u_chunkPosition;

#define POSITION_BITMASK  0x3Fu
#define X_POSITION_OFFSET 26
#define Y_POSITION_OFFSET 20
#define Z_POSITION_OFFSET 14

#define SET_BITS(target, value, bitmask, position) (target = (target & ~(bitmask << position)) | ((value & bitmask) << position))
#define GET_BITS(target, bitmask, position) ((target >> position) & bitmask)

vec3 decodePosition(uint compressedData) {
    uvec3 pos;
    pos.x = GET_BITS(compressedData, POSITION_BITMASK, X_POSITION_OFFSET);
    pos.y = GET_BITS(compressedData, POSITION_BITMASK, Y_POSITION_OFFSET);
    pos.z = GET_BITS(compressedData, POSITION_BITMASK, Z_POSITION_OFFSET);
    return vec3(pos);
}

void main() {
	vec3 localPosInChunk = decodePosition(compressedData1);
	vec3 worldVertexPos = u_chunkPosition + localPosInChunk;

	gl_Position = u_viewProjection * vec4(worldVertexPos, 1.0);
}