#ifndef RENDER_H
#define RENDER_H

#include "camera.h"
#include "enemy.h"
#include "inventory.h"
#include "item_drop.h"
#include "player.h"

void render_init(void);
void render_world(const Camera *camera);
void render_hud(const Inventory *inventory, const Player *player);
void render_menu(const Inventory *inventory, bool near_workbench, uint8_t craft_error_timer);
void render_frame(const Camera *camera, const Player *player, const Enemy *enemies, const ItemDrop *drops);

#endif
