#version 430 core

layout (location = 0) in vec2 v_position;  // 2D-Position des Fullscreen-Quads
layout (location = 1) in vec2 v_uv; // UV-Koordinaten

out vec2 screenUV; // Weitergabe an Fragment-Shader

void main() {
    screenUV = v_uv;  
    gl_Position = vec4(v_position, 0.0, 1.0);  // 2D in Clip-Space
}