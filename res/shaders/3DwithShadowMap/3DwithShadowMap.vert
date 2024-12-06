#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec3 v_normal;

out vec3 position;
out vec2 uv;
out vec3 normal;
out vec4 lightSpacePosition;

uniform mat4 u_mvp;
uniform mat4 u_model;
uniform mat4 u_lightSpaceMatrix;

void main() {
	vec4 worldPosition = u_model * vec4(v_position, 1.0);
	lightSpacePosition = u_lightSpaceMatrix * worldPosition;

	gl_Position = u_mvp * vec4(v_position, 1.0);
	position = worldPosition.xyz;
	uv = v_uv;
	normal = mat3(u_model) * v_normal; // Transform normal to world space
}