#version 330 core

in vec3 position;
in vec2 uv;
in vec3 normal;

layout(location = 0) out vec4 frag_color;

uniform vec3 u_lightPos;
uniform mat4 u_model;
uniform sampler2D u_texture;

void main() {
	vec4 texColor = texture(u_texture, uv);

	vec3 vWorldPosition = vec3(u_model * vec4(position, 1.0));
	vec3 vWorldNormal = normalize(vec3(u_model * vec4(normal, 1.0)));

	vec3 lightDirection = normalize(u_lightPos - vWorldPosition);

    float diff = max(dot(vWorldNormal, lightDirection), 0.0);

	vec3 diffuse = 0.9 * diff * texColor.rgb;
	vec3 ambient = 0.1 * texColor.rgb;

	frag_color = vec4(diffuse + ambient, texColor.a);
}