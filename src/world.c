#include "world.h"
#include "tile_defs.h"

#define WORLD_DIRTY_TILE_COUNT 128u
#define WORLD_TORCH_LIGHT_RADIUS 4u
#define WORLD_TORCH_BRIGHT_RADIUS 2u

typedef struct DirtyTile {
    uint16_t tx;
    uint8_t ty;
} DirtyTile;

static uint8_t tiles[WORLD_ACTIVE_WIDTH_TILES * WORLD_HEIGHT_TILES];
static uint8_t light_levels[WORLD_ACTIVE_WIDTH_TILES * WORLD_HEIGHT_TILES];
static DirtyTile dirty_tiles[WORLD_DIRTY_TILE_COUNT];
static ChangedTile changed_tiles[WORLD_CHANGE_COUNT];
static uint8_t dirty_count;
static uint8_t changed_count;
static uint8_t active_start_chunk;
static uint8_t lighting_dirty;
static uint16_t active_seed = WORLD_DEFAULT_SEED;

static uint16_t active_start_tile(void)
{
    return (uint16_t)(active_start_chunk * CHUNK_WIDTH_TILES);
}

static bool world_tile_is_active(uint16_t tx)
{
    uint16_t start_tile = active_start_tile();

    return tx >= start_tile && tx < (uint16_t)(start_tile + WORLD_ACTIVE_WIDTH_TILES);
}

static uint16_t active_tile_index(uint16_t tx, uint8_t ty)
{
    return (uint16_t)((ty * WORLD_ACTIVE_WIDTH_TILES) + (tx - active_start_tile()));
}

static uint8_t distance_u16_u8(uint16_t ax, uint8_t ay, uint16_t bx, uint8_t by)
{
    uint16_t dx = ax > bx ? (uint16_t)(ax - bx) : (uint16_t)(bx - ax);
    uint8_t dy = ay > by ? (uint8_t)(ay - by) : (uint8_t)(by - ay);

    if (dx > WORLD_TORCH_LIGHT_RADIUS) {
        return (uint8_t)(WORLD_TORCH_LIGHT_RADIUS + 1u);
    }

    return (uint8_t)(dx + dy);
}

static uint8_t world_find_change(uint16_t tx, uint8_t ty)
{
    uint8_t i;

    for (i = 0u; i < changed_count; ++i) {
        if (changed_tiles[i].tx == tx && changed_tiles[i].ty == ty) {
            return i;
        }
    }

    return WORLD_CHANGE_COUNT;
}

static uint8_t world_generated_tile(uint16_t tx, uint8_t ty)
{
    uint8_t chunk = (uint8_t)(tx / CHUNK_WIDTH_TILES);
    uint8_t local_x = (uint8_t)(tx % CHUNK_WIDTH_TILES);

    return chunk_generate_tile(chunk, local_x, ty);
}

static uint8_t world_generated_tile_with_changes(uint16_t tx, uint8_t ty)
{
    uint8_t change_index = world_find_change(tx, ty);

    if (change_index < WORLD_CHANGE_COUNT) {
        return changed_tiles[change_index].tile;
    }

    return world_generated_tile(tx, ty);
}

static uint8_t light_level_from_changed_torches(uint16_t tx, uint8_t ty)
{
    uint8_t i;
    uint8_t best = 2u;

    for (i = 0u; i < changed_count; ++i) {
        uint8_t distance;

        if (changed_tiles[i].tile != TILE_TORCH) {
            continue;
        }

        distance = distance_u16_u8(tx, ty, changed_tiles[i].tx, changed_tiles[i].ty);

        if (distance <= WORLD_TORCH_BRIGHT_RADIUS) {
            return 0u;
        }

        if (distance <= WORLD_TORCH_LIGHT_RADIUS) {
            best = 1u;
        }
    }

    return best;
}

