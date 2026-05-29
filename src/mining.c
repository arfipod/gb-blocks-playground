#include <gb/gb.h>
#include "audio.h"
#include "mining.h"
#include "tile_defs.h"
#include "world.h"

#define MINING_ACTION_ERROR_TIME 18u
#define MINING_MIN_REQUIRED_TIME 4u

static void mining_reset(MiningState *mining)
{
    mining->active = 0u;
    mining->progress = 0u;
}

static uint8_t tile_base_mining_time(uint8_t tile)
{
    switch (tile) {
    case TILE_TORCH:
        return 4u;
    case TILE_DIRT:
    case TILE_GRASS:
        return 12u;
    case TILE_LEAVES:
        return 10u;
    case TILE_WOOD:
    case TILE_PLANK:
    case TILE_PLATFORM:
        return 18u;
    case TILE_DOOR_CLOSED_TOP:
    case TILE_DOOR_CLOSED_BOTTOM:
    case TILE_DOOR_OPEN_TOP:
    case TILE_DOOR_OPEN_BOTTOM:
        return 24u;
    case TILE_WORKBENCH:
        return 24u;
    case TILE_STONE:
        return 36u;
    case TILE_COAL:
    case TILE_COPPER:
    case TILE_IRON:
        return 48u;
    default:
        return 12u;
    }
}

static uint8_t mining_required_time(uint8_t tile, uint8_t tool_level)
{
    uint8_t required = tile_base_mining_time(tile);
    uint8_t required_tool_level = inventory_tile_required_tool_level(tile);

    if (tool_level > required_tool_level) {
        uint8_t reduction = (uint8_t)((tool_level - required_tool_level) * 8u);

        if (required > (uint8_t)(reduction + MINING_MIN_REQUIRED_TIME)) {
            required = (uint8_t)(required - reduction);
        } else {
            required = MINING_MIN_REQUIRED_TIME;
        }
    }

    return required;
}

static void mining_apply_progress_flags(const MiningState *mining, Player *player)
{
    uint16_t scaled_progress;

    player->aim_flags &= (uint8_t)~(PLAYER_AIM_PROGRESS_1 | PLAYER_AIM_PROGRESS_2);

    if (!mining->active || mining->required == 0u) {
        return;
    }

    scaled_progress = (uint16_t)mining->progress * 3u;

    if (scaled_progress >= (uint16_t)mining->required * 2u) {
        player->aim_flags |= PLAYER_AIM_PROGRESS_2;
    } else if (scaled_progress >= mining->required) {
        player->aim_flags |= PLAYER_AIM_PROGRESS_1;
    }
}

void mining_init(MiningState *mining)
{
    mining->active = 0u;
    mining->tx = 0u;
    mining->ty = 0u;
    mining->tile = TILE_EMPTY;
    mining->progress = 0u;
    mining->required = 0u;
}

void mining_update(MiningState *mining,
                   Player *player,
                   const InputState *input,
                   Inventory *inventory,
                   ItemDrop *drops)
{
    uint8_t tile;
    uint8_t item;
    uint8_t required;

    player->aim_flags &= (uint8_t)~(PLAYER_AIM_PROGRESS_1 | PLAYER_AIM_PROGRESS_2);

    if (!(input->current & J_A)) {
        mining_reset(mining);
        return;
    }

    if (player->aim_tx >= WORLD_WIDTH_TILES || player->aim_ty >= WORLD_HEIGHT_TILES) {
        mining_reset(mining);
        return;
    }

    tile = world_get_tile_or_empty(player->aim_tx, player->aim_ty);
    item = inventory_item_for_tile(tile);

    if (tile == TILE_EMPTY) {
        mining_reset(mining);
        return;
    }

    if (!inventory_can_mine_tile(inventory, tile) ||
        (item != ITEM_NONE && !item_drops_has_free_slot(drops))) {
        mining_reset(mining);

        if (input->pressed & J_A) {
            player->action_error_timer = MINING_ACTION_ERROR_TIME;
            audio_play_error();
        }

        return;
    }

    required = mining_required_time(tile, inventory->tool_level);

    if (!mining->active ||
        mining->tx != player->aim_tx ||
        mining->ty != player->aim_ty ||
        mining->tile != tile) {
        mining->active = 1u;
        mining->tx = player->aim_tx;
        mining->ty = player->aim_ty;
        mining->tile = tile;
        mining->progress = 0u;
        mining->required = required;
    }

    if (mining->progress < 255u) {
        ++mining->progress;
    }

    mining_apply_progress_flags(mining, player);

    if (mining->progress >= mining->required) {
        item = inventory_item_for_tile(tile);

        if (item == ITEM_NONE ||
            item_drops_spawn(drops, mining->tx, mining->ty, item, 1u)) {
            world_remove_multitile_at(mining->tx, mining->ty);
            player->action_error_timer = 0u;
            audio_play_mine();
        } else {
            player->action_error_timer = MINING_ACTION_ERROR_TIME;
            audio_play_error();
        }

        mining_reset(mining);
        player->aim_flags &= (uint8_t)~(PLAYER_AIM_PROGRESS_1 | PLAYER_AIM_PROGRESS_2);
    }
}
