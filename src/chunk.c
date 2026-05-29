#include "chunk.h"
#include "tile_defs.h"
#include "world.h"

static uint8_t active_chunk;

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
    uint8_t wave = (uint8_t)(((x >> 2) & 1u) + ((x >> 4) & 1u));
    uint8_t noise = (uint8_t)((x * 17u + 23u) & 31u);
    uint8_t surface = (uint8_t)(10u + wave);

    if (noise < 5u) {
        --surface;
    } else if (noise > 25u) {
        ++surface;
    }

    if (((x + (x >> 3)) & 15u) == 0u) {
        --surface;
    }

    return clamp_surface(surface);
}

static uint8_t tree_base_at(uint16_t x)
{
    if (x < 4u || x > (WORLD_WIDTH_TILES - 5u)) {
        return 0u;
    }

    return (uint8_t)(((x * 29u + 11u) & 31u) == 3u);
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
    uint8_t surface = column_surface(x);
    uint8_t tree_tile = tree_tile_at(x, y);
    uint8_t cave_noise = (uint8_t)((x * 13u + y * 23u + chunk_index * 7u) & 31u);

    if (tree_tile != TILE_EMPTY) {
        return tree_tile;
    }

    if (y < surface) {
        return TILE_EMPTY;
    }

    if (y == surface) {
        return TILE_GRASS;
    }

    if (y > (uint8_t)(surface + 2u) && y < 17u && cave_noise < 3u) {
        return TILE_EMPTY;
    }

    if (y < (uint8_t)(surface + 4u)) {
        return TILE_DIRT;
    }

    return TILE_STONE;
}

void chunk_tick(uint16_t camera_x)
{
    uint8_t next_chunk = (uint8_t)((camera_x >> 3) / CHUNK_WIDTH_TILES);

    if (next_chunk >= WORLD_CHUNK_COUNT) {
        next_chunk = (uint8_t)(WORLD_CHUNK_COUNT - 1u);
    }

    active_chunk = world_stream_to_chunk(next_chunk);
}

uint8_t chunk_active_index(void)
{
    return active_chunk;
}