static void apply_torch_to_light_cache(uint16_t torch_tx, uint8_t torch_ty)
{
    int8_t dx;
    int8_t dy;
    uint16_t start_tile = active_start_tile();

    for (dy = (int8_t)-WORLD_TORCH_LIGHT_RADIUS; dy <= (int8_t)WORLD_TORCH_LIGHT_RADIUS; ++dy) {
        int16_t check_ty = (int16_t)torch_ty + dy;

        if (check_ty < 0 || check_ty >= WORLD_HEIGHT_TILES) {
            continue;
        }

        for (dx = (int8_t)-WORLD_TORCH_LIGHT_RADIUS; dx <= (int8_t)WORLD_TORCH_LIGHT_RADIUS; ++dx) {
            uint8_t distance = (uint8_t)((dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy));
            int16_t check_tx_signed;
            uint16_t check_tx;
            uint16_t index;

            if (distance > WORLD_TORCH_LIGHT_RADIUS) {
                continue;
            }

            check_tx_signed = (int16_t)torch_tx + dx;

            if (check_tx_signed < 0 || check_tx_signed >= WORLD_WIDTH_TILES) {
                continue;
            }

            check_tx = (uint16_t)check_tx_signed;

            if (check_tx < start_tile ||
                check_tx >= (uint16_t)(start_tile + WORLD_ACTIVE_WIDTH_TILES)) {
                continue;
            }

            index = active_tile_index(check_tx, (uint8_t)check_ty);

            if (distance <= WORLD_TORCH_BRIGHT_RADIUS) {
                light_levels[index] = 0u;
            } else if (light_levels[index] > 1u) {
                light_levels[index] = 1u;
            }
        }
    }
}

static void world_rebuild_light_cache(void)
{
    uint16_t i;
    uint8_t x;
    uint8_t y;
    uint16_t start_tile = active_start_tile();

    for (i = 0u; i < (uint16_t)(WORLD_ACTIVE_WIDTH_TILES * WORLD_HEIGHT_TILES); ++i) {
        light_levels[i] = 2u;
    }

    for (y = 0u; y < WORLD_HEIGHT_TILES; ++y) {
        for (x = 0u; x < WORLD_ACTIVE_WIDTH_TILES; ++x) {
            if (tiles[(y * WORLD_ACTIVE_WIDTH_TILES) + x] == TILE_TORCH) {
                apply_torch_to_light_cache((uint16_t)(start_tile + x), y);
            }
        }
    }
}

static void world_mark_dirty(uint16_t tx, uint8_t ty);

static void world_mark_light_area_dirty(uint16_t center_tx, uint8_t center_ty)
{
    int8_t dx;
    int8_t dy;

    for (dy = (int8_t)-WORLD_TORCH_LIGHT_RADIUS; dy <= (int8_t)WORLD_TORCH_LIGHT_RADIUS; ++dy) {
        int16_t check_ty = (int16_t)center_ty + dy;

        if (check_ty < 0 || check_ty >= WORLD_HEIGHT_TILES) {
            continue;
        }

        for (dx = (int8_t)-WORLD_TORCH_LIGHT_RADIUS; dx <= (int8_t)WORLD_TORCH_LIGHT_RADIUS; ++dx) {
            uint8_t distance = (uint8_t)((dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy));
            int16_t check_tx_signed;

            if (distance > WORLD_TORCH_LIGHT_RADIUS) {
                continue;
            }

            check_tx_signed = (int16_t)center_tx + dx;

            if (check_tx_signed < 0 || check_tx_signed >= WORLD_WIDTH_TILES) {
                continue;
            }

            world_mark_dirty((uint16_t)check_tx_signed, (uint8_t)check_ty);
        }
    }
}

