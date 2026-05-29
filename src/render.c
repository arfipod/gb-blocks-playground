#include <gb/gb.h>
#include <stdbool.h>
#include <stdint.h>
#include "render.h"
#include "tile_defs.h"
#include "world.h"

#define BG_BUFFER_WIDTH 32u
#define WIN_WIDTH 20u
#define WIN_HEIGHT 18u
#define HUD_HEIGHT 3u
#define HUD_Y 120u
#define PLAYER_SPRITE 0u
#define CURSOR_SPRITE 1u
#define ENEMY_FIRST_SPRITE 2u
#define DROP_FIRST_SPRITE (ENEMY_FIRST_SPRITE + ENEMY_MAX_ACTIVE)
#define SPRITE_HIDE_X 0u
#define SPRITE_HIDE_Y 0u
#define RENDER_DIRTY_TILE_BUDGET 12u

static uint8_t bg_buffer[WORLD_HEIGHT_TILES];
static uint8_t win_buffer[WIN_WIDTH * WIN_HEIGHT];
static uint8_t tile_variant_buffer[16u];
static uint16_t rendered_origin_tile;
static uint8_t world_rendered;

static void init_light_tiles(void);
static uint8_t render_tile_for_world_pos(uint16_t tx, uint8_t ty);
static void render_world_column(uint16_t world_x);

const uint8_t terrain_tiles[TILE_COUNT * 16u] = {
    // 0: empty
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    // 1: dirt
    0x00,0x00,0x24,0x24,0x00,0x00,0x42,0x42,0x18,0x18,0x00,0x00,0x24,0x24,0x00,0x00,

    // 2: grass
    0xFF,0x00,0x00,0xFF,0xAA,0x55,0x00,0xFF,0x24,0x24,0x00,0x00,0x42,0x42,0x18,0x18,

    // 3: stone
    0x81,0x7E,0x42,0xBD,0x24,0xDB,0x18,0xE7,0x18,0xE7,0x24,0xDB,0x42,0xBD,0x81,0x7E

    // 4: wood
    ,0x66,0x18,0x42,0x3C,0x66,0x18,0x42,0x3C,0x66,0x18,0x42,0x3C,0x66,0x18,0x42,0x3C,

    // 5: leaves
    0x24,0x5A,0x7E,0x81,0xDB,0x24,0xFF,0x00,0x7E,0x81,0xDB,0x24,0x7E,0x81,0x24,0x5A,

    // 6: planks
    0xFF,0x00,0x00,0xFF,0x7E,0x00,0x00,0x7E,0xFF,0x00,0x00,0xFF,0x7E,0x00,0x00,0x7E,

    // 7: workbench
    0xFF,0x00,0x81,0x7E,0xBD,0x42,0xA5,0x5A,0xBD,0x42,0x99,0x66,0xFF,0x00,0x81,0x7E,

    // 8: torch
    0x18,0x00,0x3C,0x00,0x7E,0x18,0x3C,0x24,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,

    // 9: coal
    0x81,0x7E,0x42,0xBD,0x2C,0xD3,0x18,0xE7,0x34,0xCB,0x24,0xDB,0x52,0xAD,0x81,0x7E,

    // 10: copper
    0x81,0x7E,0x5A,0xA5,0x24,0xDB,0x5A,0xA5,0x18,0xE7,0x24,0xDB,0x5A,0xA5,0x81,0x7E,

    // 11: iron
    0x81,0x7E,0x42,0xBD,0x3C,0xC3,0x24,0xDB,0x3C,0xC3,0x24,0xDB,0x42,0xBD,0x81,0x7E,

    // 12: platform
    0x00,0x00,0x00,0x00,0xFF,0x00,0xAA,0x55,0xFF,0x00,0x24,0x24,0x42,0x42,0x00,0x00,

    // 13: closed door top
    0x7E,0x00,0x81,0x7E,0x99,0x66,0x81,0x7E,0x89,0x76,0x81,0x7E,0x99,0x66,0x81,0x7E,

    // 14: closed door bottom
    0x81,0x7E,0x99,0x66,0x81,0x7E,0x89,0x76,0x81,0x7E,0x99,0x66,0x81,0x7E,0xFF,0x00,

    // 15: open door top
    0x3C,0x00,0x24,0x18,0x24,0x18,0x24,0x18,0x24,0x18,0x24,0x18,0x24,0x18,0x3C,0x00,

    // 16: open door bottom
    0x3C,0x00,0x24,0x18,0x24,0x18,0x24,0x18,0x24,0x18,0x24,0x18,0x24,0x18,0x3C,0x00,

    // 17: cursor
    0xFF,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0xFF,0x00,

    // 18: panel
    0x00,0xFF,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0xFF,

    // 19: arrow
    0x10,0x00,0x18,0x00,0x1C,0x00,0xFE,0x00,0xFE,0x00,0x1C,0x00,0x18,0x00,0x10,0x00,

    // 20: lock
    0x3C,0x00,0x42,0x00,0x42,0x00,0xFF,0x00,0xDB,0x00,0xC3,0x00,0xFF,0x00,0x00,0x00,

    // 21-30: digits
    0x3C,0x00,0x66,0x00,0x6E,0x00,0x76,0x00,0x66,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x18,0x00,0x38,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x7E,0x00,0x00,0x00,
    0x3C,0x00,0x66,0x00,0x06,0x00,0x1C,0x00,0x30,0x00,0x60,0x00,0x7E,0x00,0x00,0x00,
    0x3C,0x00,0x66,0x00,0x06,0x00,0x1C,0x00,0x06,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x0C,0x00,0x1C,0x00,0x3C,0x00,0x6C,0x00,0x7E,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,
    0x7E,0x00,0x60,0x00,0x7C,0x00,0x06,0x00,0x06,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x1C,0x00,0x30,0x00,0x60,0x00,0x7C,0x00,0x66,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x7E,0x00,0x06,0x00,0x0C,0x00,0x18,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x00,0x00,
    0x3C,0x00,0x66,0x00,0x66,0x00,0x3C,0x00,0x66,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x3C,0x00,0x66,0x00,0x66,0x00,0x3E,0x00,0x06,0x00,0x0C,0x00,0x38,0x00,0x00,0x00,

    // 31: basic pickaxe
    0x1E,0x00,0x7C,0x00,0x18,0x00,0x38,0x00,0x70,0x00,0xE0,0x00,0x40,0x00,0x00,0x00,

    // 32: iron pickaxe
    0x3F,0x00,0xFE,0x00,0x18,0x00,0x3C,0x00,0x76,0x00,0xE0,0x00,0x40,0x00,0x00,0x00,

    // 33: heart
    0x00,0x00,0x66,0x00,0xFF,0x00,0xFF,0x00,0x7E,0x00,0x3C,0x00,0x18,0x00,0x00,0x00
};

