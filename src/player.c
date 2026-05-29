#include <gb/gb.h>
#include "audio.h"
#include "item_drop.h"
#include "mining.h"
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
#define PLAYER_FALL_DAMAGE_DISTANCE 40u
#define PLAYER_INVULN_TIME 45u
#define PLAYER_RESPAWN_INVULN_TIME 90u
#define PLAYER_ACTION_ERROR_TIME 18u

static MiningState mining_state;

static uint8_t player_point_blocking_at(int16_t x,
                                        int16_t y,
                                        int16_t prev_bottom,
                                        int8_t vertical_step,
                                        uint8_t dropping_down,
                                        uint8_t is_bottom_point)
{
    uint8_t tile;
    uint8_t ty;
    int16_t tile_top;

    if (x < 0 || y < 0) {
        return 1u;
    }

    tile = world_get_tile_for_collision((uint16_t)(x >> 3), (uint8_t)(y >> 3));

    if (tile == TILE_PLATFORM) {
        if (!is_bottom_point || vertical_step <= 0 || dropping_down) {
            return 0u;
        }

        ty = (uint8_t)(y >> 3);
        tile_top = (int16_t)(ty << 3);
        return prev_bottom <= tile_top && y >= tile_top;
    }

    return world_is_solid_tile(tile);
}

static uint8_t player_aabb_blocking_at(int16_t x,
                                       int16_t y,
                                       int16_t prev_y,
                                       int8_t vertical_step,
                                       uint8_t dropping_down)
{
    int16_t hitbox_x = (int16_t)(x + PLAYER_HITBOX_X_OFFSET);
    int16_t hitbox_y = (int16_t)(y + PLAYER_HITBOX_Y_OFFSET);
    int16_t hitbox_right = (int16_t)(hitbox_x + PLAYER_HITBOX_WIDTH - 1);
    int16_t hitbox_bottom = (int16_t)(hitbox_y + PLAYER_HITBOX_HEIGHT - 1);
    int16_t prev_bottom = (int16_t)(prev_y + PLAYER_HITBOX_Y_OFFSET + PLAYER_HITBOX_HEIGHT - 1);

    return player_point_blocking_at(hitbox_x, hitbox_y, prev_bottom, vertical_step, dropping_down, 0u) ||
           player_point_blocking_at(hitbox_right, hitbox_y, prev_bottom, vertical_step, dropping_down, 0u) ||
           player_point_blocking_at(hitbox_x, hitbox_bottom, prev_bottom, vertical_step, dropping_down, 1u) ||
           player_point_blocking_at(hitbox_right, hitbox_bottom, prev_bottom, vertical_step, dropping_down, 1u);
}