static void world_rebuild_light_area(uint16_t center_tx, uint8_t center_ty)
{
    int8_t dx;
    int8_t dy;
    uint16_t start_tile = active_start_tile();

    for (dy = (int8_t)-WORLD_TORCH_LIGHT_RADIUS; dy <= (int8_t)WORLD_TORCH_LIGHT_RADIUS; ++dy) {
        int16_t check_ty = (int16_t)center_ty + dy;

        if (check_ty < 0 || check_ty >= WORLD_HEIGHT_TILES) {
            continue;
        }

        for (dx = (int8_t)-WORLD_TORCH_LIGHT_RADIUS; dx <= (int8_t)WORLD_TORCH_LIGHT_RADIUS; ++dx) {
            uint8_t distance = (uint8_t)((dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy));
            int16_t check_tx_signed;
            uint16_t check_tx;

            if (distance > WORLD_TORCH_LIGHT_RADIUS) {
                continue;
            }

            check_tx_signed = (int16_t)center_tx + dx;

            if (check_tx_signed < 0 || check_tx_signed >= WORLD_WIDTH_TILES) {
                continue;
            }

            check_tx = (uint16_t)check_tx_signed;

            if (check_tx >= start_tile &&
                check_tx < (uint16_t)(start_tile + WORLD_ACTIVE_WIDTH_TILES)) {
                light_levels[active_tile_index(check_tx, (uint8_t)check_ty)] =
                    light_level_from_changed_torches(check_tx, (uint8_t)check_ty);
            }

            world_mark_dirty(check_tx, (uint8_t)check_ty);
        }
    }
}

static void world_save_change(uint16_t tx, uint8_t ty, uint8_t tile)
{
    uint8_t change_index = world_find_change(tx, ty);

    if (tile == world_generated_tile(tx, ty)) {
        if (change_index < WORLD_CHANGE_COUNT) {
            --changed_count;
            changed_tiles[change_index] = changed_tiles[changed_count];
        }

        return;
    }

    if (change_index < WORLD_CHANGE_COUNT) {
        changed_tiles[change_index].tile = tile;
        return;
    }

    if (changed_count < WORLD_CHANGE_COUNT) {
        changed_tiles[changed_count].tx = tx;
        changed_tiles[changed_count].ty = ty;
        changed_tiles[changed_count].tile = tile;
        ++changed_count;
    }
}

static uint8_t clamp_active_start_chunk(uint8_t start_chunk)
{
#if WORLD_ACTIVE_CHUNK_COUNT >= WORLD_CHUNK_COUNT
    (void)start_chunk;
    return 0u;
#else
    if (start_chunk > (uint8_t)(WORLD_CHUNK_COUNT - WORLD_ACTIVE_CHUNK_COUNT)) {
        return (uint8_t)(WORLD_CHUNK_COUNT - WORLD_ACTIVE_CHUNK_COUNT);
    }

    return start_chunk;
#endif
}

static void world_load_generated_window(void)
{
    uint8_t x;
    uint8_t y;
    uint16_t start_tile = active_start_tile();

    for (y = 0u; y < WORLD_HEIGHT_TILES; ++y) {
        for (x = 0u; x < WORLD_ACTIVE_WIDTH_TILES; ++x) {
            uint16_t world_x = (uint16_t)(start_tile + x);

            tiles[(y * WORLD_ACTIVE_WIDTH_TILES) + x] =
                world_generated_tile(world_x, y);
        }
    }
}

void world_apply_changes(void)
{
    uint8_t i;

    for (i = 0u; i < changed_count; ++i) {
        if (changed_tiles[i].ty < WORLD_HEIGHT_TILES &&
            world_tile_is_active(changed_tiles[i].tx)) {
            tiles[active_tile_index(changed_tiles[i].tx, changed_tiles[i].ty)] =
                changed_tiles[i].tile;
        }
    }
}

static void world_load_active_window(void)
{
    world_load_generated_window();
    world_apply_changes();
    world_rebuild_light_cache();
    lighting_dirty = 1u;
}

