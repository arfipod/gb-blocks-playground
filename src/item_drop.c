#include "item_drop.h"
#include "tile_defs.h"
#include "world.h"

#define ITEM_DROP_GRAVITY 1
#define ITEM_DROP_MAX_FALL_SPEED 3
#define ITEM_DROP_LIFETIME 3600u
#define ITEM_DROP_PICKUP_SIZE 10u
#define ITEM_DROP_PICKUP_DELAY 18u

void item_drops_init(ItemDrop *drops)
{
    uint8_t i;

    for (i = 0u; i < ITEM_DROP_MAX_COUNT; ++i) {
        drops[i].active = 0u;
    }
}

bool item_drops_has_free_slot(const ItemDrop *drops)
{
    uint8_t i;

    for (i = 0u; i < ITEM_DROP_MAX_COUNT; ++i) {
        if (!drops[i].active) {
            return true;
        }
    }

    return false;
}

bool item_drops_spawn(ItemDrop *drops, uint16_t tx, uint8_t ty, uint8_t item, uint8_t count)
{
    uint8_t i;

    if (item == ITEM_NONE || count == 0u) {
        return true;
    }

    for (i = 0u; i < ITEM_DROP_MAX_COUNT; ++i) {
        if (!drops[i].active) {
            drops[i].active = 1u;
            drops[i].x = (uint16_t)(tx << 3);
            drops[i].y = (int16_t)(ty << 3);
            drops[i].vx = 0;
            drops[i].vy = 0;
            drops[i].item = item;
            drops[i].count = count;
            drops[i].pickup_delay = ITEM_DROP_PICKUP_DELAY;
            drops[i].lifetime = ITEM_DROP_LIFETIME;
            return true;
        }
    }

    return false;
}

static void item_drop_move_vertical(ItemDrop *drop)
{
    int8_t remaining;
    int8_t step;

    if (drop->vy < ITEM_DROP_MAX_FALL_SPEED) {
        ++drop->vy;
    }

    remaining = drop->vy;

    while (remaining != 0) {
        step = remaining > 0 ? 1 : -1;

        if (step > 0 &&
            (world_is_solid_at_pixel((int16_t)drop->x, (int16_t)(drop->y + TILE_SIZE)) ||
             world_is_solid_at_pixel((int16_t)(drop->x + TILE_SIZE - 1u), (int16_t)(drop->y + TILE_SIZE)))) {
            drop->vy = 0;
            return;
        }

        drop->y = (int16_t)(drop->y + step);
        remaining = (int8_t)(remaining - step);
    }
}

void item_drops_update(ItemDrop *drops, const Player *player, Inventory *inventory)
{
    uint8_t i;

    for (i = 0u; i < ITEM_DROP_MAX_COUNT; ++i) {
        ItemDrop *drop = &drops[i];

        if (!drop->active) {
            continue;
        }

        if (drop->lifetime == 0u || drop->y > WORLD_HEIGHT_PIXELS) {
            drop->active = 0u;
            continue;
        }

        --drop->lifetime;
        item_drop_move_vertical(drop);

        if (drop->pickup_delay != 0u) {
            --drop->pickup_delay;
            continue;
        }

        if (player_overlaps_aabb(player,
                                 (int16_t)drop->x,
                                 drop->y,
                                 ITEM_DROP_PICKUP_SIZE,
                                 ITEM_DROP_PICKUP_SIZE) &&
            inventory_can_add_item(inventory, drop->item, drop->count)) {
            inventory_add_item(inventory, drop->item, drop->count);
            drop->active = 0u;
        }
    }
}
