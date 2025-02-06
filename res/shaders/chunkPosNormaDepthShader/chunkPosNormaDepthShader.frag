#version 330 core

in vec3 position;
flat in uint texIndex;
in vec2 uv;
flat in vec3 normal;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;

void main() {
    gPosition = position;
    gNormal = normal;
}