const uint8_t player_tiles[PLAYER_TILE_COUNT * 16u] = {
    // Tile 0: Eyenaut head
    0x3C,0x00, 0x7E,0x00, 0xC3,0x3C, 0xC3,0x7E,
    0xDB,0x3C, 0x66,0x18, 0x3C,0x00, 0x00,0x18,

    // Tile 1: Eyenaut body
    0x24,0x18, 0x42,0x3C, 0x81,0x7E, 0x81,0x3C,
    0x24,0x18, 0x24,0x00, 0x42,0x00, 0xC3,0x00
};

const uint8_t cursor_sprite_tiles[CURSOR_SPRITE_TILE_COUNT * 16u] = {
    // Tile 0: actionable cursor
    0xFF,0x00, 0x81,0x00, 0x81,0x00, 0x81,0x00,
    0x81,0x00, 0x81,0x00, 0x81,0x00, 0xFF,0x00,

    // Tile 1: transparent lower half for 8x16 sprite mode
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,

    // Tile 2: visible but not actionable
    0xFF,0x00, 0x81,0x00, 0xA5,0x00, 0x99,0x00,
    0x99,0x00, 0xA5,0x00, 0x81,0x00, 0xFF,0x00,

    // Tile 3: transparent lower half for 8x16 sprite mode
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,

    // Tile 4: mining progress 33%
    0xFF,0x00, 0x81,0x00, 0x99,0x00, 0x81,0x00,
    0x99,0x00, 0x81,0x00, 0x99,0x00, 0xFF,0x00,

    // Tile 5: transparent lower half for 8x16 sprite mode
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,

    // Tile 6: mining progress 66%
    0xFF,0x00, 0x99,0x00, 0xA5,0x00, 0x99,0x00,
    0xA5,0x00, 0x99,0x00, 0xA5,0x00, 0xFF,0x00,

    // Tile 7: transparent lower half for 8x16 sprite mode
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00
};

