#include <gb/gb.h>
#include "camera.h"
#include "chunk.h"
#include "game.h"
#include "input.h"
#include "inventory.h"
#include "player.h"
#include "render.h"
#include "world.h"

static InputState input;
static Player player;
static Camera camera;
static Inventory inventory;
static uint8_t menu_open;
static uint8_t hud_dirty;
static uint8_t menu_dirty;
static bool workbench_nearby;

static bool inventory_changed(const Inventory *a, const Inventory *b)
{
    uint8_t i;

    if (a->selected_slot != b->selected_slot ||
        a->selected_recipe != b->selected_recipe) {
        return true;
    }

    for (i = 0u; i < INVENTORY_SLOT_COUNT; ++i) {
        if (a->counts[i] != b->counts[i]) {
            return true;
        }
    }

    return false;
}

static bool player_near_workbench(void)
{
    return world_has_tile_near_pixel((int16_t)(player.x + (PLAYER_WIDTH / 2)),
                                     (int16_t)(player.y + (PLAYER_HEIGHT / 2)),
                                     TILE_WORKBENCH,
                                     2u);
}

void game_init(void)
{
    input.current = joypad();
    input.previous = input.current;
    input.pressed = 0u;

    world_init();
    inventory_init(&inventory);
    player_init(&player);
    camera.x = 0u;
    camera.y = 0u;
    menu_open = 0u;
    hud_dirty = 1u;
    menu_dirty = 0u;
    workbench_nearby = false;

    render_init();

    /* TODO: add save data after the world representation is stable. */
    /* TODO: fake lighting with tile variants or coarse palettes on CGB later. */
}

void game_update_logic(void)
{
    Inventory previous_inventory;

    input_update(&input);

    if (input.pressed & J_SELECT) {
        menu_open = (uint8_t)!menu_open;

        if (menu_open) {
            menu_dirty = 1u;
        } else {
            hud_dirty = 1u;
        }
    }

    workbench_nearby = player_near_workbench();

    if (menu_open) {
        previous_inventory = inventory;

        if ((input.pressed & J_RIGHT) || (input.pressed & J_DOWN)) {
            inventory_next_recipe(&inventory);
        }

        if ((input.pressed & J_LEFT) || (input.pressed & J_UP)) {
            inventory_prev_recipe(&inventory);
        }

        if (input.pressed & J_A) {
            inventory_craft_selected(&inventory, workbench_nearby);
        }

        if (input.pressed & J_B) {
            menu_open = 0u;
            hud_dirty = 1u;
            menu_dirty = 0u;
            return;
        }

        if (menu_dirty || inventory_changed(&previous_inventory, &inventory)) {
            menu_dirty = 1u;
        }

        return;
    }

    previous_inventory = inventory;
    player_update(&player, &input, &inventory);
    camera_update(&camera, &player);
    chunk_tick(camera.x);

    if (inventory_changed(&previous_inventory, &inventory)) {
        hud_dirty = 1u;
    }
}

void game_render_commit(void)
{
    if (menu_open) {
        if (menu_dirty) {
            render_menu(&inventory, workbench_nearby);
            menu_dirty = 0u;
        }

        render_frame(&camera, &player);
        return;
    }

    render_world(&camera);

    if (hud_dirty) {
        render_hud(&inventory);
        hud_dirty = 0u;
    }

    render_frame(&camera, &player);
}
