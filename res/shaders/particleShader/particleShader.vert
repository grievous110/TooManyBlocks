#version 430

layout(location = 0) in vec2 v_position;
layout(location = 1) in vec2 v_uv;

layout(location = 2) in vec4 in_color;
layout(location = 3) in vec3 in_velocity;
layout(location = 4) in vec3 in_position;
layout(location = 5) in float in_timeToLive;
layout(location = 6) in float in_initialTimeToLive;
layout(location = 7) in float in_size;
layout(location = 8) in uint in_flags;

out vec2 uv;
flat out vec4 color;

uniform mat4 u_mvp;
uniform vec3 u_cameraRight;
uniform vec3 u_cameraUp;

void main() {
    vec3 offset = (u_cameraRight * v_position.x + u_cameraUp * v_position.y) * in_size;
    vec3 position = in_position + offset;

    uv = v_uv;
    color = in_color;

    gl_Position = u_mvp * vec4(position, 1.0);
}