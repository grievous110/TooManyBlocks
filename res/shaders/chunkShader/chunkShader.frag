#version 330 core

#define MAX_LIGHTS 84

#define DIRECTIONALLIGHT 0u
#define SPOTLIGHT 1u
#define POINTLIGHT 1u

in vec3 position;
flat in uint texIndex;
in vec2 uv;
flat in vec3 normal;

struct Light {
    uint lightType;
    uint priority;
    uint shadowMapIndex;
    float padding1; 
    vec3 lightPosition;
    float padding2; 
    vec3 direction;
    float padding3; 
    vec3 color;
    float intensity;
    float range; // Used by point- / spotlight
    float fovy; // Used by spotlicht
    float innerCutoffAngle; // Used by spotlicht
    float padding4;
};

layout(std140) uniform LightsBlock {
    Light u_lights[MAX_LIGHTS];
};

layout(std140) uniform LightViewProjBlock {
    mat4 u_lightViewProjections[MAX_LIGHTS];
};

uniform sampler2D u_textureAtlas;
uniform vec3 u_cameraPosition;
uniform uint u_textureAtlasSize;
uniform uint u_textureSize;

uniform int u_lightCount;

uniform sampler2D u_shadowMapAtlas[3];
uniform uint u_shadowMapAtlasSizes[3];
uniform uint u_shadowMapSizes[3];

vec4 sampleFromTexAtlas(vec2 uv_coord) {
    float textureScale = float(u_textureSize) / float(u_textureAtlasSize);
    float texturesPerRow = float(u_textureAtlasSize) / float(u_textureSize);
    float row = floor(float(texIndex) / texturesPerRow);
    row = texturesPerRow - 1.0 - row; // Flip Y-axis (Image is loaded starting from top left / opengl reads uvs starting from bottom left tho)
    float col = mod(float(texIndex), texturesPerRow);

    vec2 scaledUV = uv_coord * textureScale;
    vec2 atlasUV = vec2(
        (col * textureScale) + scaledUV.x,
        (row * textureScale) + scaledUV.y
    );

    return texture(u_textureAtlas, atlasUV);
}

vec3 calcLightContribution(int lightIndex) {
    vec4 lightSpacePosition = u_lightViewProjections[lightIndex] * vec4(position, 1.0);
    
    uint lightType = u_lights[lightIndex].lightType;
    uint priority = u_lights[lightIndex].priority;
    uint shadowMapIndex = u_lights[lightIndex].shadowMapIndex;
    vec3 lightPosition = u_lights[lightIndex].lightPosition;
    vec3 direction = u_lights[lightIndex].direction;
    vec3 color = u_lights[lightIndex].color;
    float intensity = u_lights[lightIndex].intensity;
    float range = u_lights[lightIndex].range;
    float fovy = u_lights[lightIndex].fovy;
    float innerCutoffAngle = u_lights[lightIndex].innerCutoffAngle;

    vec3 lightSpaceCoord = lightSpacePosition.xyz / lightSpacePosition.w; // Perspective divide
    lightSpaceCoord = lightSpaceCoord * 0.5 + 0.5; // Map to [0, 1]

    // Check if the fragment is outside the light's projection bounds
    if (lightSpaceCoord.x < 0.0 || lightSpaceCoord.x > 1.0 ||
        lightSpaceCoord.y < 0.0 || lightSpaceCoord.y > 1.0 ||
        lightSpaceCoord.z < 0.0 || lightSpaceCoord.z > 1.0) {
        return vec3(0.0); // In shadow
    }

    // Calculate the shadow map atlas layout for this light
    float shadowMapSize = float(u_shadowMapSizes[priority]);

    // Compute the number of shadow maps per row in the atlas
    float shadowAtlasSize = float(u_shadowMapAtlasSizes[priority]);
    float tilesPerRow = shadowAtlasSize / shadowMapSize;

    // Determine the light's position in the shadow map atlas
    float col = mod(float(shadowMapIndex), tilesPerRow);
    float row = floor(float(shadowMapIndex) / tilesPerRow);

    // Offset the lightSpaceCoord to target the correct tile in the atlas
    vec2 atlasUV = lightSpaceCoord.xy / tilesPerRow + vec2(col, row) * (shadowMapSize / shadowAtlasSize);

    // Sample the shadow map atlas
    float shadowDepth = texture(u_shadowMapAtlas[priority], atlasUV).r;

    // Compare depths with bias to determine shadow
    float bias = 0.00005;
    float currentDepth = lightSpaceCoord.z;

    bool inShadow = currentDepth > shadowDepth + bias;

    if (!inShadow) {
        // Diffuse color
        vec3 lightDirection = normalize(lightPosition - position);
        float diffuseFactor = max(dot(normal, lightDirection), 0.0);

        // Specular color
        vec3 viewDirection = normalize(u_cameraPosition - position);
        vec3 reflectionDirection = reflect(-lightDirection, normal);
        float specularEpxponent = 30.0;
        float specularFactor = pow(max(dot(viewDirection, reflectionDirection), 0.0), specularEpxponent);

        float spotFactor = 1.0;
        if (lightType == SPOTLIGHT) {
            // Spot effect
            float angle = degrees(acos(dot(-direction, lightDirection)));
            spotFactor = 1.0 - smoothstep(innerCutoffAngle / 2.0, fovy / 2.0, angle);
        }

        // Distant fade
        float falloff = 1.0;
        if (lightType == SPOTLIGHT || lightType == POINTLIGHT) {
            float distanceToLight = length(lightPosition - position);
            float falloffStartRange = range * 0.9;
            falloff = 1.0 - smoothstep(falloffStartRange, range, distanceToLight);
        }

        return color * intensity * falloff * spotFactor * (diffuseFactor + specularFactor);
    } else {
        return vec3(0.0);
    }
}

void main() {
    vec2 uv_frag = fract(uv); // Effectively modulo for repeating texture on faces larger 1
    uv_frag.y = 1.0 - uv_frag.y; // Flip vertically    

    // Sample the texture atlas
    vec3 color = vec3(sampleFromTexAtlas(uv_frag));

    // Initialize shadow factor
    vec3 lightContrib = vec3(0.0);
    for (int i = 0; i < u_lightCount; i++) {
        // Accumulate light contribution
        lightContrib += calcLightContribution(i);
    }

    color = (0.15 * color) + (0.85 * clamp(lightContrib * color, 0.0, 1.0));

    // Gradual fade to black for distant elements
    float dist = length(u_cameraPosition - position);
    float dropoffStartDistance = 50.0;
    float fadeDistance = 32.0;
    if (dist > dropoffStartDistance) {
        float fade = smoothstep(dropoffStartDistance, dropoffStartDistance + fadeDistance, dist);
        color *= 1.0 - fade;
    }

   gl_FragColor = vec4(color, 1.0);
}