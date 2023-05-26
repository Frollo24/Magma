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

layout(push_constant) uniform Push {
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
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in vec3 v_PosWorld;
layout(location = 4) in mat3 v_TBN;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Coords;

#include "include/SceneSet.glslh"
#include "include/LightsStructs.glslh"

layout(set = 1, binding = 0) uniform sampler2D t_AlbedoTexture;
layout(set = 1, binding = 1) uniform sampler2D t_NormalTexture;
layout(set = 1, binding = 2) uniform sampler2D t_MetallicTexture;
layout(set = 1, binding = 3) uniform sampler2D t_RoughnessTexture;
layout(set = 1, binding = 4) uniform sampler2D t_EmissiveTexture;

layout(push_constant) uniform Push {
	mat4 model;
	vec4 tint;
	float metallic;
	float roughness;
} push;

#include "include/PBRFunctions.glslh"
#include "include/IBLFunctions.glslh"

vec3 calcDirLight(DirLight light, Material material, vec3 N, vec3 V, vec3 F0) {
    // Per-light radiance
    vec3 L = normalize(vec3(u_Scene.view * vec4(-light.direction, 0.0)));
    vec3 H = normalize(V + L);
    vec3 radiance = light.color.rgb * light.intensity;

    // Cook-Torrance BRDF
	vec3 Lo = CookTorranceBRDF(N, V, L, H, F0, material, radiance, light.intensity);
	return Lo;
}

vec3 calcPointLight(PointLight light, Material material, vec3 N, vec3 V, vec3 F0){
    // Per-light radiance
    vec3 L = normalize(vec3(u_Scene.view * vec4(light.position - v_PosWorld, 0.0)));
    vec3 H = normalize(V + L);
    float dist = length(light.position - v_PosWorld);
    float attenuation = 1.0 / (dist * dist);
    vec3 radiance = light.color.rgb * light.intensity * attenuation;

    // Cook-Torrance BRDF
    vec3 Lo = CookTorranceBRDF(N, V, L, H, F0, material, radiance, light.intensity);
	return Lo;
}

const float innerCutoff = cos(radians(15.0));
vec3 calcSpotLight(SpotLight light, Material material, vec3 N, vec3 V, vec3 F0){
    // Per-light radiance
    vec3 L = normalize(vec3(u_Scene.view * vec4(light.position - v_PosWorld, 0.0)));
    vec3 H = normalize(V + L);
    float dist = length(light.position - v_PosWorld);
    float attenuation = 1.0 / (dist * dist);
    vec3 radiance = light.color.rgb * light.intensity * attenuation;

	// Spot light cutoff factor
	float theta = dot(normalize(L), normalize(vec3(u_Scene.view * vec4(light.direction, 0.0))));
	float cutoff = cos(light.angle);
	float epsilon = innerCutoff - cutoff;
	//float factor = max(sign(theta - cutoff), 0.0);
	float factor = clamp((theta - cutoff) / epsilon, 0.0, 1.0);

    // Cook-Torrance BRDF
    vec3 Lo = CookTorranceBRDF(N, V, L, H, F0, material, radiance, light.intensity);
    return Lo * factor;
}

Material material;

const float e = 2.717281;
vec3 addFog(in vec3 c){
    float fogDist = length(v_Position);

    float f = clamp(pow(e, -pow(u_Fog.fogColor.w * fogDist, 2)), 0, 1);
    c = f * c + (1-f) * u_Fog.fogColor.rgb;

    return c;
}

void main(){
	vec4 texColor = texture(t_AlbedoTexture, v_TexCoord);
	vec4 texNormal = texture(t_NormalTexture, v_TexCoord);
	vec4 texMetallic = texture(t_MetallicTexture, v_TexCoord);
	vec4 texRoughness = texture(t_RoughnessTexture, v_TexCoord);

	material.albedo = texColor * push.tint;
	vec3 ambient = 0.03 * material.albedo.rgb;

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
	vec3 N = normalize(normal);
    vec3 V = normalize(-v_Position);

    vec3 F0 = mix(vec3(0.04), material.albedo.rgb, vec3(material.metallic));

	vec3 light = vec3(0.0);
	for (int i = 0; i < NR_DIR_LIGHTS; i++)
		light += calcDirLight(u_Lights.dirLight[i], material, N, V, F0);
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
		light += calcPointLight(u_Lights.pointLight[i], material, N, V, F0);
	for (int i = 0; i < NR_SPOT_LIGHTS; i++)
		light += calcSpotLight(u_Lights.spotLight[i], material, N, V, F0);

	float NdotV = max(dot(N, V), 0.0000001);
    vec3 F = FresnelSchlick(NdotV, F0);
	vec3 kD = (1.0 - F) * (1.0 - material.metallic);
	vec3 R = reflect(-V, N);
	vec3 environment = IBL(material, N, V);

	vec3 diffuse = textureLod(t_IrradianceMap, R, 10).rgb * material.albedo.rgb * kD;
	vec3 specular = environment * F * (1.0 - material.roughness);

	vec3 hdrColor = ambient + light + diffuse + specular;
	vec3 color = vec3(1.0) - exp(-hdrColor * u_PhysCamera.exposure);
	color = addFog(color);
	o_Color = vec4(color, 1.0);

	o_Coords = vec4(v_Position.xyz, 1.0);
}