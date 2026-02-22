#version 430 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_uv;

out vec2 uv;

uniform mat4 u_mvp;

void main() {
	gl_Position = u_mvp * vec4(v_position, 1.0);
	uv = v_uv;
}