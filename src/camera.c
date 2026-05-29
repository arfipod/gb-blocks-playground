#include "camera.h"
#include "world.h"

#define SCREEN_W 160u

void camera_update(Camera *camera, const Player *player)
{
    int16_t desired = (int16_t)(player->x - 72);
    uint16_t max_x = 0u;

    if (WORLD_WIDTH_PIXELS > SCREEN_W) {
        max_x = (uint16_t)(WORLD_WIDTH_PIXELS - SCREEN_W);
    }

    if (desired < 0) {
        camera->x = 0u;
    } else if ((uint16_t)desired > max_x) {
        camera->x = max_x;
    } else {
        camera->x = (uint16_t)desired;
    }

    camera->y = 0u;
}
