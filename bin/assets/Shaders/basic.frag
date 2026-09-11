#version 460 core

in vec2 v_UV;
in float v_AO;

flat in int v_TextureIndex;

uniform sampler2D u_TextureAtlas;

out vec4 FragColor;

const int ATLAS_SIZE = 256;
const int TILE_SIZE = 16;
const int ATLAS_COLUMNS = 16;

void main()
{
    /*
        Coordonnées locales dans la face.

        Exemple pour une face 4x3 :

        U : 0 -> 4
        V : 0 -> 3

        fract() nous donne la position
        à l'intérieur de la répétition actuelle.
    */
    vec2 localUV = fract(v_UV);

    /*
        Convertit 0..1 en pixel 0..15
        à l'intérieur de la texture 16x16.
    */
    int pixelX = int(
        floor(localUV.x * float(TILE_SIZE))
    );

    int pixelY = int(
        floor(localUV.y * float(TILE_SIZE))
    );

    /*
        Sécurité.
    */
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

    /*
        Position de la tuile dans l'atlas.
    */
    int tileX =
        v_TextureIndex % ATLAS_COLUMNS;

    int tileY =
        v_TextureIndex / ATLAS_COLUMNS;

    /*
        Pixel absolu dans l'atlas.
    */
    int atlasX =
        tileX * TILE_SIZE +
        pixelX;

    int atlasY =
        tileY * TILE_SIZE +
        pixelY;

    /*
        Lecture exacte d'un texel.
    */
    vec4 color =
        texelFetch(
            u_TextureAtlas,
            ivec2(atlasX, atlasY),
            0
        );

    /*
        Ambient occlusion.
    */
    color.rgb *=
        mix(
            0.55,
            1.0,
            v_AO
        );

    FragColor = color;
}