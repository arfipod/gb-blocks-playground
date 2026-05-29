#ifndef INVENTORY_H
#define INVENTORY_H

#include <stdbool.h>
#include <stdint.h>

#define INVENTORY_SLOT_COUNT 6u
#define RECIPE_COUNT 3u
#define INVENTORY_MAX_COUNT 99u

#define ITEM_DIRT 0u
#define ITEM_STONE 1u
#define ITEM_WOOD 2u
#define ITEM_PLANK 3u
#define ITEM_WORKBENCH 4u
#define ITEM_TORCH 5u
#define ITEM_NONE 255u

typedef struct Recipe {
    uint8_t output_item;
    uint8_t output_count;
    uint8_t input_a;
    uint8_t input_a_count;
    uint8_t input_b;
    uint8_t input_b_count;
    uint8_t requires_workbench;
} Recipe;

typedef struct Inventory {
    uint8_t counts[INVENTORY_SLOT_COUNT];
    uint8_t selected_slot;
    uint8_t selected_recipe;
} Inventory;

void inventory_init(Inventory *inventory);
bool inventory_can_add_item(const Inventory *inventory, uint8_t item, uint8_t count);
void inventory_add_item(Inventory *inventory, uint8_t item, uint8_t count);
void inventory_add_block(Inventory *inventory, uint8_t tile);
bool inventory_consume_item(Inventory *inventory, uint8_t item, uint8_t count);
bool inventory_consume_selected(Inventory *inventory);
void inventory_next_slot(Inventory *inventory);
void inventory_prev_slot(Inventory *inventory);
uint8_t inventory_selected_tile(const Inventory *inventory);
uint8_t inventory_item_tile(uint8_t item);
uint8_t inventory_item_for_tile(uint8_t tile);
const Recipe *inventory_recipe(uint8_t recipe_index);
void inventory_next_recipe(Inventory *inventory);
void inventory_prev_recipe(Inventory *inventory);
bool inventory_can_craft(const Inventory *inventory, uint8_t recipe_index, bool near_workbench);
bool inventory_craft_selected(Inventory *inventory, bool near_workbench);

#endif
