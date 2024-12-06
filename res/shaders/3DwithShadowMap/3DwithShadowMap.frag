#version 330 core

in vec3 position;
in vec2 uv;
in vec3 normal;
in vec4 lightSpacePosition;

layout(location = 0) out vec4 frag_color;

uniform vec3 u_lightPos;
uniform mat4 u_model;
uniform float bias;
uniform sampler2D u_texture;
uniform sampler2D u_shadowMap;

void main() {
	vec4 texColor = texture(u_texture, uv);
	vec3 vWorldPosition = position;
	vec3 vWorldNormal = normalize(normal);

	vec3 lightDirection = normalize(u_lightPos - vWorldPosition);
    float diff = max(dot(vWorldNormal, lightDirection), 0.0);

	vec3 diffuse = 0.9 * diff * texColor.rgb;
	vec3 ambient = 0.1 * texColor.rgb;

	// ----- Shadow Mapping -----
    // Calculate the shadow coordinates (divide by w to perform perspective division)
    vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;

	// Transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;

    // Sample the closest depth from the shadow map at this fragment's position
    float closestDepth = texture(u_shadowMap, projCoords.xy).r;
    
    // Current fragment's depth in light space
    float currentDepth = projCoords.z;

    // Check if the fragment is in shadow
    float shadow = currentDepth > closestDepth + bias ? 0.0 : 1.0;

    // Combine lighting with shadow factor
    vec3 lighting = diffuse * shadow + ambient;

	frag_color = vec4(lighting, 1.0);
}