const uint8_t enemy_sprite_tiles[ENEMY_SPRITE_TILE_COUNT * 16u] = {
    // Tile 0: slime body
    0x00,0x00, 0x3C,0x00, 0x7E,0x24, 0xDB,0x24,
    0xFF,0x18, 0xBD,0x42, 0xFF,0x00, 0x66,0x00,

    // Tile 1: transparent lower half for 8x16 sprite mode
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00
};

const uint8_t drop_sprite_tiles[DROP_SPRITE_TILE_COUNT * 16u] = {
    // Tile 0: small generic pickup
    0x00,0x00, 0x18,0x00, 0x3C,0x00, 0x7E,0x18,
    0x7E,0x18, 0x3C,0x00, 0x18,0x00, 0x00,0x00,

    // Tile 1: transparent lower half for 8x16 sprite mode
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00
};

void render_init(void)
{
    uint8_t i;

    DISPLAY_OFF;
    SPRITES_8x16;
    set_bkg_data(0u, TILE_NORMAL_COUNT, terrain_tiles);
    init_light_tiles();
    set_sprite_data(PLAYER_TILE_BASE, PLAYER_TILE_COUNT, player_tiles);
    set_sprite_data(CURSOR_SPRITE_TILE_BASE, CURSOR_SPRITE_TILE_COUNT, cursor_sprite_tiles);
    set_sprite_data(ENEMY_SPRITE_TILE_BASE, ENEMY_SPRITE_TILE_COUNT, enemy_sprite_tiles);
    set_sprite_data(DROP_SPRITE_TILE_BASE, DROP_SPRITE_TILE_COUNT, drop_sprite_tiles);
    set_sprite_tile(PLAYER_SPRITE, PLAYER_TILE_BASE);
    set_sprite_tile(CURSOR_SPRITE, CURSOR_SPRITE_TILE_BASE);
    move_sprite(CURSOR_SPRITE, SPRITE_HIDE_X, SPRITE_HIDE_Y);

    for (i = 0u; i < ENEMY_MAX_ACTIVE; ++i) {
        set_sprite_tile((uint8_t)(ENEMY_FIRST_SPRITE + i), ENEMY_SPRITE_TILE_BASE);
        move_sprite((uint8_t)(ENEMY_FIRST_SPRITE + i), SPRITE_HIDE_X, SPRITE_HIDE_Y);
    }

    for (i = 0u; i < ITEM_DROP_MAX_COUNT; ++i) {
        set_sprite_tile((uint8_t)(DROP_FIRST_SPRITE + i), DROP_SPRITE_TILE_BASE);
        move_sprite((uint8_t)(DROP_FIRST_SPRITE + i), SPRITE_HIDE_X, SPRITE_HIDE_Y);
    }

    rendered_origin_tile = 0u;
    world_rendered = 0u;
    move_win(7u, HUD_Y);
    SHOW_WIN;
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
}

static void build_light_variant(uint8_t tile, uint8_t level)
{
    uint8_t row;
    const uint8_t *source = &terrain_tiles[(uint16_t)tile * 16u];

    for (row = 0u; row < 8u; ++row) {
        uint8_t pattern = level == 1u ?
                          (row & 1u ? 0x44u : 0x11u) :
                          (row & 1u ? 0xAAu : 0x55u);
        uint8_t i = (uint8_t)(row * 2u);

        tile_variant_buffer[i] = source[i] | pattern;
        tile_variant_buffer[(uint8_t)(i + 1u)] = source[(uint8_t)(i + 1u)] | pattern;
    }
}

static void init_light_tiles(void)
{
    uint8_t tile;

    for (tile = 0u; tile < TILE_WORLD_COUNT; ++tile) {
        build_light_variant(tile, 1u);
        set_bkg_data((uint8_t)(TILE_DARK_BASE + tile), 1u, tile_variant_buffer);

        build_light_variant(tile, 2u);
        set_bkg_data((uint8_t)(TILE_VERY_DARK_BASE + tile), 1u, tile_variant_buffer);
    }
}

static void clear_window(uint8_t rows)
{
    uint16_t i;

    for (i = 0u; i < (uint16_t)(WIN_WIDTH * rows); ++i) {
        win_buffer[i] = TILE_UI_PANEL;
    }
}

static void put_win_tile(uint8_t x, uint8_t y, uint8_t tile)
{
    if (x < WIN_WIDTH && y < WIN_HEIGHT) {
        win_buffer[(y * WIN_WIDTH) + x] = tile;
    }
}

