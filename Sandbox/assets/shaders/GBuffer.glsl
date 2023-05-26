#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec2 v_TexCoord;

layout(location = 3) out vec3 v_PosWorld;
layout(location = 4) out mat3 v_TBN;

#include "include/SceneSet.glslh"

layout(push_constant) uniform Push{
	mat4 model;
	vec4 tint;
	float metallic;
	float roughness;
} push;

void main() {
	mat4 space = u_Scene.view * push.model;
	mat3 normal = mat3(transpose(inverse(space)));
	gl_Position = u_Scene.viewProj * push.model * vec4(a_Position, 1.0);
	v_Position = (space * vec4(a_Position, 1.0)).xyz;
	v_Normal = normal * a_Normal;
	v_TexCoord = a_TexCoord;

	vec3 T = normalize(mat3(space) * a_Tangent);
	vec3 B = normalize(mat3(space) * a_Bitangent);
	vec3 N = normalize(normal * a_Normal);
	mat3 TBN = mat3(T, B, N);

	v_PosWorld = (push.model * vec4(a_Position, 1.0)).xyz;
	v_TBN = TBN;
}

#shader fragment
#version 450 core

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in vec3 v_PosWorld;
layout(location = 4) in mat3 v_TBN;

layout(location = 0) out vec4 o_Position;
layout(location = 1) out vec4 o_PosWorld;
layout(location = 2) out vec4 o_Albedo;
layout(location = 3) out vec4 o_NormalMetalRoughness;
// layout(location = 4) out vec4 o_EmissiveColor; // We don't support emissive colors at the moment

layout(set = 1, binding = 0) uniform sampler2D t_AlbedoTexture;
layout(set = 1, binding = 1) uniform sampler2D t_NormalTexture;
layout(set = 1, binding = 2) uniform sampler2D t_MetallicTexture;
layout(set = 1, binding = 3) uniform sampler2D t_RoughnessTexture;
layout(set = 1, binding = 4) uniform sampler2D t_EmissiveTexture;

layout(push_constant) uniform Push{
	mat4 model;
	vec4 tint;
	float metallic;
	float roughness;
} push;

struct Material {
	vec4 albedo;
	float metallic;
	float roughness;
};
Material material;

void main() {
	vec4 texColor = texture(t_AlbedoTexture, v_TexCoord);
	vec4 texNormal = texture(t_NormalTexture, v_TexCoord);
	vec4 texMetallic = texture(t_MetallicTexture, v_TexCoord);
	vec4 texRoughness = texture(t_RoughnessTexture, v_TexCoord);

	material.albedo = texColor * push.tint;

	int texSize = textureSize(t_MetallicTexture, 0).x;
	float matMetallic = int(texSize == 1) * push.metallic + int(texSize != 1) * texMetallic.r;
	material.metallic = matMetallic;

	texSize = textureSize(t_RoughnessTexture, 0).x;
	float matRoughness = int(texSize == 1) * push.roughness + int(texSize != 1) * texRoughness.r;
	matRoughness = max(0.04, matRoughness); // Preserve specular highlights
	material.roughness = matRoughness;

	texNormal = texNormal * 2.0 - 1.0;
	texSize = textureSize(t_NormalTexture, 0).x;
	vec3 normal = int(texSize == 1) * v_Normal + int(texSize != 1) * v_TBN * texNormal.xyz;
	normal = normalize(normal);

	o_Position = vec4(v_Position.xyz, 1.0);
	o_PosWorld = vec4(v_PosWorld.xyz, 1.0);
	o_Albedo = texColor * push.tint;
	o_NormalMetalRoughness = vec4(normal.xy, material.metallic, material.roughness);
	// o_EmissiveColor = vec4(0.0); // We don't support emissive colors at the moment
}