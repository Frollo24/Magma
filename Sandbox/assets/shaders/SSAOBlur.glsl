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
	v_TexCoord = 0.5 * v_TexCoord + 0.5;
}

#shader fragment
#version 450 core

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out float o_Color;

layout(set = 0, binding = 0) uniform sampler2D t_SSAOTexture; // For simplicity in host code

void main() {
	vec2 texelSize = 1.0 / vec2(textureSize(t_SSAOTexture, 0));
	float result = 0.0;
	for (int x = -2; x <= 2; x++) {
		for (int y = -2; y <= 2; y++) {
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(t_SSAOTexture, v_TexCoord + offset).r;
		}
	}
	o_Color = result / 25.0;
}