static void put_count(uint8_t x, uint8_t y, uint8_t count)
{
    if (count > 99u) {
        count = 99u;
    }

    put_win_tile(x, y, (uint8_t)(TILE_DIGIT_0 + (count / 10u)));
    put_win_tile((uint8_t)(x + 1u), y, (uint8_t)(TILE_DIGIT_0 + (count % 10u)));
}

static void draw_inventory_slots(const Inventory *inventory, uint8_t y)
{
    uint8_t slot;

    for (slot = 0u; slot < INVENTORY_SLOT_COUNT; ++slot) {
        uint8_t x = (uint8_t)(slot * 2u);

        put_win_tile(x, y,
                     slot == inventory->selected_slot ? TILE_UI_CURSOR : TILE_UI_PANEL);
        put_win_tile(x, (uint8_t)(y + 1u), inventory_item_tile(slot));
        put_count(x, (uint8_t)(y + 2u), inventory->counts[slot]);
    }
}

void render_world(const Camera *camera)
{
    int16_t delta;
    uint16_t dirty_tx;
    uint16_t origin_tile = (uint16_t)(camera->x >> 3);
    uint8_t dirty_ty;
    uint8_t dirty_budget = RENDER_DIRTY_TILE_BUDGET;
    bool full_redraw = false;
    bool lighting_dirty = world_lighting_dirty();

    if (!world_rendered) {
        uint8_t x;

        for (x = 0u; x < BG_BUFFER_WIDTH; ++x) {
            render_world_column((uint16_t)(origin_tile + x));
        }

        rendered_origin_tile = origin_tile;
        world_rendered = 1u;
        world_clear_lighting_dirty();

        while (world_take_dirty_tile(&dirty_tx, &dirty_ty)) {
            /* The full redraw already covered queued world_init changes. */
        }

        return;
    }

    delta = (int16_t)(origin_tile - rendered_origin_tile);

    if (lighting_dirty ||
        delta >= (int16_t)BG_BUFFER_WIDTH ||
        delta <= -(int16_t)BG_BUFFER_WIDTH) {
        uint8_t x;

        for (x = 0u; x < BG_BUFFER_WIDTH; ++x) {
            render_world_column((uint16_t)(origin_tile + x));
        }

        full_redraw = true;
    } else if (delta > 0) {
        uint8_t x;

        for (x = 0u; x < (uint8_t)delta; ++x) {
            render_world_column((uint16_t)(rendered_origin_tile + BG_BUFFER_WIDTH + x));
        }
    } else if (delta < 0) {
        uint8_t x;
        uint8_t count = (uint8_t)-delta;

        for (x = 0u; x < count; ++x) {
            render_world_column((uint16_t)(origin_tile + x));
        }
    }

    rendered_origin_tile = origin_tile;

    if (lighting_dirty) {
        world_clear_lighting_dirty();
    }

    if (full_redraw) {
        while (world_take_dirty_tile(&dirty_tx, &dirty_ty)) {
            /* The full redraw already covered queued world changes. */
        }

        return;
    }

    while (dirty_budget != 0u && world_take_dirty_tile(&dirty_tx, &dirty_ty)) {
        if (dirty_ty < WORLD_HEIGHT_TILES &&
            dirty_tx >= origin_tile &&
            dirty_tx < (uint16_t)(origin_tile + BG_BUFFER_WIDTH)) {
            uint8_t tile = render_tile_for_world_pos(dirty_tx, dirty_ty);
            set_bkg_tiles((uint8_t)(dirty_tx & 31u), (uint8_t)(dirty_ty & 31u), 1u, 1u, &tile);
        }

        --dirty_budget;
    }
}

static uint8_t render_tile_for_world_pos(uint16_t tx, uint8_t ty)
{
    uint8_t tile = world_get_tile_or_empty(tx, ty);

    if (tile >= TILE_WORLD_COUNT) {
        return tile;
    }

    switch (world_light_level(tx, ty)) {
    case 0u:
        return tile;
    case 1u:
        return (uint8_t)(TILE_DARK_BASE + tile);
    default:
        return (uint8_t)(TILE_VERY_DARK_BASE + tile);
    }
}

