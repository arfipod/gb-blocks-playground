#ifndef TILE_DEFS_H
#define TILE_DEFS_H

#include <stdint.h>

#define TILE_EMPTY 0u
#define TILE_DIRT 1u
#define TILE_GRASS 2u
#define TILE_STONE 3u

#define TILE_COUNT 4u
#define TILE_SIZE 8u
#define PLAYER_TILE_COUNT 2u

extern const uint8_t terrain_tiles[TILE_COUNT * 16u];
extern const uint8_t player_tiles[PLAYER_TILE_COUNT * 16u];

#endif
