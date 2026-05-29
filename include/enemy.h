#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>
#include "camera.h"
#include "player.h"

#define ENEMY_MAX_ACTIVE 3u
#define ENEMY_WIDTH 8u
#define ENEMY_HEIGHT 8u

typedef struct Enemy {
    int16_t x;
    int16_t y;
    int8_t vx;
    int8_t vy;
    uint8_t active;
    uint8_t grounded;
} Enemy;

void enemies_init(Enemy *enemies);
void enemies_update(Enemy *enemies, const Camera *camera, Player *player);

#endif
