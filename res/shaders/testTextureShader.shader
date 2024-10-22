#vertex shader
#version 330 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_uv;

out vec2 uv;

uniform mat4 u_mvp;

void main() {
	gl_Position = u_mvp * vec4(v_position, 1.0f);
	uv = v_uv;
}

#fragment shader
#version 330 core

in vec2 uv;

layout(location = 0) out vec4 color;

uniform sampler2D u_texture;

void main() {
	vec4 texColor = texture(u_texture, uv);
	color = texColor;
}