static void world_mark_dirty(uint16_t tx, uint8_t ty)
{
    uint8_t i;

    for (i = 0u; i < dirty_count; ++i) {
        if (dirty_tiles[i].tx == tx && dirty_tiles[i].ty == ty) {
            return;
        }
    }

    if (dirty_count < WORLD_DIRTY_TILE_COUNT) {
        dirty_tiles[dirty_count].tx = tx;
        dirty_tiles[dirty_count].ty = ty;
        ++dirty_count;
    } else {
        lighting_dirty = 1u;
    }
}

void world_init(void)
{
    chunk_set_seed(active_seed);
    dirty_count = 0u;
    lighting_dirty = 1u;
    active_start_chunk = 0u;
    world_load_active_window();
}

void world_set_seed(uint16_t seed)
{
    active_seed = seed;
    chunk_set_seed(active_seed);
}

uint16_t world_seed(void)
{
    return active_seed;
}

bool world_lighting_dirty(void)
{
    return lighting_dirty != 0u;
}

void world_clear_lighting_dirty(void)
{
    lighting_dirty = 0u;
}

void world_clear_changes(void)
{
    changed_count = 0u;
    world_load_active_window();
    lighting_dirty = 1u;
}

uint8_t world_changed_count(void)
{
    return changed_count;
}

bool world_get_changed_tile(uint8_t index, ChangedTile *change)
{
    if (change == 0 || index >= changed_count) {
        return false;
    }

    *change = changed_tiles[index];
    return true;
}

bool world_restore_changed_tile(const ChangedTile *change)
{
    if (change == 0 ||
        change->tx >= WORLD_WIDTH_TILES ||
        change->ty >= WORLD_HEIGHT_TILES) {
        return false;
    }

    world_save_change(change->tx, change->ty, change->tile);
    world_load_active_window();
    lighting_dirty = 1u;
    return true;
}

Biome world_biome_at_x(uint16_t x)
{
    if (x < 32u) {
        return BIOME_MEADOW;
    }

    if (x < 64u) {
        return BIOME_ROOTWOOD_GROVE;
    }

    if (x < 96u) {
        return BIOME_STONE_BELT;
    }

    return BIOME_CELESTIAL_RUINS;
}

uint8_t world_get_tile_or_empty(uint16_t tx, uint8_t ty)
{
    if (tx >= WORLD_WIDTH_TILES || ty >= WORLD_HEIGHT_TILES) {
        return TILE_EMPTY;
    }

    if (world_tile_is_active(tx)) {
        return tiles[active_tile_index(tx, ty)];
    }

    return world_generated_tile_with_changes(tx, ty);
}

uint8_t world_get_tile_for_collision(uint16_t tx, uint8_t ty)
{
    if (tx >= WORLD_WIDTH_TILES || ty >= WORLD_HEIGHT_TILES) {
        return TILE_STONE;
    }

    if (world_tile_is_active(tx)) {
        return tiles[active_tile_index(tx, ty)];
    }

    return world_generated_tile_with_changes(tx, ty);
}

uint8_t world_light_level(uint16_t tx, uint8_t ty)
{
    if (tx >= WORLD_WIDTH_TILES || ty >= WORLD_HEIGHT_TILES) {
        return 2u;
    }

    if (world_tile_is_active(tx)) {
        return light_levels[active_tile_index(tx, ty)];
    }

    return light_level_from_changed_torches(tx, ty);
}

void world_set_tile(uint16_t tx, uint8_t ty, uint8_t tile)
{
    if (tx < WORLD_WIDTH_TILES && ty < WORLD_HEIGHT_TILES) {
        uint8_t previous_tile = world_get_tile_or_empty(tx, ty);

        if (previous_tile == tile) {
            return;
        }

        if (world_tile_is_active(tx)) {
            tiles[active_tile_index(tx, ty)] = tile;
        }

        world_save_change(tx, ty, tile);

        if (previous_tile == TILE_TORCH) {
            world_rebuild_light_area(tx, ty);
        } else if (tile == TILE_TORCH) {
            if (world_tile_is_active(tx)) {
                apply_torch_to_light_cache(tx, ty);
            }

            world_mark_light_area_dirty(tx, ty);
        } else {
            world_mark_dirty(tx, ty);
        }
    }
}

