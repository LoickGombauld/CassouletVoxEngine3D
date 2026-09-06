#version 460 core

in vec3 v_Normal;
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
    /*
        UV locales :
            (0..width, 0..height)

        On garde uniquement la partie
        correspondant à la répétition actuelle.
    */

    vec2 uv = fract(localUV);

    const float tileWidth =
        1.0 / float(ATLAS_COLUMNS);

    const float tileHeight =
        1.0 / float(ATLAS_ROWS);

    const int column =
        textureIndex % ATLAS_COLUMNS;

    const int row =
        textureIndex / ATLAS_COLUMNS;

    /*
        Centre du texel pour éviter de tomber
        exactement sur les frontières des tuiles.
    */

    const float texelWidth =
        1.0 / 256.0;

    const float texelHeight =
        1.0 / 256.0;

    vec2 tileUV;

    tileUV.x =
        float(column) * tileWidth +
        texelWidth * 0.5 +
        uv.x * (tileWidth - texelWidth);

    tileUV.y =
        float(row) * tileHeight +
        texelHeight * 0.5 +
        uv.y * (tileHeight - texelHeight);

    return tileUV;
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

    float ao =
        mix(0.55, 1.0, v_AO);

    color.rgb *= ao;

    FragColor = color;
}