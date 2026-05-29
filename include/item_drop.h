#ifndef ITEM_DROP_H
#define ITEM_DROP_H

#include <stdbool.h>
#include <stdint.h>
#include "inventory.h"
#include "player.h"

#define ITEM_DROP_MAX_COUNT 8u

typedef struct ItemDrop {
    uint8_t active;
    uint16_t x;
    int16_t y;
    int8_t vx;
    int8_t vy;
    uint8_t item;
    uint8_t count;
    uint16_t lifetime;
} ItemDrop;

void item_drops_init(ItemDrop *drops);
bool item_drops_has_free_slot(const ItemDrop *drops);
bool item_drops_spawn(ItemDrop *drops, uint16_t tx, uint8_t ty, uint8_t item, uint8_t count);
void item_drops_update(ItemDrop *drops, const Player *player, Inventory *inventory);

#endif
