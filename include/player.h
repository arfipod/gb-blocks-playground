#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <stdint.h>
#include "inventory.h"
#include "input.h"

typedef struct ItemDrop ItemDrop;

#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 16
#define PLAYER_MAX_HP 5u
#define PLAYER_SPAWN_X 160
#define PLAYER_SPAWN_Y 64
#define PLAYER_AIM_VISIBLE 0x01u
#define PLAYER_AIM_ACTIONABLE 0x02u
#define PLAYER_AIM_PROGRESS_1 0x04u
#define PLAYER_AIM_PROGRESS_2 0x08u

typedef struct Player {
    int16_t x;
    int16_t y;
    int16_t vx;
    int16_t vy;
    uint16_t aim_tx;
    uint8_t aim_ty;
    uint8_t aim_flags;
    uint8_t hp;
    uint8_t invuln_timer;
    uint8_t action_error_timer;
    uint8_t fall_distance;
    uint8_t facing;
    uint8_t grounded;
} Player;

void player_init(Player *player);
void player_update(Player *player, const InputState *input, Inventory *inventory, ItemDrop *drops);
void player_respawn(Player *player);
void player_damage(Player *player, int8_t knockback);
bool player_overlaps_aabb(const Player *player, int16_t x, int16_t y, uint8_t w, uint8_t h);

#endif
