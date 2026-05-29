#ifndef MINING_H
#define MINING_H

#include <stdint.h>
#include "input.h"
#include "inventory.h"
#include "item_drop.h"
#include "player.h"

typedef struct MiningState {
    uint8_t active;
    uint16_t tx;
    uint8_t ty;
    uint8_t tile;
    uint8_t progress;
    uint8_t required;
} MiningState;

void mining_init(MiningState *mining);
void mining_update(MiningState *mining,
                   Player *player,
                   const InputState *input,
                   Inventory *inventory,
                   ItemDrop *drops);

#endif
