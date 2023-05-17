#shader vertex
#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

layout(location = 0) out vec3 v_Color;

void main() {
	gl_Position = vec4(a_Position, 1.0);
	v_Color = a_Color;
}

#shader fragment
#version 450 core

layout(location = 0) in vec3 v_Color;

layout(location = 0) out vec4 o_Color;

void main() {
	o_Color = vec4(v_Color, 1.0);
}