static void render_world_column(uint16_t world_x)
{
    uint8_t y;
    uint8_t map_x = (uint8_t)(world_x & 31u);

    for (y = 0u; y < WORLD_HEIGHT_TILES; ++y) {
        bg_buffer[y] = render_tile_for_world_pos(world_x, y);
    }

    set_bkg_tiles(map_x, 0u, 1u, WORLD_HEIGHT_TILES, bg_buffer);
}

static void draw_player_hp(const Player *player)
{
    put_win_tile(18u, 0u, TILE_UI_HEART);
    put_win_tile(19u, 0u, (uint8_t)(TILE_DIGIT_0 + player->hp));
}

void render_hud(const Inventory *inventory, const Player *player)
{
    SHOW_SPRITES;
    move_win(7u, HUD_Y);
    clear_window(HUD_HEIGHT);
    draw_inventory_slots(inventory, 0u);
    draw_player_hp(player);
    set_win_tiles(0u, 0u, WIN_WIDTH, HUD_HEIGHT, win_buffer);
}

static void draw_recipe_input(const Inventory *inventory,
                              uint8_t recipe_index,
                              uint8_t input_index,
                              uint8_t x,
                              uint8_t y,
                              bool flash_missing)
{
    const Recipe *recipe = inventory_recipe(recipe_index);
    uint8_t item = input_index == 0u ? recipe->input_a : recipe->input_b;
    uint8_t count = input_index == 0u ? recipe->input_a_count : recipe->input_b_count;
    bool missing = inventory_recipe_input_missing(inventory, recipe_index, input_index);

    if (item == ITEM_NONE) {
        return;
    }

    if (flash_missing && missing) {
        put_win_tile(x, y, TILE_UI_PANEL);
        put_win_tile((uint8_t)(x + 1u), y, TILE_UI_PANEL);
        put_win_tile((uint8_t)(x + 2u), y, TILE_UI_PANEL);
        return;
    }

    put_win_tile(x, y, inventory_item_tile(item));
    put_count((uint8_t)(x + 1u), y, count);
}

static void draw_recipe_row(const Inventory *inventory,
                            uint8_t recipe_index,
                            bool near_workbench,
                            uint8_t craft_error_timer,
                            uint8_t y)
{
    const Recipe *recipe = inventory_recipe(recipe_index);
    uint8_t craft_status = inventory_craft_status(inventory, recipe_index, near_workbench);
    bool selected = recipe_index == inventory->selected_recipe;
    bool flash_missing = selected &&
                         craft_error_timer != 0u &&
                         (craft_error_timer & 4u) != 0u &&
                         craft_status == CRAFT_STATUS_NEEDS_MATERIAL;
    uint8_t output_tile = recipe->output_tool_level != TOOL_LEVEL_NONE ?
                          inventory_tool_tile(recipe->output_tool_level) :
                          inventory_item_tile(recipe->output_item);
    uint8_t output_count = recipe->output_tool_level != TOOL_LEVEL_NONE ?
                           recipe->output_tool_level :
                           recipe->output_count;

    put_win_tile(0u, y, selected ? TILE_UI_CURSOR : TILE_UI_PANEL);
    draw_recipe_input(inventory, recipe_index, 0u, 2u, y, flash_missing);
    draw_recipe_input(inventory, recipe_index, 1u, 6u, y, flash_missing);

    put_win_tile(10u, y, TILE_UI_ARROW);
    put_win_tile(12u, y, output_tile);
    put_count(13u, y, output_count);

    if (craft_status == CRAFT_STATUS_NEEDS_WORKBENCH) {
        put_win_tile(18u, y, TILE_UI_LOCK);
    }
}

static void draw_resource_counts(const Inventory *inventory, uint8_t y)
{
    put_win_tile(0u, y, TILE_COAL);
    put_count(1u, y, inventory->counts[ITEM_COAL]);
    put_win_tile(5u, y, TILE_COPPER);
    put_count(6u, y, inventory->counts[ITEM_COPPER]);
    put_win_tile(10u, y, TILE_IRON);
    put_count(11u, y, inventory->counts[ITEM_IRON]);
    put_win_tile(15u, y, inventory_tool_tile(inventory->tool_level));
    put_count(16u, y, inventory->tool_level);
}

