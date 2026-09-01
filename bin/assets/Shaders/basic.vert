#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;
layout(location = 3) in float a_AO;
layout(location = 4) in float a_TextureIndex;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_Normal;
out vec2 v_UV;
out float v_AO;
out float v_TextureIndex;

void main()
{
    gl_Position =
        u_Projection *
        u_View *
        u_Model *
        vec4(a_Position, 1.0);

    v_Normal = a_Normal;
    v_UV = a_UV;
    v_AO = a_AO;
    v_TextureIndex = a_TextureIndex;
}