uint8_t world_stream_to_chunk(uint8_t start_chunk)
{
    uint8_t clamped_start_chunk = clamp_active_start_chunk(start_chunk);

    if (clamped_start_chunk != active_start_chunk) {
        active_start_chunk = clamped_start_chunk;
        world_load_active_window();
    }

    return active_start_chunk;
}

bool world_take_dirty_tile(uint16_t *tx, uint8_t *ty)
{
    if (dirty_count == 0u) {
        return false;
    }

    --dirty_count;
    *tx = dirty_tiles[dirty_count].tx;
    *ty = dirty_tiles[dirty_count].ty;
    return true;
}

bool world_is_solid_tile(uint8_t tile)
{
    return tile != TILE_EMPTY &&
           tile != TILE_TORCH &&
           tile != TILE_DOOR_OPEN_TOP &&
           tile != TILE_DOOR_OPEN_BOTTOM;
}

bool world_is_door_tile(uint8_t tile)
{
    return tile == TILE_DOOR_CLOSED_TOP ||
           tile == TILE_DOOR_CLOSED_BOTTOM ||
           tile == TILE_DOOR_OPEN_TOP ||
           tile == TILE_DOOR_OPEN_BOTTOM;
}

bool world_is_solid_at_pixel(int16_t x, int16_t y)
{
    if (x < 0 || y < 0) {
        return true;
    }

    return world_is_solid_tile(world_get_tile_for_collision((uint16_t)(x >> 3), (uint8_t)(y >> 3)));
}

bool world_find_surface_spawn(uint16_t tx, int16_t *x, int16_t *y)
{
    uint8_t ty;

    if (tx >= WORLD_WIDTH_TILES || x == 0 || y == 0) {
        return false;
    }

    ty = chunk_surface_y(tx);

    if (ty == 0u || ty >= WORLD_HEIGHT_TILES) {
        return false;
    }

    if (world_tile_is_active(tx)) {
        uint16_t column = (uint16_t)(tx - active_start_tile());
        uint16_t above_index = (uint16_t)(((uint8_t)(ty - 1u) * WORLD_ACTIVE_WIDTH_TILES) + column);
        uint16_t ground_index = (uint16_t)((ty * WORLD_ACTIVE_WIDTH_TILES) + column);

        if (tiles[above_index] != TILE_EMPTY ||
            !world_is_solid_tile(tiles[ground_index])) {
            return false;
        }
    }

    *x = (int16_t)(tx << 3);
    *y = (int16_t)((uint8_t)(ty - 1u) << 3);
    return true;
}

uint8_t world_mine_at_pixel(int16_t x, int16_t y)
{
    uint16_t tx;
    uint8_t ty;
    uint8_t tile;

    if (x < 0 || y < 0) {
        return TILE_EMPTY;
    }

    tx = (uint16_t)(x >> 3);
    ty = (uint8_t)(y >> 3);

    if (tx >= WORLD_WIDTH_TILES || ty >= WORLD_HEIGHT_TILES) {
        return TILE_EMPTY;
    }

    tile = world_get_tile_or_empty(tx, ty);

    if (tile != TILE_EMPTY) {
        world_set_tile(tx, ty, TILE_EMPTY);
    }

    return tile;
}

bool world_place_at_pixel(int16_t x, int16_t y, uint8_t tile)
{
    uint16_t tx;
    uint8_t ty;

    if (x < 0 || y < 0) {
        return false;
    }

    tx = (uint16_t)(x >> 3);
    ty = (uint8_t)(y >> 3);

    if (tx >= WORLD_WIDTH_TILES || ty >= WORLD_HEIGHT_TILES) {
        return false;
    }

    return world_place_tile(tx, ty, tile);
}

