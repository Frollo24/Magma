#shader vertex
#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(location = 0) out vec3 v_Normal;

layout(push_constant) uniform Push{
	mat4 viewProj;
	vec4 tint;
	vec3 offset;
} push;

void main() {
	gl_Position = push.viewProj * vec4(a_Position + push.offset, 1.0);
	v_Normal = a_Normal;
}

#shader fragment
#version 450 core

layout(location = 0) in vec3 v_Normal;

layout(location = 0) out vec4 o_Color;

layout(push_constant) uniform Push{
	mat4 viewProj;
	vec4 tint;
	vec3 offset;
} push;

void main() {
	vec3 normColor = v_Normal * 0.5 + 0.5;
	o_Color = vec4(normColor * push.tint.rgb, 1.0);
}