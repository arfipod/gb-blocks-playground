#include "chunk.h"
#include "tile_defs.h"
#include "world.h"

static uint8_t active_chunk;
static uint16_t generation_seed = WORLD_DEFAULT_SEED;

void chunk_set_seed(uint16_t seed)
{
    generation_seed = seed;
}

uint16_t chunk_seed(void)
{
    return generation_seed;
}

static uint8_t clamp_surface(uint8_t y)
{
    if (y < 7u) {
        return 7u;
    }

    if (y > 13u) {
        return 13u;
    }

    return y;
}

static uint8_t column_surface(uint16_t x)
{
    uint8_t seed_lo = (uint8_t)generation_seed;
    uint8_t seed_hi = (uint8_t)(generation_seed >> 8);
    Biome biome = world_biome_at_x(x);
    uint8_t wave = (uint8_t)((((x + seed_hi) >> 2) & 1u) +
                             (((x + seed_lo) >> 4) & 1u));
    uint8_t noise = (uint8_t)((x * 17u + 23u + seed_lo + (seed_hi * 3u)) & 31u);
    uint8_t surface = (uint8_t)(10u + wave);

    switch (biome) {
    case BIOME_ROOTWOOD_GROVE:
        surface = (uint8_t)(9u + wave);
        break;
    case BIOME_STONE_BELT:
        surface = (uint8_t)(11u + (wave >> 1));
        break;
    case BIOME_CELESTIAL_RUINS:
        surface = (uint8_t)(9u + wave + ((x >> 3) & 1u));
        break;
    default:
        break;
    }

    if (noise < 5u) {
        --surface;
    } else if (noise > 25u) {
        ++surface;
    }

    if (((x + (x >> 3) + seed_hi) & 15u) == 0u) {
        --surface;
    }

    if (biome == BIOME_CELESTIAL_RUINS && ((x + seed_lo) & 7u) == 0u) {
        surface = (uint8_t)(surface - 2u);
    }

    return clamp_surface(surface);
}

static uint8_t tree_base_at(uint16_t x)
{
    uint8_t seed_lo = (uint8_t)generation_seed;
    uint8_t tree_noise;
    Biome biome = world_biome_at_x(x);

    if (x < 4u || x > (WORLD_WIDTH_TILES - 5u)) {
        return 0u;
    }

    tree_noise = (uint8_t)((x * 29u + 11u + seed_lo) & 31u);

    switch (biome) {
    case BIOME_ROOTWOOD_GROVE:
        return tree_noise == 3u || tree_noise == 11u || tree_noise == 23u;
    case BIOME_STONE_BELT:
        return tree_noise == 3u && ((x >> 2) & 1u) == 0u;
    case BIOME_CELESTIAL_RUINS:
        return tree_noise == 7u;
    default:
        return tree_noise == 3u;
    }
}

static uint8_t surface_tile_for_biome(Biome biome, uint8_t noise)
{
    switch (biome) {
    case BIOME_STONE_BELT:
        return noise < 18u ? TILE_STONE : TILE_GRASS;
    case BIOME_CELESTIAL_RUINS:
        return noise < 20u ? TILE_STONE : TILE_GRASS;
    default:
        return TILE_GRASS;
    }
}

static uint8_t cave_threshold_for_biome(Biome biome)
{
    switch (biome) {
    case BIOME_ROOTWOOD_GROVE:
        return 2u;
    case BIOME_STONE_BELT:
        return 4u;
    case BIOME_CELESTIAL_RUINS:
        return 6u;
    default:
        return 3u;
    }
}

static uint8_t dirt_depth_for_biome(Biome biome)
{
    switch (biome) {
    case BIOME_ROOTWOOD_GROVE:
        return 5u;
    case BIOME_STONE_BELT:
        return 2u;
    case BIOME_CELESTIAL_RUINS:
        return 2u;
    default:
        return 4u;
    }
}

