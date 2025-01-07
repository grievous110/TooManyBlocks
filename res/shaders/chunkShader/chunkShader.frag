#version 330 core

in vec3 position;
flat in uint texIndex;
in vec2 uv;
flat in vec3 normal;

uniform sampler2D u_textureAtlas;
uniform uint u_textureAtlasSize;
uniform uint u_textureSize;

layout(location = 0) out vec4 frag_color;

void main() {
    vec2 uv_frag = fract(uv); // Effectively modulo for repeating texture on faces larger 1
    uv_frag.y = 1.0 - uv_frag.y; // Flip vertically

    float textureScale = float(u_textureSize) / float(u_textureAtlasSize);

    float texturesPerRow = float(u_textureAtlasSize) / float(u_textureSize);
    float row = floor(float(texIndex) / texturesPerRow);
    row = texturesPerRow - 1.0 - row; // Flip Y-axis (Image is loaded starting from top left / opengl reads uvs starting from bottom left tho)
    float col = mod(float(texIndex), texturesPerRow);

    vec2 scaledUV = uv_frag * textureScale;
    vec2 atlasUV = vec2(
        (col * textureScale) + scaledUV.x,
        (row * textureScale) + scaledUV.y
    );

    // Sample the texture atlas
    vec4 color = texture(u_textureAtlas, atlasUV);
    frag_color = vec4(color.rgb, 1.0);
}