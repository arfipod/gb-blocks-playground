#include "enemy.h"
#include "collision.h"
#include "world.h"

#define ENEMY_DESPAWN_MARGIN 48
#define ENEMY_SPAWN_COOLDOWN 96u
#define ENEMY_GRAVITY 1
#define ENEMY_MAX_FALL_SPEED 3

static uint8_t spawn_cooldown;
static uint8_t spawn_phase;
static uint8_t enemy_update_frame;
static uint8_t enemy_deferred_slot;

static void advance_enemy_frame(void)
{
    ++enemy_update_frame;
    ++enemy_deferred_slot;

    if (enemy_deferred_slot >= ENEMY_MAX_ACTIVE) {
        enemy_deferred_slot = 0u;
    }
}

static bool enemy_aabb_solid_at(int16_t x, int16_t y)
{
    return collision_aabb_solid(x, y, ENEMY_WIDTH, ENEMY_HEIGHT);
}

static bool enemy_has_floor(const Enemy *enemy)
{
    int16_t foot_y = (int16_t)(enemy->y + ENEMY_HEIGHT);

    return world_is_solid_at_pixel(enemy->x, foot_y) ||
           world_is_solid_at_pixel((int16_t)(enemy->x + ENEMY_WIDTH - 1), foot_y);
}

static uint8_t enemy_count(const Enemy *enemies)
{
    uint8_t i;
    uint8_t count = 0u;

    for (i = 0u; i < ENEMY_MAX_ACTIVE; ++i) {
        if (enemies[i].active) {
            ++count;
        }
    }

    return count;
}

static int8_t enemy_direction_for_slot(uint8_t slot)
{
    return (slot & 1u) ? -1 : 1;
}

static uint8_t spawn_enemy(Enemy *enemies, const Camera *camera)
{
    uint8_t i;
    uint16_t spawn_px;
    uint16_t tx;

    if ((uint16_t)(camera->x + 152u) < (uint16_t)(WORLD_WIDTH_PIXELS - ENEMY_WIDTH)) {
        spawn_px = (uint16_t)(camera->x + 152u + ((spawn_phase & 1u) * 24u));
    } else if (camera->x > 32u) {
        spawn_px = (uint16_t)(camera->x - 32u);
    } else {
        return ENEMY_MAX_ACTIVE;
    }

    tx = (uint16_t)(spawn_px >> 3);

    for (i = 0u; i < ENEMY_MAX_ACTIVE; ++i) {
        if (!enemies[i].active &&
            world_find_surface_spawn(tx, &enemies[i].x, &enemies[i].y)) {
            enemies[i].vx = enemy_direction_for_slot((uint8_t)(i + spawn_phase));
            enemies[i].vy = 0;
            enemies[i].active = 1u;
            enemies[i].grounded = 0u;
            ++spawn_phase;
            return i;
        }
    }

    return ENEMY_MAX_ACTIVE;
}

static bool enemy_far_from_camera(const Enemy *enemy, const Camera *camera)
{
    int16_t left = camera->x > ENEMY_DESPAWN_MARGIN ?
                   (int16_t)(camera->x - ENEMY_DESPAWN_MARGIN) :
                   0;
    int16_t right = (int16_t)(camera->x + 160u + ENEMY_DESPAWN_MARGIN);

    return enemy->x < left || enemy->x > right;
}

static void move_enemy_h(Enemy *enemy)
{
    int16_t next_x = (int16_t)(enemy->x + enemy->vx);
    int16_t edge_x = enemy->vx > 0 ? (int16_t)(enemy->x + ENEMY_WIDTH) :
                                      (int16_t)(enemy->x - 1);
    int16_t foot_y = (int16_t)(enemy->y + ENEMY_HEIGHT);

    if (next_x < 0 ||
        next_x > (int16_t)(WORLD_WIDTH_PIXELS - ENEMY_WIDTH) ||
        enemy_aabb_solid_at(next_x, enemy->y) ||
        !world_is_solid_at_pixel(edge_x, foot_y)) {
        enemy->vx = (int8_t)-enemy->vx;
        return;
    }

    enemy->x = next_x;
}

static void move_enemy_v(Enemy *enemy)
{
    int8_t remaining = enemy->vy;
    int8_t step;

    while (remaining != 0) {
        step = remaining > 0 ? 1 : -1;

        if (enemy_aabb_solid_at(enemy->x, (int16_t)(enemy->y + step))) {
            if (step > 0) {
                enemy->grounded = 1u;
            }

            enemy->vy = 0;
            return;
        }

        enemy->y = (int16_t)(enemy->y + step);
        remaining = (int8_t)(remaining - step);
    }
}

void enemies_init(Enemy *enemies)
{
    uint8_t i;

    for (i = 0u; i < ENEMY_MAX_ACTIVE; ++i) {
        enemies[i].active = 0u;
        enemies[i].x = 0;
        enemies[i].y = 0;
        enemies[i].vx = 1;
        enemies[i].vy = 0;
        enemies[i].grounded = 0u;
    }

    spawn_cooldown = 30u;
    spawn_phase = 0u;
    enemy_update_frame = 0u;
    enemy_deferred_slot = 0u;
}

void enemies_update(Enemy *enemies, const Camera *camera, Player *player)
{
    uint8_t i;
    uint8_t active_count = enemy_count(enemies);
    uint8_t deferred_slot = enemy_deferred_slot;

    if (spawn_cooldown != 0u) {
        --spawn_cooldown;
    } else if (active_count < ENEMY_MAX_ACTIVE) {
        uint8_t spawned_slot = spawn_enemy(enemies, camera);
        spawn_cooldown = ENEMY_SPAWN_COOLDOWN;

        if (spawned_slot < ENEMY_MAX_ACTIVE) {
            advance_enemy_frame();
            return;
        }
    }

    for (i = 0u; i < ENEMY_MAX_ACTIVE; ++i) {
        Enemy *enemy = &enemies[i];

        if (!enemy->active) {
            continue;
        }

        if (enemy_far_from_camera(enemy, camera)) {
            enemy->active = 0u;
            continue;
        }

        if (active_count >= ENEMY_MAX_ACTIVE && i == deferred_slot) {
            continue;
        }

        move_enemy_h(enemy);

        if (enemy->grounded) {
            if ((((uint8_t)(enemy_update_frame + i)) & 3u) == 0u &&
                !enemy_has_floor(enemy)) {
                enemy->grounded = 0u;
            }
        }

        if (!enemy->grounded) {
            if (enemy->vy < ENEMY_MAX_FALL_SPEED) {
                enemy->vy = (int8_t)(enemy->vy + ENEMY_GRAVITY);
            }

            move_enemy_v(enemy);
        }

        if (player_overlaps_aabb(player, enemy->x, enemy->y, ENEMY_WIDTH, ENEMY_HEIGHT)) {
            player_damage(player, enemy->x < player->x ? 2 : -2);
            enemy->vx = (int8_t)-enemy->vx;
        }
    }

    advance_enemy_frame();
}
