#shader vertex
#version 450 core
#extension GL_KHR_vulkan_glsl : enable

vec2 positions[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 v_Color;

void main() {
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	v_Color = colors[gl_VertexIndex];
}

#shader fragment
#version 450 core

layout(location = 0) in vec3 v_Color;

layout(location = 0) out vec4 o_Color;

void main() {
	o_Color = vec4(v_Color, 1.0);
}