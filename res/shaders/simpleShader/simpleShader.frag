#version 430 core

in vec2 uv;

layout(location = 0) out vec4 outColor;

uniform vec3 u_color;
uniform bool u_useTexture;
uniform sampler2D u_texture;

void main() {
	vec4 color = u_useTexture ? texture(u_texture, uv) : vec4(u_color, 1.0);
	if (color.a < 0.99)
		discard;
	outColor = color;
}