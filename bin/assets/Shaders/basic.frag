#version 460 core

in vec2 v_UV;
in float v_AO;

flat in int v_TextureIndex;

uniform sampler2D u_TextureAtlas;

out vec4 FragColor;

const int ATLAS_COLUMNS = 16;
const int ATLAS_ROWS = 16;

vec2 getAtlasUV(
    vec2 localUV,
    int textureIndex
)
{
    vec2 uv = fract(localUV);

    int column =
        textureIndex % ATLAS_COLUMNS;

    int row =
        textureIndex / ATLAS_COLUMNS;

    vec2 tileSize = vec2(
        1.0 / float(ATLAS_COLUMNS),
        1.0 / float(ATLAS_ROWS)
    );

    return vec2(
        float(column) * tileSize.x +
            uv.x * tileSize.x,

        float(row) * tileSize.y +
            uv.y * tileSize.y
    );
}

void main()
{
    vec2 atlasUV =
        getAtlasUV(
            v_UV,
            v_TextureIndex
        );

    vec4 color =
        texture(
            u_TextureAtlas,
            atlasUV
        );

    color.rgb *=
        mix(
            0.55,
            1.0,
            v_AO
        );

    FragColor = color;
}