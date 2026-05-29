#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "chunk.h"
#include "tile_defs.h"

#define WORLD_WIDTH_TILES (WORLD_CHUNK_COUNT * CHUNK_WIDTH_TILES)
#define WORLD_HEIGHT_TILES 18u
#define WORLD_WIDTH_PIXELS (WORLD_WIDTH_TILES * TILE_SIZE)
#define WORLD_ACTIVE_CHUNK_COUNT 6u
#define WORLD_ACTIVE_WIDTH_TILES (WORLD_ACTIVE_CHUNK_COUNT * CHUNK_WIDTH_TILES)

void world_init(void);
uint8_t world_get_tile_or_empty(uint16_t tx, uint8_t ty);
uint8_t world_get_tile_for_collision(uint16_t tx, uint8_t ty);
void world_set_tile(uint16_t tx, uint8_t ty, uint8_t tile);
uint8_t world_stream_to_chunk(uint8_t start_chunk);
bool world_take_dirty_tile(uint16_t *tx, uint8_t *ty);
bool world_is_solid_tile(uint8_t tile);
bool world_is_solid_at_pixel(int16_t x, int16_t y);
uint8_t world_mine_at_pixel(int16_t x, int16_t y);
bool world_place_at_pixel(int16_t x, int16_t y, uint8_t tile);
bool world_place_tile(uint16_t tx, uint8_t ty, uint8_t tile);
bool world_has_tile_near_pixel(int16_t x, int16_t y, uint8_t tile, uint8_t radius);

#endif
