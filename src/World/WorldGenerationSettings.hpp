#pragma once

namespace Voxel::WorldGenerationSettings
{
    // Nombre de chunks générés autour du centre du monde.
    inline constexpr int WORLD_RADIUS = 8;

    // Fréquence des cartes de température et d'humidité.
    inline constexpr float TEMPERATURE_FREQUENCY = 0.0012f;
    inline constexpr float HUMIDITY_FREQUENCY = 0.0015f;

    // Densité de végétation par biome.
    inline constexpr float FOREST_TREE_DENSITY = 0.01f;
    inline constexpr float PLAINS_TREE_DENSITY = 0.005f;
    inline constexpr float SAVANNA_TREE_DENSITY = 0.003f;
    inline constexpr float DESERT_CACTUS_DENSITY = 0.002f;

    // Taille des arbres.
    inline constexpr int TREE_MIN_HEIGHT = 4;
    inline constexpr int TREE_HEIGHT_VARIATION = 3;
    inline constexpr int TREE_GENERATION_MARGIN = 3;

    // Taille des cactus.
    inline constexpr int CACTUS_MIN_HEIGHT = 2;
    inline constexpr int CACTUS_HEIGHT_VARIATION = 3;

    // Hauteur et relief du terrain.
    inline constexpr float BASE_TERRAIN_HEIGHT = 28.0f;
    inline constexpr float HILL_HEIGHT = 40.0f;
    inline constexpr float DETAIL_HEIGHT = 4.0f;
    inline constexpr float MOUNTAIN_HEIGHT = 90.0f;
}