static uint8_t tree_tile_at(uint16_t x, uint8_t y)
{
    int8_t dx;

    for (dx = -2; dx <= 2; ++dx) {
        int16_t base_x_signed = (int16_t)x + dx;
        uint16_t base_x;
        uint8_t base_y;

        if (base_x_signed < 0 || base_x_signed >= WORLD_WIDTH_TILES) {
            continue;
        }

        base_x = (uint16_t)base_x_signed;

        if (!tree_base_at(base_x)) {
            continue;
        }

        base_y = column_surface(base_x);

        if (x == base_x && y >= (uint8_t)(base_y - 4u) && y < base_y) {
            return TILE_WOOD;
        }

        if (y >= (uint8_t)(base_y - 6u) && y <= (uint8_t)(base_y - 3u)) {
            return TILE_LEAVES;
        }
    }

    return TILE_EMPTY;
}

uint8_t chunk_generate_tile(uint8_t chunk_index, uint8_t local_x, uint8_t y)
{
    uint16_t x = (uint16_t)(((uint16_t)chunk_index * CHUNK_WIDTH_TILES) + local_x);
    Biome biome = world_biome_at_x(x);
    uint8_t surface = column_surface(x);
    uint8_t tree_tile = tree_tile_at(x, y);
    uint8_t seed_lo = (uint8_t)generation_seed;
    uint8_t seed_hi = (uint8_t)(generation_seed >> 8);
    uint8_t cave_noise = (uint8_t)((x * 13u + y * 23u + chunk_index * 7u +
                                    seed_lo + (seed_hi * 5u)) & 31u);
    uint8_t ore_noise = (uint8_t)((x * 19u + y * 11u + chunk_index * 5u +
                                   (seed_lo * 3u) + seed_hi) & 63u);
    uint8_t cave_threshold = cave_threshold_for_biome(biome);
    uint8_t dirt_depth = dirt_depth_for_biome(biome);

    if (tree_tile != TILE_EMPTY) {
        return tree_tile;
    }

    if (y < surface) {
        return TILE_EMPTY;
    }

    if (y == surface) {
        return surface_tile_for_biome(biome, cave_noise);
    }

    if (y > (uint8_t)(surface + 2u) &&
        y < (uint8_t)(WORLD_HEIGHT_TILES - 1u) &&
        cave_noise < cave_threshold) {
        return TILE_EMPTY;
    }

    if (biome == BIOME_STONE_BELT &&
        y <= (uint8_t)(surface + 3u) &&
        ((x + y + seed_hi) & 3u) != 0u) {
        return TILE_STONE;
    }

    if (y < (uint8_t)(surface + dirt_depth)) {
        return TILE_DIRT;
    }

    if (biome == BIOME_CELESTIAL_RUINS &&
        y > (uint8_t)(surface + 2u) &&
        (ore_noise == 0u || ore_noise == 1u)) {
        return TILE_IRON;
    }

    if (y > (uint8_t)(surface + 4u) &&
        ore_noise >= 1u &&
        ore_noise <= (biome == BIOME_STONE_BELT ? 5u : 3u)) {
        return TILE_IRON;
    }

    if (y > (uint8_t)(surface + 3u) &&
        ore_noise >= 4u &&
        ore_noise <= (biome == BIOME_STONE_BELT ? 12u : 8u)) {
        return TILE_COPPER;
    }

    if (ore_noise >= 9u &&
        ore_noise <= (biome == BIOME_STONE_BELT || biome == BIOME_CELESTIAL_RUINS ? 20u : 16u)) {
        return TILE_COAL;
    }

    return TILE_STONE;
}

void chunk_tick(uint16_t camera_x)
{
    uint16_t center_x = (uint16_t)(camera_x + 80u);
    uint8_t center_chunk = (uint8_t)((center_x >> 3) / CHUNK_WIDTH_TILES);
    uint8_t start_chunk;

    if (center_chunk >= WORLD_CHUNK_COUNT) {
        center_chunk = (uint8_t)(WORLD_CHUNK_COUNT - 1u);
    }

    if (center_chunk < 2u) {
        start_chunk = 0u;
    } else {
        start_chunk = (uint8_t)(center_chunk - 2u);
    }

    active_chunk = world_stream_to_chunk(start_chunk);
}

uint8_t chunk_active_index(void)
{
    return active_chunk;
}
