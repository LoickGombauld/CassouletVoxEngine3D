#version 460 core

in vec3 v_Normal;
in vec2 v_UV;

out vec4 FragColor;

void main()
{
    FragColor = vec4(
        0.2 + v_Normal * 0.5,
        1.0
    );
}