#version 330 core

in vec2 uv;

layout(location = 0) out vec4 frag_color;

uniform vec3 u_color;
uniform int u_useTexture;
uniform sampler2D u_texture;

void main() {
	if (u_useTexture != 0) {
		vec4 texColor = texture(u_texture, uv);
		frag_color = texColor;
	} else {
		frag_color = vec4(u_color, 1.0);
	}
}