static bool world_has_solid_neighbor(uint16_t tx, uint8_t ty)
{
    if (tx > 0u && world_is_solid_tile(world_get_tile_or_empty((uint16_t)(tx - 1u), ty))) {
        return true;
    }

    if ((uint16_t)(tx + 1u) < WORLD_WIDTH_TILES &&
        world_is_solid_tile(world_get_tile_or_empty((uint16_t)(tx + 1u), ty))) {
        return true;
    }

    if (ty > 0u && world_is_solid_tile(world_get_tile_or_empty(tx, (uint8_t)(ty - 1u)))) {
        return true;
    }

    if ((uint8_t)(ty + 1u) < WORLD_HEIGHT_TILES &&
        world_is_solid_tile(world_get_tile_or_empty(tx, (uint8_t)(ty + 1u)))) {
        return true;
    }

    return false;
}

static bool world_torch_has_support(uint16_t tx, uint8_t ty)
{
    if (tx > 0u && world_is_solid_tile(world_get_tile_or_empty((uint16_t)(tx - 1u), ty))) {
        return true;
    }

    if ((uint16_t)(tx + 1u) < WORLD_WIDTH_TILES &&
        world_is_solid_tile(world_get_tile_or_empty((uint16_t)(tx + 1u), ty))) {
        return true;
    }

    if ((uint8_t)(ty + 1u) < WORLD_HEIGHT_TILES &&
        world_is_solid_tile(world_get_tile_or_empty(tx, (uint8_t)(ty + 1u)))) {
        return true;
    }

    return false;
}

static bool world_workbench_has_floor(uint16_t tx, uint8_t ty)
{
    return (uint8_t)(ty + 1u) < WORLD_HEIGHT_TILES &&
           world_is_solid_tile(world_get_tile_or_empty(tx, (uint8_t)(ty + 1u)));
}

static uint8_t world_door_bottom_ty(uint8_t tile, uint8_t ty)
{
    if (tile == TILE_DOOR_CLOSED_TOP || tile == TILE_DOOR_OPEN_TOP) {
        return (uint8_t)(ty + 1u);
    }

    return ty;
}

static bool world_can_place_door(uint16_t tx, uint8_t bottom_ty)
{
    if (tx >= WORLD_WIDTH_TILES ||
        bottom_ty == 0u ||
        bottom_ty >= WORLD_HEIGHT_TILES ||
        (uint8_t)(bottom_ty + 1u) >= WORLD_HEIGHT_TILES) {
        return false;
    }

    if (world_get_tile_or_empty(tx, bottom_ty) != TILE_EMPTY ||
        world_get_tile_or_empty(tx, (uint8_t)(bottom_ty - 1u)) != TILE_EMPTY) {
        return false;
    }

    return world_is_solid_tile(world_get_tile_or_empty(tx, (uint8_t)(bottom_ty + 1u)));
}

bool world_can_place_tile(uint16_t tx, uint8_t ty, uint8_t tile)
{
    if (tx >= WORLD_WIDTH_TILES || ty >= WORLD_HEIGHT_TILES || tile == TILE_EMPTY) {
        return false;
    }

    if (world_get_tile_or_empty(tx, ty) != TILE_EMPTY) {
        return false;
    }

    if (tile == TILE_DOOR ||
        tile == TILE_DOOR_CLOSED_BOTTOM ||
        tile == TILE_DOOR_CLOSED_TOP ||
        tile == TILE_DOOR_OPEN_BOTTOM ||
        tile == TILE_DOOR_OPEN_TOP) {
        if (!world_can_place_door(tx, ty)) {
            return false;
        }
    } else if (tile == TILE_TORCH) {
        if (!world_torch_has_support(tx, ty)) {
            return false;
        }
    } else if (tile == TILE_WORKBENCH) {
        if (!world_workbench_has_floor(tx, ty)) {
            return false;
        }
    } else if (!world_has_solid_neighbor(tx, ty)) {
        return false;
    }

    return true;
}

