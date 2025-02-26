#version 430 core

in vec2 uv;

uniform vec3 u_color;
uniform int u_useTexture;
uniform sampler2D u_texture;

void main() {
	if (u_useTexture != 0) {
		gl_FragColor = texture(u_texture, uv).rgba;
	} else {
		gl_FragColor = vec4(u_color, 1.0);
	}
}