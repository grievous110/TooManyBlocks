#version 330 core

layout(location = 0) in vec3 v_position;

uniform mat4 u_lightViewProjection;

void main() {
	gl_Position = u_lightViewProjection * vec4(v_position, 1.0);
}