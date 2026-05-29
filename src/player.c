#include <gb/gb.h>
#include "collision.h"
#include "player.h"
#include "tile_defs.h"
#include "world.h"

#define WALK_SPEED 1
#define GRAVITY 1
#define MAX_FALL_SPEED 3
#define JUMP_SPEED -5
#define MAX_STEP_UP TILE_SIZE
#define PLAYER_HITBOX_X_OFFSET 1
#define PLAYER_HITBOX_Y_OFFSET 8
#define PLAYER_HITBOX_WIDTH 6
#define PLAYER_HITBOX_HEIGHT 8
#define PLAYER_PLACEMENT_X_OFFSET 0
#define PLAYER_PLACEMENT_Y_OFFSET 0
#define PLAYER_PLACEMENT_WIDTH PLAYER_WIDTH
#define PLAYER_PLACEMENT_HEIGHT PLAYER_HEIGHT

static uint8_t player_aabb_solid_at(int16_t x, int16_t y)
{
    return collision_aabb_solid((int16_t)(x + PLAYER_HITBOX_X_OFFSET),
                                (int16_t)(y + PLAYER_HITBOX_Y_OFFSET),
                                PLAYER_HITBOX_WIDTH,
                                PLAYER_HITBOX_HEIGHT);
}

static uint8_t has_ground_below(int16_t x, int16_t y)
{
    return player_aabb_solid_at(x, (int16_t)(y + 1));
}

static uint8_t try_step_up(Player *player, int8_t step)
{
    uint8_t ascent;
    int16_t next_x = (int16_t)(player->x + step);

    if (!player->grounded || player->vy < 0) {
        return 0u;
    }

    for (ascent = 1u; ascent <= MAX_STEP_UP; ++ascent) {
        int16_t next_y = (int16_t)(player->y - ascent);

        if (!player_aabb_solid_at(next_x, next_y) &&
            has_ground_below(next_x, next_y)) {
            player->x = next_x;
            player->y = next_y;
            player->vy = 0;
            return 1u;
        }
    }

    return 0u;
}

static void move_h(Player *player)
{
    int8_t remaining = (int8_t)player->vx;
    int8_t step;

    while (remaining != 0) {
        step = remaining > 0 ? 1 : -1;

        if (player_aabb_solid_at((int16_t)(player->x + step), player->y)) {
            if (try_step_up(player, step)) {
                remaining = (int8_t)(remaining - step);
                continue;
            }

            player->vx = 0;
            return;
        }

        player->x = (int16_t)(player->x + step);
        remaining = (int8_t)(remaining - step);
    }
}

static void move_v(Player *player)
{
    int8_t remaining = (int8_t)player->vy;
    int8_t step;

    player->grounded = 0u;

    while (remaining != 0) {
        step = remaining > 0 ? 1 : -1;

        if (player_aabb_solid_at(player->x, (int16_t)(player->y + step))) {
            if (step > 0) {
                player->grounded = 1u;
            }

            player->vy = 0;
            return;
        }

        player->y = (int16_t)(player->y + step);
        remaining = (int8_t)(remaining - step);
    }
}

static uint8_t overlaps_player(const Player *player, uint16_t tx, uint8_t ty)
{
    int16_t tile_x = (int16_t)(tx << 3);
    int16_t tile_y = (int16_t)(ty << 3);
    int16_t player_x = (int16_t)(player->x + PLAYER_PLACEMENT_X_OFFSET);
    int16_t player_y = (int16_t)(player->y + PLAYER_PLACEMENT_Y_OFFSET);

    return player_x < (int16_t)(tile_x + TILE_SIZE) &&
           (int16_t)(player_x + PLAYER_PLACEMENT_WIDTH) > tile_x &&
           player_y < (int16_t)(tile_y + TILE_SIZE) &&
           (int16_t)(player_y + PLAYER_PLACEMENT_HEIGHT) > tile_y;
}

static void aim_tile(const Player *player, const InputState *input, uint16_t *tx, uint8_t *ty)
{
    int16_t hitbox_left = (int16_t)(player->x + PLAYER_HITBOX_X_OFFSET);
    int16_t hitbox_right = (int16_t)(hitbox_left + PLAYER_HITBOX_WIDTH - 1);
    uint16_t left_tile;
    uint16_t right_tile;
    int16_t hitbox_top = (int16_t)(player->y + PLAYER_HITBOX_Y_OFFSET);
    int16_t target_y = (int16_t)(hitbox_top + (PLAYER_HITBOX_HEIGHT / 2));

    if (hitbox_left < 0) {
        left_tile = 0u;
    } else {
        left_tile = (uint16_t)(hitbox_left >> 3);
    }

    if (hitbox_right < 0) {
        right_tile = 0u;
    } else {
        right_tile = (uint16_t)(hitbox_right >> 3);
    }

    if (player->facing == 0u) {
        *tx = left_tile > 0u ? (uint16_t)(left_tile - 1u) : 0u;
    } else {
        *tx = (uint16_t)(right_tile + 1u);
    }

    if (input->current & J_UP) {
        target_y = (int16_t)(hitbox_top - 1);
    } else if (input->current & J_DOWN) {
        target_y = (int16_t)(hitbox_top + PLAYER_HITBOX_HEIGHT);
    }

    if (target_y < 0) {
        *ty = 0u;
    } else {
        *ty = (uint8_t)(target_y >> 3);
    }
}

static void use_tool(Player *player, const InputState *input, Inventory *inventory)
{
    uint16_t target_tx;
    uint8_t target_ty;
    uint8_t tile;
    uint8_t item;

    aim_tile(player, input, &target_tx, &target_ty);

    if (input->pressed & J_A) {
        tile = world_get_tile_or_empty(target_tx, target_ty);
        item = inventory_item_for_tile(tile);

        if (item == ITEM_NONE || inventory_can_add_item(inventory, item, 1u)) {
            tile = world_mine_at_pixel((int16_t)(target_tx << 3), (int16_t)(target_ty << 3));
            inventory_add_block(inventory, tile);
        }
    }

    else if (input->pressed & J_B) {
        tile = inventory_selected_tile(inventory);

        if (inventory->counts[inventory->selected_slot] > 0u) {
            if (!overlaps_player(player, target_tx, target_ty) &&
                world_place_tile(target_tx, target_ty, tile)) {
                inventory_consume_selected(inventory);
            }
        }
    }
}

void player_init(Player *player)
{
    player->x = 160;
    player->y = 64;
    player->vx = 0;
    player->vy = 0;
    player->facing = 1u;
    player->grounded = 0u;
}

void player_update(Player *player, const InputState *input, Inventory *inventory)
{
    uint8_t using_tool = (uint8_t)(input->current & (J_A | J_B));

    player->vx = 0;

    if (input->pressed & J_START) {
        inventory_next_slot(inventory);
    }

    if (input->current & J_LEFT) {
        player->vx = -WALK_SPEED;
        player->facing = 0u;
    } else if (input->current & J_RIGHT) {
        player->vx = WALK_SPEED;
        player->facing = 1u;
    }

    if ((input->pressed & J_UP) && player->grounded && !using_tool) {
        player->vy = JUMP_SPEED;
        player->grounded = 0u;
    }

    if (player->vy < MAX_FALL_SPEED) {
        player->vy = (int16_t)(player->vy + GRAVITY);
    }

    move_h(player);
    move_v(player);
    use_tool(player, input, inventory);
}
