#version 330 core

layout(location = 0) in uint compressedPosition;

uniform mat4 u_lightViewProjection;
uniform vec3 u_chunkPosition;

#define POSITION_BITMASK 0x3FFu
#define X_POSITION_OFFSET 20
#define Y_POSITION_OFFSET 10
#define Z_POSITION_OFFSET 0

#define SET_BITS(target, value, bitmask, position) (target = (target & ~(bitmask << position)) | ((value & bitmask) << position))
#define GET_BITS(target, bitmask, position) ((target >> position) & bitmask)

vec3 decodePosition(uint compressedPosition) {
    uvec3 pos;
    pos.x = GET_BITS(compressedPosition, POSITION_BITMASK, X_POSITION_OFFSET);
    pos.y = GET_BITS(compressedPosition, POSITION_BITMASK, Y_POSITION_OFFSET);
    pos.z = GET_BITS(compressedPosition, POSITION_BITMASK, Z_POSITION_OFFSET);
    return vec3(pos);
}

void main() {
	vec3 localPosInChunk = decodePosition(compressedPosition);
	vec3 worldVertexPos = u_chunkPosition + localPosInChunk;

	gl_Position = u_lightViewProjection * vec4(worldVertexPos, 1.0);
}