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

layout(set = 0, binding = 0) uniform CameraUbo{
	mat4 viewProj;
	mat4 proj;
	mat4 view;
} camera;

layout(set = 1, binding = 0) uniform sampler2D t_PositionTexture;
layout(set = 1, binding = 3) uniform sampler2D t_NormalMetalRoughnessTexture;

layout(set = 2, binding = 0) uniform KernelSamples{
	vec4 samples[64]; // Don't bother with UBO alignment
} kernels;
layout(set = 2, binding = 1) uniform sampler2D t_NoiseTexture;

const int kernelSize = 64;
const float radius = 0.5;
const float bias = 0.025;
const vec2 noiseScale = vec2(1600.0 / 4.0, 900.0 / 4.0); // Based on screen size

void main() {
	vec4 texPosition = texture(t_PositionTexture, v_TexCoord);
	vec4 texNormalMetRough = texture(t_NormalMetalRoughnessTexture, v_TexCoord);

	if (texPosition.w == 0.0) discard;

	vec3 fragPos = texPosition.xyz;
	vec2 normParams = texNormalMetRough.xy;
	vec3 normal = vec3(normParams, sqrt(1.0 - normParams.x * normParams.x - normParams.y * normParams.y));
	normal = normalize(normal);

	vec3 randomVec = normalize(texture(t_NoiseTexture, v_TexCoord * noiseScale).xyz);
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < kernelSize; i++) {
		// Get sample position in view-space from tangent-space
		vec3 samplePos = TBN * kernels.samples[i].xyz;
		samplePos = fragPos + samplePos * radius;

		// Get sample position in screen-space
		vec4 offset = vec4(samplePos, 1.0);
		offset = camera.proj * offset; // From view-space from clip-space
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		// Get possible occluder corresponding to sample position in screen-space
		vec3 occluderPos = texture(t_PositionTexture, offset.xy).rgb;

		// Occluding geometry shouldn't count if it's outside the dome radius
		float rangeCheck = smoothstep(0.0, 1.0, radius / length(fragPos - occluderPos));

		// In view-space, greater Z values are always closer to the camera
		occlusion += (occluderPos.z >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	o_Color = 1.0 - (occlusion / kernelSize);
}