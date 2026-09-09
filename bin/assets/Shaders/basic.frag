#version 460 core

in vec2 v_UV;

out vec4 FragColor;

void main()
{
    vec2 uv = fract(v_UV);

    FragColor = vec4(
        uv.x,
        uv.y,
        0.0,
        1.0
    );
}