#version 430

layout(location = 0) in vec4 in_color;
layout(location = 1) in vec3 in_velocity;
layout(location = 2) in vec3 in_position;
layout(location = 3) in float in_timeToLive;
layout(location = 4) in float in_initialTimeToLive;
layout(location = 5) in float in_size;
layout(location = 6) in uint in_flags;

flat out vec4 color;

uniform mat4 u_mvp;

void main() {
    color = in_color;

    gl_Position = u_mvp * vec4(in_position, 1.0);
}