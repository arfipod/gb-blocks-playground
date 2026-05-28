#include <gb/gb.h>
#include "collision.h"
#include "player.h"
#include "tile_defs.h"
#include "world.h"

#define WALK_SPEED 1
#define GRAVITY 1
#define MAX_FALL_SPEED 3

static void move_h(Player *player)
{
    int16_t next_x = (int16_t)(player->x + player->vx);

    if (!collision_aabb_solid(next_x, player->y, PLAYER_WIDTH, PLAYER_HEIGHT)) {
        player->x = next_x;
    }
}

static void move_v(Player *player)
{
    int16_t next_y = (int16_t)(player->y + player->vy);

    player->grounded = 0u;

    if (!collision_aabb_solid(player->x, next_y, PLAYER_WIDTH, PLAYER_HEIGHT)) {
        player->y = next_y;
        return;
    }

    if (player->vy > 0) {
        player->grounded = 1u;
        player->y = (int16_t)(((player->y + PLAYER_HEIGHT) & 0xF8) - PLAYER_HEIGHT);
    }

    player->vy = 0;
}

static void use_tool(Player *player, const InputState *input)
{
    int16_t target_x = player->x;
    int16_t target_y = (int16_t)(player->y + (PLAYER_HEIGHT / 2));

    if (player->facing == 0u) {
        target_x = (int16_t)(target_x - TILE_SIZE);
    } else {
        target_x = (int16_t)(target_x + TILE_SIZE);
    }

    if (input->pressed & J_A) {
        world_mine_at_pixel(target_x, target_y);
    }

    if (input->pressed & J_B) {
        world_place_at_pixel(target_x, target_y, TILE_DIRT);
    }

    /* TODO: route mining and placing through inventory and tool strength. */
}

void player_init(Player *player)
{
    player->x = 32;
    player->y = 64;
    player->vx = 0;
    player->vy = 0;
    player->facing = 1u;
    player->grounded = 0u;
}

void player_update(Player *player, const InputState *input)
{
    player->vx = 0;

    if (input->current & J_LEFT) {
        player->vx = -WALK_SPEED;
        player->facing = 0u;
    } else if (input->current & J_RIGHT) {
        player->vx = WALK_SPEED;
        player->facing = 1u;
    }

    if (player->vy < MAX_FALL_SPEED) {
        player->vy = (int16_t)(player->vy + GRAVITY);
    }

    move_h(player);
    move_v(player);
    use_tool(player, input);
}
