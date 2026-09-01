#version 460 core

in vec3 v_Normal;
in vec2 v_UV;
in float v_AO;

out vec4 FragColor;

void main()
{
    vec3 normal =
        normalize(v_Normal);

    vec3 lightDirection =
        normalize(
            vec3(
                -0.5,
                1.0,
                -0.3
            )
        );

    float diffuse =
        max(
            dot(
                normal,
                lightDirection
            ),
            0.2
        );

    /*
        AO:

        0 = aucun blocage
        1 = occlusion maximale
    */

    float ambient =
        1.0 -
        v_AO * 0.65;

    vec3 baseColor =
        vec3(
            0.55,
            0.70,
            0.45
        );

    vec3 finalColor =
        baseColor *
        diffuse *
        ambient;

    FragColor =
        vec4(
            finalColor,
            1.0
        );
}