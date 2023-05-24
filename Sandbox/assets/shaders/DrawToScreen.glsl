#shader vertex
#version 450 core
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec2 v_TexCoord;

const vec2 ScreenCoords[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(1.0, 1.0),
	vec2(1.0, 1.0),
	vec2(-1.0, 1.0),
	vec2(-1.0, -1.0)
);

void main() {
	v_TexCoord = ScreenCoords[gl_VertexIndex];
	gl_Position = vec4(v_TexCoord, 0.0, 1.0);
	v_TexCoord.y = -v_TexCoord.y;
	v_TexCoord = 0.5 * v_TexCoord + 0.5;
}

#shader fragment
#version 450 core

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 0) uniform sampler2D t_ScreenTexture;

void main() {
	o_Color = texture(t_ScreenTexture, v_TexCoord);
}