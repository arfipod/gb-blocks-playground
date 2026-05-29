#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "chunk.h"
#include "tile_defs.h"

#define WORLD_WIDTH_TILES (WORLD_CHUNK_COUNT * CHUNK_WIDTH_TILES)
#define WORLD_HEIGHT_TILES 32u
#define WORLD_WIDTH_PIXELS (WORLD_WIDTH_TILES * TILE_SIZE)
#define WORLD_HEIGHT_PIXELS (WORLD_HEIGHT_TILES * TILE_SIZE)
#define WORLD_ACTIVE_CHUNK_COUNT 5u
#define WORLD_ACTIVE_WIDTH_TILES (WORLD_ACTIVE_CHUNK_COUNT * CHUNK_WIDTH_TILES)
#define WORLD_CHANGE_COUNT 128u
#define WORLD_DEFAULT_SEED 0x2D5Au

typedef struct ChangedTile {
    uint16_t tx;
    uint8_t ty;
    uint8_t tile;
} ChangedTile;

typedef enum Biome {
    BIOME_MEADOW = 0u,
    BIOME_ROOTWOOD_GROVE,
    BIOME_STONE_BELT,
    BIOME_CELESTIAL_RUINS
} Biome;

void world_init(void);
void world_set_seed(uint16_t seed);
uint16_t world_seed(void);
bool world_lighting_dirty(void);
void world_clear_lighting_dirty(void);
void world_clear_changes(void);
uint8_t world_changed_count(void);
bool world_get_changed_tile(uint8_t index, ChangedTile *change);
bool world_restore_changed_tile(const ChangedTile *change);
void world_apply_changes(void);
uint8_t world_get_tile_or_empty(uint16_t tx, uint8_t ty);
uint8_t world_get_tile_for_collision(uint16_t tx, uint8_t ty);
void world_set_tile(uint16_t tx, uint8_t ty, uint8_t tile);
uint8_t world_stream_to_chunk(uint8_t start_chunk);
bool world_take_dirty_tile(uint16_t *tx, uint8_t *ty);
bool world_is_solid_tile(uint8_t tile);
bool world_is_door_tile(uint8_t tile);
bool world_is_solid_at_pixel(int16_t x, int16_t y);
uint8_t world_mine_at_pixel(int16_t x, int16_t y);
bool world_place_at_pixel(int16_t x, int16_t y, uint8_t tile);
bool world_can_place_tile(uint16_t tx, uint8_t ty, uint8_t tile);
bool world_place_tile(uint16_t tx, uint8_t ty, uint8_t tile);
bool world_place_door(uint16_t tx, uint8_t bottom_ty);
bool world_toggle_door(uint16_t tx, uint8_t ty);
bool world_remove_multitile_at(uint16_t tx, uint8_t ty);
Biome world_biome_at_x(uint16_t x);
bool world_has_tile_near_pixel(int16_t x, int16_t y, uint8_t tile, uint8_t radius);

#endif
