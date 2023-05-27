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
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Coords;

struct DirLight {
	vec4 color;

	vec3 direction;
	float intensity;
};

layout(set = 0, binding = 1) uniform Lights{
	DirLight dirLight;
} lights;

layout(set = 1, binding = 0) uniform sampler2D t_PositionTexture;
layout(set = 1, binding = 1) uniform sampler2D t_PosWorldTexture;
layout(set = 1, binding = 2) uniform sampler2D t_AlbedoTexture;
layout(set = 1, binding = 3) uniform sampler2D t_NormalMetalRoughnessTexture;
layout(set = 1, binding = 4) uniform sampler2D t_SSAOTexture;

layout(push_constant) uniform Push{
	mat4 model;
	vec4 tint;
} push;

void main() {
	vec4 texColor = texture(t_AlbedoTexture, v_TexCoord);
	if (texColor.a > 0.5) discard;

	vec2 normParams = texture(t_NormalMetalRoughnessTexture, v_TexCoord).xy;
	vec3 normal = vec3(normParams, sqrt(1.0 - normParams.x * normParams.x - normParams.y * normParams.y));
	normal = normalize(normal);
	vec3 lightDir = normalize(-lights.dirLight.direction);

	// ambient occlusion
	float AO = texture(t_SSAOTexture, v_TexCoord).r;

	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// specular shading
	vec3 viewDir = vec3(0.0, 0.0, 1.0);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 16);

	// combine results
	vec3 fragColor = texColor.rgb;
	vec3 ambient = 0.03 * lights.dirLight.color.rgb * fragColor;
	vec3 diffuse = lights.dirLight.color.rgb * diff * fragColor;
	vec3 specular = lights.dirLight.color.rgb * spec * fragColor;

	vec3 color = (ambient * AO) + (diffuse + specular) * lights.dirLight.intensity;
	o_Color = vec4(color, 1.0);
}