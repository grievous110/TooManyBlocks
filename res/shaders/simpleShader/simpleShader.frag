#version 430 core

in vec2 uv;

layout(location = 0) out vec4 outColor;

uniform vec3 u_color;
uniform bool u_useTexture;
uniform sampler2D u_texture;

void main() {
	if (u_useTexture) {
		outColor = texture(u_texture, uv).rgba;
	} else {
		outColor = vec4(u_color, 1.0);
	}
}