bool world_place_tile(uint16_t tx, uint8_t ty, uint8_t tile)
{
    if (!world_can_place_tile(tx, ty, tile)) {
        return false;
    }

    if (tile == TILE_DOOR ||
        tile == TILE_DOOR_CLOSED_BOTTOM ||
        tile == TILE_DOOR_CLOSED_TOP ||
        tile == TILE_DOOR_OPEN_BOTTOM ||
        tile == TILE_DOOR_OPEN_TOP) {
        return world_place_door(tx, ty);
    }

    world_set_tile(tx, ty, tile);
    return true;
}

bool world_place_door(uint16_t tx, uint8_t bottom_ty)
{
    if (!world_can_place_door(tx, bottom_ty)) {
        return false;
    }

    world_set_tile(tx, (uint8_t)(bottom_ty - 1u), TILE_DOOR_CLOSED_TOP);
    world_set_tile(tx, bottom_ty, TILE_DOOR_CLOSED_BOTTOM);
    return true;
}

bool world_toggle_door(uint16_t tx, uint8_t ty)
{
    uint8_t tile = world_get_tile_or_empty(tx, ty);
    uint8_t bottom_ty;
    uint8_t bottom_tile;

    if (!world_is_door_tile(tile)) {
        return false;
    }

    bottom_ty = world_door_bottom_ty(tile, ty);

    if (bottom_ty == 0u || bottom_ty >= WORLD_HEIGHT_TILES) {
        return false;
    }

    bottom_tile = world_get_tile_or_empty(tx, bottom_ty);

    if (bottom_tile == TILE_DOOR_CLOSED_BOTTOM) {
        world_set_tile(tx, (uint8_t)(bottom_ty - 1u), TILE_DOOR_OPEN_TOP);
        world_set_tile(tx, bottom_ty, TILE_DOOR_OPEN_BOTTOM);
        return true;
    }

    if (bottom_tile == TILE_DOOR_OPEN_BOTTOM) {
        world_set_tile(tx, (uint8_t)(bottom_ty - 1u), TILE_DOOR_CLOSED_TOP);
        world_set_tile(tx, bottom_ty, TILE_DOOR_CLOSED_BOTTOM);
        return true;
    }

    return false;
}

bool world_remove_multitile_at(uint16_t tx, uint8_t ty)
{
    uint8_t tile = world_get_tile_or_empty(tx, ty);
    uint8_t bottom_ty;

    if (!world_is_door_tile(tile)) {
        if (tile == TILE_EMPTY) {
            return false;
        }

        world_set_tile(tx, ty, TILE_EMPTY);
        return true;
    }

    bottom_ty = world_door_bottom_ty(tile, ty);

    if (bottom_ty == 0u || bottom_ty >= WORLD_HEIGHT_TILES) {
        return false;
    }

    world_set_tile(tx, (uint8_t)(bottom_ty - 1u), TILE_EMPTY);
    world_set_tile(tx, bottom_ty, TILE_EMPTY);
    return true;
}

bool world_has_tile_near_pixel(int16_t x, int16_t y, uint8_t tile, uint8_t radius)
{
    int8_t dx;
    int8_t dy;
    int16_t center_tx;
    int16_t center_ty;

    if (x < 0 || y < 0) {
        return false;
    }

    center_tx = x >> 3;
    center_ty = y >> 3;

    for (dy = (int8_t)-radius; dy <= (int8_t)radius; ++dy) {
        for (dx = (int8_t)-radius; dx <= (int8_t)radius; ++dx) {
            int16_t tx = center_tx + dx;
            int16_t ty = center_ty + dy;

            if (tx < 0 || ty < 0) {
                continue;
            }

            if (world_get_tile_or_empty((uint16_t)tx, (uint8_t)ty) == tile) {
                return true;
            }
        }
    }

    return false;
}
