#version 430

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_velocity;
layout(location = 2) in float in_timeToLive;
layout(location = 3) in uint in_type;

uniform mat4 u_mvp;

void main() {
    gl_Position = u_mvp * vec4(in_position, 1.0);
}