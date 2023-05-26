#shader vertex
#version 450 core

layout(location = 0) in vec3 a_SkyboxVertices;

layout(set = 0, binding = 0) uniform SceneUbo{
    mat4 viewProj;
    mat4 proj;
    mat4 view;
} u_Scene;

layout(location = 0) out vec3 v_TexCoords;

void main()
{
    v_TexCoords = a_SkyboxVertices;
    vec4 pos = u_Scene.viewProj * vec4(a_SkyboxVertices, 0.0);
    gl_Position = pos.xyww;
}

#shader fragment
#version 450 core

layout(location = 0) in vec3 v_TexCoords;

layout(set = 0, binding = 5) uniform samplerCube t_Skybox;

layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = texture(t_Skybox, v_TexCoords);
}