#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Normal;

layout(set = 0, binding = 0) uniform CameraUbo{
	mat4 viewProj;
	mat4 proj;
	mat4 view;
} camera;

layout(push_constant) uniform Push{
	mat4 model;
	vec4 tint;
} push;

void main() {
	gl_Position = camera.viewProj * push.model * vec4(a_Position, 1.0);
	v_Normal = a_Normal;
}

#shader fragment
#version 450 core

layout(location = 0) in vec3 v_Normal;

layout(location = 0) out vec4 o_Color;

struct DirLight {
	vec4 color;

	vec3 direction;
	float intensity;
};

layout(set = 0, binding = 1) uniform Lights{
	DirLight dirLight;
} lights;

layout(push_constant) uniform Push{
	mat4 model;
	vec4 tint;
} push;

void main() {
	vec3 normal = normalize(v_Normal);
	vec3 lightDir = normalize(-lights.dirLight.direction);

	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// specular shading
	vec3 viewDir = vec3(0.0, 0.0, 1.0);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 16);

	// combine results
	vec3 fragColor = push.tint.rgb;
	vec3 ambient = 0.03 * lights.dirLight.color.rgb * fragColor;
	vec3 diffuse = lights.dirLight.color.rgb * diff * fragColor;
	vec3 specular = lights.dirLight.color.rgb * spec * fragColor;

	vec3 color = ambient + (diffuse + specular) * lights.dirLight.intensity;
	color = ambient + diffuse + specular;

	o_Color = vec4(color, 1.0);
}