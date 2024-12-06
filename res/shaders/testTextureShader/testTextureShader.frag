#version 330 core

in vec2 uv;

layout(location = 0) out vec4 color;

uniform sampler2D u_texture;

void main() {
	vec4 texColor = texture(u_texture, uv);
	color = texColor;
}