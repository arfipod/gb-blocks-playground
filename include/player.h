#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include "input.h"

#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 16

typedef struct Player {
    int16_t x;
    int16_t y;
    int16_t vx;
    int16_t vy;
    uint8_t facing;
    uint8_t grounded;
} Player;

void player_init(Player *player);
void player_update(Player *player, const InputState *input);

#endif
