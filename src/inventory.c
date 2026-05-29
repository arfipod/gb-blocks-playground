#include "inventory.h"
#include "tile_defs.h"

static const Recipe recipes[RECIPE_COUNT] = {
    { ITEM_PLANK, 4u, TOOL_LEVEL_NONE, ITEM_WOOD, 1u, ITEM_NONE, 0u, 0u },
    { ITEM_WORKBENCH, 1u, TOOL_LEVEL_NONE, ITEM_PLANK, 4u, ITEM_NONE, 0u, 0u },
    { ITEM_NONE, 0u, TOOL_LEVEL_STONE, ITEM_WOOD, 1u, ITEM_STONE, 1u, 0u },
    { ITEM_TORCH, 4u, TOOL_LEVEL_NONE, ITEM_COAL, 1u, ITEM_WOOD, 1u, 0u },
    { ITEM_TORCH, 2u, TOOL_LEVEL_NONE, ITEM_STONE, 1u, ITEM_WOOD, 1u, 0u },
    { ITEM_PLATFORM, 2u, TOOL_LEVEL_NONE, ITEM_PLANK, 1u, ITEM_NONE, 0u, 0u },
    { ITEM_DOOR, 1u, TOOL_LEVEL_NONE, ITEM_PLANK, 3u, ITEM_NONE, 0u, 0u },
    { ITEM_NONE, 0u, TOOL_LEVEL_IRON, ITEM_IRON, 3u, ITEM_NONE, 0u, 1u }
};

void inventory_init(Inventory *inventory)
{
    uint8_t i;

    for (i = 0u; i < INVENTORY_ITEM_COUNT; ++i) {
        inventory->counts[i] = 0u;
    }

    inventory->tool_level = TOOL_LEVEL_NONE;
    inventory->selected_slot = ITEM_DIRT;
    inventory->selected_recipe = 0u;

    inventory_add_item(inventory, ITEM_DIRT, 8u);
    inventory_add_item(inventory, ITEM_WOOD, 3u);
    inventory_add_item(inventory, ITEM_STONE, 2u);
}

bool inventory_can_add_item(const Inventory *inventory, uint8_t item, uint8_t count)
{
    if (item >= INVENTORY_ITEM_COUNT || count == 0u || count > INVENTORY_MAX_COUNT) {
        return false;
    }

    return inventory->counts[item] <= (uint8_t)(INVENTORY_MAX_COUNT - count);
}

void inventory_add_item(Inventory *inventory, uint8_t item, uint8_t count)
{
    uint16_t next_count;

    if (item >= INVENTORY_ITEM_COUNT || count == 0u) {
        return;
    }

    next_count = (uint16_t)(inventory->counts[item] + count);

    if (next_count > INVENTORY_MAX_COUNT) {
        inventory->counts[item] = INVENTORY_MAX_COUNT;
    } else {
        inventory->counts[item] = (uint8_t)next_count;
    }
}

void inventory_add_block(Inventory *inventory, uint8_t tile)
{
    uint8_t item = inventory_item_for_tile(tile);

    if (item != ITEM_NONE) {
        inventory_add_item(inventory, item, 1u);
    }
}

bool inventory_consume_item(Inventory *inventory, uint8_t item, uint8_t count)
{
    if (item >= INVENTORY_ITEM_COUNT || inventory->counts[item] < count) {
        return false;
    }

    inventory->counts[item] = (uint8_t)(inventory->counts[item] - count);
    return true;
}

bool inventory_consume_selected(Inventory *inventory)
{
    return inventory_consume_item(inventory, inventory->selected_slot, 1u);
}

void inventory_next_slot(Inventory *inventory)
{
    inventory->selected_slot = (uint8_t)((inventory->selected_slot + 1u) % INVENTORY_SLOT_COUNT);
}

void inventory_prev_slot(Inventory *inventory)
{
    if (inventory->selected_slot == 0u) {
        inventory->selected_slot = (uint8_t)(INVENTORY_SLOT_COUNT - 1u);
    } else {
        --inventory->selected_slot;
    }
}

uint8_t inventory_selected_tile(const Inventory *inventory)
{
    return inventory_item_tile(inventory->selected_slot);
}

uint8_t inventory_item_tile(uint8_t item)
{
    switch (item) {
    case ITEM_DIRT:
        return TILE_DIRT;
    case ITEM_STONE:
        return TILE_STONE;
    case ITEM_WOOD:
        return TILE_WOOD;
    case ITEM_PLANK:
        return TILE_PLANK;
    case ITEM_WORKBENCH:
        return TILE_WORKBENCH;
    case ITEM_TORCH:
        return TILE_TORCH;
    case ITEM_PLATFORM:
        return TILE_PLATFORM;
    case ITEM_DOOR:
        return TILE_DOOR_CLOSED_BOTTOM;
    case ITEM_COAL:
        return TILE_COAL;
    case ITEM_COPPER:
        return TILE_COPPER;
    case ITEM_IRON:
        return TILE_IRON;
    default:
        return TILE_EMPTY;
    }
}

