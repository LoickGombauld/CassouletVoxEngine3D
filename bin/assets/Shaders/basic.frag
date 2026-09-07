#version 460 core

in vec2 v_UV;
in float v_AO;

flat in int v_TextureIndex;

uniform sampler2D u_TextureAtlas;

out vec4 FragColor;

const int ATLAS_COLUMNS = 16;
const int ATLAS_ROWS = 16;

void main()
{
    vec2 localUV = fract(v_UV);

    int column = v_TextureIndex % ATLAS_COLUMNS;
    int row = v_TextureIndex / ATLAS_COLUMNS;

    vec2 tileSize = vec2(
        1.0 / float(ATLAS_COLUMNS),
        1.0 / float(ATLAS_ROWS)
    );

    vec2 atlasUV =
        vec2(
            float(column),
            float(row)
        ) * tileSize
        + localUV * tileSize;

    vec4 color =
        texture(u_TextureAtlas, atlasUV);

    color.rgb *= mix(0.55, 1.0, v_AO);

    FragColor = color;
}