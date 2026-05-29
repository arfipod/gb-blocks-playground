#include <gb/gb.h>
#include "game.h"
#include "splash.h"

void main(void)
{
    splash_show();
    game_init();

    while (1) {
        game_update_logic();
        wait_vbl_done();
        game_render_commit();
    }
}