uint8_t inventory_item_for_tile(uint8_t tile)
{
    switch (tile) {
    case TILE_DIRT:
    case TILE_GRASS:
        return ITEM_DIRT;
    case TILE_STONE:
        return ITEM_STONE;
    case TILE_WOOD:
    case TILE_LEAVES:
        return ITEM_WOOD;
    case TILE_PLANK:
        return ITEM_PLANK;
    case TILE_WORKBENCH:
        return ITEM_WORKBENCH;
    case TILE_TORCH:
        return ITEM_TORCH;
    case TILE_PLATFORM:
        return ITEM_PLATFORM;
    case TILE_DOOR_CLOSED_TOP:
    case TILE_DOOR_CLOSED_BOTTOM:
    case TILE_DOOR_OPEN_TOP:
    case TILE_DOOR_OPEN_BOTTOM:
        return ITEM_DOOR;
    case TILE_COAL:
        return ITEM_COAL;
    case TILE_COPPER:
        return ITEM_COPPER;
    case TILE_IRON:
        return ITEM_IRON;
    default:
        return ITEM_NONE;
    }
}

uint8_t inventory_tool_tile(uint8_t tool_level)
{
    if (tool_level >= TOOL_LEVEL_IRON) {
        return TILE_UI_PICKAXE_PLUS;
    }

    return TILE_UI_PICKAXE;
}

bool inventory_can_mine_tile(const Inventory *inventory, uint8_t tile)
{
    return inventory->tool_level >= inventory_tile_required_tool_level(tile);
}

uint8_t inventory_tile_required_tool_level(uint8_t tile)
{
    switch (tile) {
    case TILE_STONE:
    case TILE_COAL:
    case TILE_COPPER:
    case TILE_IRON:
        return TOOL_LEVEL_STONE;
    default:
        return TOOL_LEVEL_HAND;
    }
}

const Recipe *inventory_recipe(uint8_t recipe_index)
{
    if (recipe_index >= RECIPE_COUNT) {
        return &recipes[0u];
    }

    return &recipes[recipe_index];
}

void inventory_next_recipe(Inventory *inventory)
{
    inventory->selected_recipe = (uint8_t)((inventory->selected_recipe + 1u) % RECIPE_COUNT);
}

void inventory_prev_recipe(Inventory *inventory)
{
    if (inventory->selected_recipe == 0u) {
        inventory->selected_recipe = (uint8_t)(RECIPE_COUNT - 1u);
    } else {
        --inventory->selected_recipe;
    }
}

bool inventory_can_craft(const Inventory *inventory, uint8_t recipe_index, bool near_workbench)
{
    return inventory_craft_status(inventory, recipe_index, near_workbench) == CRAFT_STATUS_OK;
}

uint8_t inventory_craft_status(const Inventory *inventory, uint8_t recipe_index, bool near_workbench)
{
    const Recipe *recipe = inventory_recipe(recipe_index);

    if (recipe->requires_workbench && !near_workbench) {
        return CRAFT_STATUS_NEEDS_WORKBENCH;
    }

    if (recipe->output_tool_level != TOOL_LEVEL_NONE) {
        if (inventory->tool_level >= recipe->output_tool_level) {
            return CRAFT_STATUS_ALREADY_OWNED;
        }
    } else if (!inventory_can_add_item(inventory, recipe->output_item, recipe->output_count)) {
        return CRAFT_STATUS_OUTPUT_BLOCKED;
    }

    if (inventory->counts[recipe->input_a] < recipe->input_a_count) {
        return CRAFT_STATUS_NEEDS_MATERIAL;
    }

    if (recipe->input_b != ITEM_NONE &&
        inventory->counts[recipe->input_b] < recipe->input_b_count) {
        return CRAFT_STATUS_NEEDS_MATERIAL;
    }

    return CRAFT_STATUS_OK;
}

bool inventory_recipe_input_missing(const Inventory *inventory, uint8_t recipe_index, uint8_t input_index)
{
    const Recipe *recipe = inventory_recipe(recipe_index);
    uint8_t item = input_index == 0u ? recipe->input_a : recipe->input_b;
    uint8_t count = input_index == 0u ? recipe->input_a_count : recipe->input_b_count;

    if (item == ITEM_NONE) {
        return false;
    }

    return inventory->counts[item] < count;
}

bool inventory_craft_selected(Inventory *inventory, bool near_workbench)
{
    const Recipe *recipe = inventory_recipe(inventory->selected_recipe);

    if (!inventory_can_craft(inventory, inventory->selected_recipe, near_workbench)) {
        return false;
    }

    inventory_consume_item(inventory, recipe->input_a, recipe->input_a_count);

    if (recipe->input_b != ITEM_NONE) {
        inventory_consume_item(inventory, recipe->input_b, recipe->input_b_count);
    }

    if (recipe->output_tool_level != TOOL_LEVEL_NONE) {
        inventory->tool_level = recipe->output_tool_level;
    } else {
        inventory_add_item(inventory, recipe->output_item, recipe->output_count);

        if (recipe->output_item < INVENTORY_SLOT_COUNT) {
            inventory->selected_slot = recipe->output_item;
        }
    }

    return true;
}