void render_menu(const Inventory *inventory, bool near_workbench, uint8_t craft_error_timer)
{
    uint8_t i;

    HIDE_SPRITES;
    move_win(7u, 0u);
    clear_window(WIN_HEIGHT);

    put_win_tile(1u, 1u, TILE_WORKBENCH);
    put_win_tile(3u, 1u, near_workbench ? TILE_UI_CURSOR : TILE_UI_LOCK);

    for (i = 0u; i < RECIPE_COUNT; ++i) {
        draw_recipe_row(inventory, i, near_workbench, craft_error_timer, (uint8_t)(3u + i));
    }

    draw_resource_counts(inventory, 12u);
    draw_inventory_slots(inventory, 14u);
    set_win_tiles(0u, 0u, WIN_WIDTH, WIN_HEIGHT, win_buffer);
}

void render_frame(const Camera *camera, const Player *player, const Enemy *enemies, const ItemDrop *drops)
{
    uint8_t i;
    int16_t sprite_x = (int16_t)(player->x - (int16_t)camera->x + 8);
    int16_t sprite_y = (int16_t)(player->y - camera->y + 16);
    int16_t cursor_x = (int16_t)((player->aim_tx << 3) - camera->x + 8);
    int16_t cursor_y = (int16_t)((player->aim_ty << 3) - camera->y + 16);

    move_bkg((uint8_t)camera->x, camera->y);

    if (player->invuln_timer != 0u && (player->invuln_timer & 4u)) {
        move_sprite(PLAYER_SPRITE, SPRITE_HIDE_X, SPRITE_HIDE_Y);
    } else {
        move_sprite(PLAYER_SPRITE, (uint8_t)sprite_x, (uint8_t)sprite_y);
    }

    if ((player->aim_flags & PLAYER_AIM_VISIBLE) &&
        cursor_x >= 0 && cursor_x < 168 &&
        cursor_y >= 0 && cursor_y < 160) {
        if (player->action_error_timer != 0u &&
            !(player->aim_flags & PLAYER_AIM_ACTIONABLE) &&
            (player->action_error_timer & 4u)) {
            move_sprite(CURSOR_SPRITE, SPRITE_HIDE_X, SPRITE_HIDE_Y);
        } else {
            uint8_t cursor_tile = CURSOR_SPRITE_TILE_BASE;

            if (player->aim_flags & PLAYER_AIM_PROGRESS_2) {
                cursor_tile = (uint8_t)(CURSOR_SPRITE_TILE_BASE + 6u);
            } else if (player->aim_flags & PLAYER_AIM_PROGRESS_1) {
                cursor_tile = (uint8_t)(CURSOR_SPRITE_TILE_BASE + 4u);
            } else if (!(player->aim_flags & PLAYER_AIM_ACTIONABLE)) {
                cursor_tile = (uint8_t)(CURSOR_SPRITE_TILE_BASE + 2u);
            }

            set_sprite_tile(CURSOR_SPRITE, cursor_tile);
            move_sprite(CURSOR_SPRITE, (uint8_t)cursor_x, (uint8_t)cursor_y);
        }
    } else {
        move_sprite(CURSOR_SPRITE, SPRITE_HIDE_X, SPRITE_HIDE_Y);
    }

    for (i = 0u; i < ENEMY_MAX_ACTIVE; ++i) {
        int16_t enemy_x = (int16_t)(enemies[i].x - (int16_t)camera->x + 8);
        int16_t enemy_y = (int16_t)(enemies[i].y - camera->y + 16);
        uint8_t sprite = (uint8_t)(ENEMY_FIRST_SPRITE + i);

        if (enemies[i].active &&
            enemy_x >= 0 && enemy_x < 168 &&
            enemy_y >= 0 && enemy_y < 160) {
            move_sprite(sprite, (uint8_t)enemy_x, (uint8_t)enemy_y);
        } else {
            move_sprite(sprite, SPRITE_HIDE_X, SPRITE_HIDE_Y);
        }
    }

    for (i = 0u; i < ITEM_DROP_MAX_COUNT; ++i) {
        int16_t drop_x = (int16_t)((int16_t)drops[i].x - (int16_t)camera->x + 8);
        int16_t drop_y = (int16_t)(drops[i].y - camera->y + 16);
        uint8_t sprite = (uint8_t)(DROP_FIRST_SPRITE + i);

        if (drops[i].active &&
            drop_x >= 0 && drop_x < 168 &&
            drop_y >= 0 && drop_y < 160) {
            move_sprite(sprite, (uint8_t)drop_x, (uint8_t)drop_y);
        } else {
            move_sprite(sprite, SPRITE_HIDE_X, SPRITE_HIDE_Y);
        }
    }
}
