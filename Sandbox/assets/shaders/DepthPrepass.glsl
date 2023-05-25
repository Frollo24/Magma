#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(push_constant) uniform Push
{
	mat4 modelViewProj;
} push;

void main()
{
	gl_Position = push.modelViewProj * vec4(a_Position, 1.0);
}

#shader fragment
#version 450 core

const float c_DepthBias = 0.0005;

void main()
{
	gl_FragDepth = gl_FragCoord.z + c_DepthBias;
}