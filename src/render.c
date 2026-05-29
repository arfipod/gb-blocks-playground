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

static uint8_t bg_buffer[WORLD_HEIGHT_TILES];
static uint8_t win_buffer[WIN_WIDTH * WIN_HEIGHT];
static uint16_t rendered_origin_tile;
static uint8_t world_rendered;

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

    // 9: cursor
    0xFF,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0xFF,0x00,

    // 10: panel
    0x00,0xFF,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0xFF,

    // 11: arrow
    0x10,0x00,0x18,0x00,0x1C,0x00,0xFE,0x00,0xFE,0x00,0x1C,0x00,0x18,0x00,0x10,0x00,

    // 12: lock
    0x3C,0x00,0x42,0x00,0x42,0x00,0xFF,0x00,0xDB,0x00,0xC3,0x00,0xFF,0x00,0x00,0x00,

    // 13-22: digits
    0x3C,0x00,0x66,0x00,0x6E,0x00,0x76,0x00,0x66,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x18,0x00,0x38,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x7E,0x00,0x00,0x00,
    0x3C,0x00,0x66,0x00,0x06,0x00,0x1C,0x00,0x30,0x00,0x60,0x00,0x7E,0x00,0x00,0x00,
    0x3C,0x00,0x66,0x00,0x06,0x00,0x1C,0x00,0x06,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x0C,0x00,0x1C,0x00,0x3C,0x00,0x6C,0x00,0x7E,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,
    0x7E,0x00,0x60,0x00,0x7C,0x00,0x06,0x00,0x06,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x1C,0x00,0x30,0x00,0x60,0x00,0x7C,0x00,0x66,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x7E,0x00,0x06,0x00,0x0C,0x00,0x18,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x00,0x00,
    0x3C,0x00,0x66,0x00,0x66,0x00,0x3C,0x00,0x66,0x00,0x66,0x00,0x3C,0x00,0x00,0x00,
    0x3C,0x00,0x66,0x00,0x66,0x00,0x3E,0x00,0x06,0x00,0x0C,0x00,0x38,0x00,0x00,0x00
};

const uint8_t player_tiles[PLAYER_TILE_COUNT * 16u] = {
    // Tile 0: head + ears
    0x42,0x42, 0xE7,0x42, 0x7E,0x3C, 0x5A,0x66,
    0xBD,0xDB, 0xA5,0xDB, 0x5A,0x7E, 0x7E,0x3C,

    // Tile 1: body + legs
    0x3C,0x3C, 0x5A,0x66, 0x99,0xFF, 0x5A,0x7E,
    0x3C,0x3C, 0x18,0x24, 0x24,0x66, 0x66,0x66
};

void render_init(void)
{
    DISPLAY_OFF;
    SPRITES_8x16;
    set_bkg_data(0u, TILE_COUNT, terrain_tiles);
    set_sprite_data(PLAYER_TILE_BASE, PLAYER_TILE_COUNT, player_tiles);
    set_sprite_tile(0u, PLAYER_TILE_BASE);
    rendered_origin_tile = 0u;
    world_rendered = 0u;
    move_win(7u, HUD_Y);
    SHOW_WIN;
    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;
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
        uint8_t x = (uint8_t)(slot * 3u);

        put_win_tile((uint8_t)(x + 1u), y,
                     slot == inventory->selected_slot ? TILE_UI_CURSOR : TILE_UI_PANEL);
        put_win_tile((uint8_t)(x + 1u), (uint8_t)(y + 1u), inventory_item_tile(slot));
        put_count(x, (uint8_t)(y + 2u), inventory->counts[slot]);
    }
}

