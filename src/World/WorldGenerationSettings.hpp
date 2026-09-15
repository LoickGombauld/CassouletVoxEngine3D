#pragma once

namespace Voxel::WorldGenerationSettings
{
    // Nombre de chunks générés autour du centre du monde.
    inline constexpr int WORLD_RADIUS = 8;
    inline constexpr int STREAMING_RADIUS = 8;
    inline constexpr int STREAMING_UNLOAD_RADIUS = 10;
    inline constexpr int STREAMING_MAX_COMPLETIONS_PER_FRAME = 2;

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
    inline constexpr int TREE_HEIGHT_VARIATION = 8;
    inline constexpr int TREE_GENERATION_MARGIN = 6;

    // Taille des cactus.
    inline constexpr int CACTUS_MIN_HEIGHT = 2;
    inline constexpr int CACTUS_HEIGHT_VARIATION = 3;

    // Hauteur et relief du terrain.
    inline constexpr float BASE_TERRAIN_HEIGHT = 28.0f;
    inline constexpr float HILL_HEIGHT = 40.0f;
    inline constexpr float DETAIL_HEIGHT = 4.0f;
    inline constexpr float MOUNTAIN_HEIGHT = 90.0f;

    // Espacement des échantillons de la heightmap.
    inline constexpr int TERRAIN_SAMPLE_STEP = 4;

    inline constexpr int SEA_LEVEL = 32;

    // Lacs garantis au démarrage.
    inline constexpr bool GUARANTEE_LAKES = true;
    inline constexpr int LAKE_SPAWN_CHUNK_X = 0;
    inline constexpr int LAKE_SPAWN_CHUNK_Z = 5;
    inline constexpr int LAKE_CENTER_OFFSET_X = 8;
    inline constexpr int LAKE_CENTER_OFFSET_Z = 8;
    inline constexpr int LAKE_RADIUS = 100;
    inline constexpr int LAKE_BOTTOM = 1;
    inline constexpr int LAKE_DEPTH = 12;
    inline constexpr float LAKE_SHAPE_VARIATION = 0.12f;

    // Zone désertique garantie dans la zone initiale.
    inline constexpr bool GUARANTEE_DESERT = false;
    inline constexpr int DESERT_SPAWN_CHUNK_X = -5;
    inline constexpr int DESERT_SPAWN_CHUNK_Z = -5;
    inline constexpr int DESERT_CENTER_OFFSET_X = 8;
    inline constexpr int DESERT_CENTER_OFFSET_Z = 8;
    inline constexpr int DESERT_RADIUS = 8;

    // Rivière traversant la zone générée au démarrage.
    inline constexpr bool GUARANTEE_RIVER = true;
    inline constexpr float RIVER_FREQUENCY = 0.025f;
    inline constexpr float RIVER_AMPLITUDE = 36.0f;
    inline constexpr int RIVER_WIDTH = 3;
    inline constexpr int RIVER_BOTTOM = 28;
    inline constexpr int RIVER_BANK_OFFSET = 1;

    // Génération des tunnels souterrains.
    inline constexpr bool GENERATE_CAVE_TUNNELS = true;
    inline constexpr float CAVE_TUNNEL_FREQUENCY = 0.035f;
    inline constexpr float CAVE_VERTICAL_FREQUENCY = 0.060f;
    inline constexpr float CAVE_TUNNEL_THRESHOLD = 0.48f;
    inline constexpr int CAVE_MIN_Y = 4;
    inline constexpr int CAVE_SURFACE_DEPTH = 5;
    inline constexpr int MOUNTAIN_CAVE_ENTRY_HEIGHT = 85;
}