static uint8_t has_ground_below(int16_t x, int16_t y)
{
    return player_aabb_blocking_at(x, (int16_t)(y + 1), y, 1, 0u);
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

        if (!player_aabb_blocking_at(next_x, next_y, player->y, 0, 0u) &&
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

        if (player_aabb_blocking_at((int16_t)(player->x + step), player->y, player->y, 0, 0u)) {
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

static void move_v(Player *player, uint8_t dropping_down)
{
    int8_t remaining = (int8_t)player->vy;
    int8_t step;

    player->grounded = 0u;

    while (remaining != 0) {
        step = remaining > 0 ? 1 : -1;

        if (player_aabb_blocking_at(player->x,
                                    (int16_t)(player->y + step),
                                    player->y,
                                    step,
                                    dropping_down)) {
            if (step > 0) {
                player->grounded = 1u;

                if (player->fall_distance >= PLAYER_FALL_DAMAGE_DISTANCE) {
                    player_damage(player, 0);
                }

                player->fall_distance = 0u;
            }

            player->vy = 0;
            return;
        }

        player->y = (int16_t)(player->y + step);

        if (step > 0 && player->fall_distance < 255u) {
            ++player->fall_distance;
        }

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

bool player_overlaps_aabb(const Player *player, int16_t x, int16_t y, uint8_t w, uint8_t h)
{
    int16_t player_x = (int16_t)(player->x + PLAYER_PLACEMENT_X_OFFSET);
    int16_t player_y = (int16_t)(player->y + PLAYER_PLACEMENT_Y_OFFSET);

    return player_x < (int16_t)(x + w) &&
           (int16_t)(player_x + PLAYER_PLACEMENT_WIDTH) > x &&
           player_y < (int16_t)(y + h) &&
           (int16_t)(player_y + PLAYER_PLACEMENT_HEIGHT) > y;
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

static void update_aim_state(Player *player,
                             const InputState *input,
                             const Inventory *inventory,
                             const ItemDrop *drops)
{
    uint8_t target_tile;
    uint8_t target_item;
    uint8_t selected_tile;

    aim_tile(player, input, &player->aim_tx, &player->aim_ty);
    player->aim_flags = 0u;

    if (player->aim_tx >= WORLD_WIDTH_TILES || player->aim_ty >= WORLD_HEIGHT_TILES) {
        return;
    }

    player->aim_flags = PLAYER_AIM_VISIBLE;
    target_tile = world_get_tile_or_empty(player->aim_tx, player->aim_ty);
    target_item = inventory_item_for_tile(target_tile);

    if (target_tile != TILE_EMPTY &&
        inventory_can_mine_tile(inventory, target_tile) &&
        (target_item == ITEM_NONE || item_drops_has_free_slot(drops))) {
        player->aim_flags |= PLAYER_AIM_ACTIONABLE;
    }

    selected_tile = inventory_selected_tile(inventory);

    if (inventory->counts[inventory->selected_slot] > 0u &&
        !overlaps_player(player, player->aim_tx, player->aim_ty) &&
        world_can_place_tile(player->aim_tx, player->aim_ty, selected_tile)) {
        player->aim_flags |= PLAYER_AIM_ACTIONABLE;
    }
}

static void use_placement(Player *player, const InputState *input, Inventory *inventory)
{
    uint8_t tile;
    uint8_t target_tile;

    if (!(input->pressed & J_B) || (input->current & J_A)) {
        return;
    }

    target_tile = world_get_tile_or_empty(player->aim_tx, player->aim_ty);

    if (world_is_door_tile(target_tile)) {
        if (world_toggle_door(player->aim_tx, player->aim_ty)) {
            player->action_error_timer = 0u;
            audio_play_place();
        } else {
            player->action_error_timer = PLAYER_ACTION_ERROR_TIME;
            audio_play_error();
        }

        return;
    }

    tile = inventory_selected_tile(inventory);

    if (inventory->counts[inventory->selected_slot] > 0u &&
        !overlaps_player(player, player->aim_tx, player->aim_ty) &&
        world_place_tile(player->aim_tx, player->aim_ty, tile)) {
        inventory_consume_selected(inventory);
        player->action_error_timer = 0u;
        audio_play_place();
    } else {
        player->action_error_timer = PLAYER_ACTION_ERROR_TIME;
        audio_play_error();
    }
}

void player_init(Player *player)
{
    player->aim_tx = 0u;
    player->aim_ty = 0u;
    player->aim_flags = 0u;
    player->hp = PLAYER_MAX_HP;
    player->invuln_timer = 0u;
    player->action_error_timer = 0u;
    player->fall_distance = 0u;
    player->facing = 1u;
    mining_init(&mining_state);
    player_respawn(player);
    player->invuln_timer = 0u;
}

void player_respawn(Player *player)
{
    player->x = PLAYER_SPAWN_X;
    player->y = PLAYER_SPAWN_Y;
    player->vx = 0;
    player->vy = 0;
    player->fall_distance = 0u;
    player->grounded = 0u;
}

void player_damage(Player *player, int8_t knockback)
{
    if (player->invuln_timer != 0u) {
        return;
    }

    if (player->hp > 0u) {
        --player->hp;
    }

    player->invuln_timer = PLAYER_INVULN_TIME;
    player->fall_distance = 0u;
    player->vx = knockback;
    player->vy = JUMP_SPEED / 2;

    if (player->hp == 0u) {
        player_respawn(player);
        player->hp = PLAYER_MAX_HP;
        player->invuln_timer = PLAYER_RESPAWN_INVULN_TIME;
    }
}

void player_update(Player *player, const InputState *input, Inventory *inventory, ItemDrop *drops)
{
    uint8_t using_tool = (uint8_t)(input->current & (J_A | J_B));

    if (player->invuln_timer != 0u) {
        --player->invuln_timer;
    } else {
        player->vx = 0;
    }

    if (player->action_error_timer != 0u) {
        --player->action_error_timer;
    }

    if (input->pressed & J_START) {
        inventory_next_slot(inventory);
    }

    if (player->invuln_timer == 0u && (input->current & J_LEFT)) {
        player->vx = -WALK_SPEED;
        player->facing = 0u;
    } else if (player->invuln_timer == 0u && (input->current & J_RIGHT)) {
        player->vx = WALK_SPEED;
        player->facing = 1u;
    }

    if (player->invuln_timer == 0u &&
        (input->pressed & J_UP) && player->grounded && !using_tool) {
        player->vy = JUMP_SPEED;
        player->grounded = 0u;
        player->fall_distance = 0u;
        audio_play_jump();
    }

    if (player->vy < MAX_FALL_SPEED) {
        player->vy = (int16_t)(player->vy + GRAVITY);
    }

    move_h(player);
    move_v(player, (uint8_t)(input->current & J_DOWN));

    if (player->invuln_timer != 0u && player->vx != 0) {
        player->vx = player->vx > 0 ? (int16_t)(player->vx - 1) : (int16_t)(player->vx + 1);
    }

    update_aim_state(player, input, inventory, drops);
    mining_update(&mining_state, player, input, inventory, drops);
    use_placement(player, input, inventory);
}
