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

#include "include/SceneSet.glslh"
#include "include/LightsStructs.glslh"

layout(set = 1, binding = 0) uniform sampler2D t_PositionTexture;
layout(set = 1, binding = 1) uniform sampler2D t_PosWorldTexture;
layout(set = 1, binding = 2) uniform sampler2D t_AlbedoTexture;
layout(set = 1, binding = 3) uniform sampler2D t_NormalMetalRoughnessTexture;

#include "include/PBRFunctions.glslh"

vec3 calcDirLight(DirLight light, Material material, vec3 N, vec3 V, vec3 F0) {
	// Per-light radiance
	vec3 L = normalize(vec3(u_Scene.view * vec4(-light.direction, 0.0)));
	vec3 H = normalize(V + L);
	vec3 radiance = light.color.rgb * light.intensity;

	// Cook-Torrance BRDF
	vec3 Lo = CookTorranceBRDF(N, V, L, H, F0, material, radiance, light.intensity);
	return Lo;
}

vec3 calcPointLight(PointLight light, Material material, vec3 N, vec3 V, vec3 F0) {
	// Per-light radiance
	vec3 posWorld = texture(t_PosWorldTexture, v_TexCoord).xyz;
	vec3 L = normalize(vec3(u_Scene.view * vec4(light.position - posWorld, 0.0)));
	vec3 H = normalize(V + L);
	float dist = length(light.position - posWorld);
	float attenuation = 1.0 / (dist * dist);
	vec3 radiance = light.color.rgb * light.intensity * attenuation;

	// Cook-Torrance BRDF
	vec3 Lo = CookTorranceBRDF(N, V, L, H, F0, material, radiance, light.intensity);
	return Lo;
}

const float innerCutoff = cos(radians(15.0));
vec3 calcSpotLight(SpotLight light, Material material, vec3 N, vec3 V, vec3 F0) {
	// Per-light radiance
	vec3 posWorld = texture(t_PosWorldTexture, v_TexCoord).xyz;
	vec3 L = normalize(vec3(u_Scene.view * vec4(light.position - posWorld, 0.0)));
	vec3 H = normalize(V + L);
	float dist = length(light.position - posWorld);
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
vec3 addFog(in vec3 c) {
	float fogDist = length(-texture(t_PositionTexture, v_TexCoord).xyz);

	float f = clamp(pow(e, -pow(u_Fog.fogColor.w * fogDist, 2)), 0, 1);
	c = f * c + (1 - f) * u_Fog.fogColor.rgb;

	return c;
}

void main() {
	vec4 texPosition = texture(t_PositionTexture, v_TexCoord);
	vec4 texPosWorld = texture(t_PosWorldTexture, v_TexCoord);
	vec4 texColor = texture(t_AlbedoTexture, v_TexCoord);
	vec4 texNormalMetRough = texture(t_NormalMetalRoughnessTexture, v_TexCoord);

	if (texPosition.w == 0.0) discard;

	material.albedo = texColor;
	material.metallic = texNormalMetRough.z; // Metallic
	material.roughness = texNormalMetRough.w; // Roughness

	vec2 normParams = texNormalMetRough.xy;
	vec3 normal = vec3(normParams, sqrt(1.0 - normParams.x * normParams.x - normParams.y * normParams.y));
	vec3 N = normalize(normal);
	vec3 V = normalize(-texPosition.xyz);

	vec3 ambient = 0.03 * material.albedo.rgb;
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
	vec3 IBL = vec3(0.15); // TODO: sample skybox color here when possible
	vec3 diffuse = IBL * material.albedo.rgb * kD;
	vec3 specular = IBL * F * (1.0 - material.roughness);

	vec3 hdrColor = ambient + light + diffuse + specular;
	vec3 color = vec3(1.0) - exp(-hdrColor * u_PhysCamera.exposure);
	color = addFog(color);
	o_Color = vec4(color, 1.0);

	o_Coords = vec4(texPosition.xyz, 1.0);
}