#ifndef TILE_DEFS_H
#define TILE_DEFS_H

#include <stdint.h>

#define TILE_EMPTY 0u
#define TILE_DIRT 1u
#define TILE_GRASS 2u
#define TILE_STONE 3u
#define TILE_WOOD 4u
#define TILE_LEAVES 5u
#define TILE_PLANK 6u
#define TILE_WORKBENCH 7u
#define TILE_TORCH 8u

#define TILE_UI_CURSOR 9u
#define TILE_UI_PANEL 10u
#define TILE_UI_ARROW 11u
#define TILE_UI_LOCK 12u
#define TILE_DIGIT_0 13u
#define TILE_DIGIT_9 22u

#define TILE_COUNT 23u
#define TILE_SIZE 8u
#define PLAYER_TILE_BASE 24u
#define PLAYER_TILE_COUNT 2u

extern const uint8_t terrain_tiles[TILE_COUNT * 16u];
extern const uint8_t player_tiles[PLAYER_TILE_COUNT * 16u];

#endif