void render_world(const Camera *camera)
{
    int16_t delta;
    uint16_t dirty_tx;
    uint16_t origin_tile = (uint16_t)(camera->x >> 3);
    uint8_t dirty_ty;

    if (!world_rendered) {
        uint8_t x;

        for (x = 0u; x < BG_BUFFER_WIDTH; ++x) {
            render_world_column((uint16_t)(origin_tile + x));
        }

        rendered_origin_tile = origin_tile;
        world_rendered = 1u;

        while (world_take_dirty_tile(&dirty_tx, &dirty_ty)) {
            /* The full redraw already covered queued world_init changes. */
        }

        return;
    }

    delta = (int16_t)(origin_tile - rendered_origin_tile);

    if (delta >= (int16_t)BG_BUFFER_WIDTH || delta <= -(int16_t)BG_BUFFER_WIDTH) {
        uint8_t x;

        for (x = 0u; x < BG_BUFFER_WIDTH; ++x) {
            render_world_column((uint16_t)(origin_tile + x));
        }
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

    while (world_take_dirty_tile(&dirty_tx, &dirty_ty)) {
        if (dirty_ty < WORLD_HEIGHT_TILES &&
            dirty_tx >= origin_tile &&
            dirty_tx < (uint16_t)(origin_tile + BG_BUFFER_WIDTH)) {
            uint8_t tile = world_get_tile_or_empty(dirty_tx, dirty_ty);
            set_bkg_tiles((uint8_t)(dirty_tx & 31u), dirty_ty, 1u, 1u, &tile);
        }
    }
}

static void render_world_column(uint16_t world_x)
{
    uint8_t y;
    uint8_t map_x = (uint8_t)(world_x & 31u);

    for (y = 0u; y < WORLD_HEIGHT_TILES; ++y) {
        bg_buffer[y] = world_get_tile_or_empty(world_x, y);
    }

    set_bkg_tiles(map_x, 0u, 1u, WORLD_HEIGHT_TILES, bg_buffer);
}

void render_hud(const Inventory *inventory)
{
    SHOW_SPRITES;
    move_win(7u, HUD_Y);
    clear_window(HUD_HEIGHT);
    draw_inventory_slots(inventory, 0u);
    set_win_tiles(0u, 0u, WIN_WIDTH, HUD_HEIGHT, win_buffer);
}

static void draw_recipe_row(const Inventory *inventory, uint8_t recipe_index, bool near_workbench, uint8_t y)
{
    const Recipe *recipe = inventory_recipe(recipe_index);
    bool can_craft = inventory_can_craft(inventory, recipe_index, near_workbench);

    put_win_tile(0u, y, recipe_index == inventory->selected_recipe ? TILE_UI_CURSOR : TILE_UI_PANEL);
    put_win_tile(2u, y, inventory_item_tile(recipe->input_a));
    put_count(3u, y, recipe->input_a_count);

    if (recipe->input_b != ITEM_NONE) {
        put_win_tile(6u, y, inventory_item_tile(recipe->input_b));
        put_count(7u, y, recipe->input_b_count);
    }

    put_win_tile(10u, y, TILE_UI_ARROW);
    put_win_tile(12u, y, inventory_item_tile(recipe->output_item));
    put_count(13u, y, recipe->output_count);

    if (!can_craft) {
        put_win_tile(18u, y, TILE_UI_LOCK);
    }
}

void render_menu(const Inventory *inventory, bool near_workbench)
{
    uint8_t i;

    HIDE_SPRITES;
    move_win(7u, 0u);
    clear_window(WIN_HEIGHT);

    put_win_tile(1u, 1u, TILE_WORKBENCH);
    put_win_tile(3u, 1u, near_workbench ? TILE_UI_CURSOR : TILE_UI_LOCK);

    for (i = 0u; i < RECIPE_COUNT; ++i) {
        draw_recipe_row(inventory, i, near_workbench, (uint8_t)(3u + (i * 2u)));
    }

    draw_inventory_slots(inventory, 14u);
    set_win_tiles(0u, 0u, WIN_WIDTH, WIN_HEIGHT, win_buffer);
}

void render_frame(const Camera *camera, const Player *player)
{
    int16_t sprite_x = (int16_t)(player->x - (int16_t)camera->x + 8);
    int16_t sprite_y = (int16_t)(player->y - camera->y + 16);

    move_bkg((uint8_t)camera->x, camera->y);
    move_sprite(0u, (uint8_t)sprite_x, (uint8_t)sprite_y);
}
