#vertex shader
#version 330 core

layout(location = 0) in vec3 v_position;

uniform mat4 u_lightSpaceMatrix;

void main() {
	gl_Position  = u_lightSpaceMatrix * vec4(v_position, 1.0);
}

#fragment shader
#version 330 core

void main() {
	// Nothing
}