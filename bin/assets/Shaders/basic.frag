#version 460 core

in vec2 v_UV;
in float v_AO;

flat in int v_TextureIndex;

uniform sampler2D u_TextureAtlas;
uniform int u_RenderWater;

out vec4 FragColor;

const int ATLAS_SIZE = 256;
const int TILE_SIZE = 16;
const int ATLAS_COLUMNS = 16;

void main()
{
    // ------------------------------------------------------------
    // UV locale à l'intérieur de la texture répétée
    // ------------------------------------------------------------

    vec2 localUV = fract(v_UV);

    // ------------------------------------------------------------
    // Conversion UV -> pixel de la texture 16x16
    // ------------------------------------------------------------

    int pixelX = int(
        floor(localUV.x * float(TILE_SIZE))
    );

    int pixelY = int(
        floor(localUV.y * float(TILE_SIZE))
    );

    // Sécurité : toujours rester dans 0..15
    pixelX = clamp(
        pixelX,
        0,
        TILE_SIZE - 1
    );

    pixelY = clamp(
        pixelY,
        0,
        TILE_SIZE - 1
    );

    // ------------------------------------------------------------
    // Position de la texture dans l'atlas
    // ------------------------------------------------------------

    int tileX =
        v_TextureIndex % ATLAS_COLUMNS;

    int tileY =
        v_TextureIndex / ATLAS_COLUMNS;

    // ------------------------------------------------------------
    // Position absolue du texel dans l'atlas
    // ------------------------------------------------------------

    int atlasX =
        tileX * TILE_SIZE +
        pixelX;

    int atlasY =
        tileY * TILE_SIZE +
        pixelY;

    // ------------------------------------------------------------
    // Lecture EXACTE du texel
    //
    // Important :
    // on n'utilise surtout pas texture() ici.
    // ------------------------------------------------------------

    vec4 color =
        texelFetch(
            u_TextureAtlas,
            ivec2(atlasX, atlasY),
            0
        );

    if (u_RenderWater == 0 && v_TextureIndex == 5)
    {
        discard;
    }

    if (u_RenderWater == 1 && v_TextureIndex != 5)
    {
        discard;
    }

    // ------------------------------------------------------------
    // Ambient Occlusion
    // ------------------------------------------------------------

    color.rgb *=
        mix(
            0.55,
            1.0,
            v_AO
        );

    if (v_TextureIndex == 5)
    {
        color.a = 0.55;
    }

    FragColor = color;
}