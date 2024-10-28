#include <stdio.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <zos_errors.h>
#include <zvb_gfx.h>
#include <zgdk.h>

#include "assets.h"
#include "bricked.h"

#include "ball.h"
#include "player.h"
#include "bricks.h"
#include "levels.h"

gfx_context vctx;
static uint8_t controller_mode = 1;
Level level;

uint16_t frames = 0;

int main(void) {
    init();

    reset(true);

    load_level(0);

    while(true) {
        sound_loop();
        uint8_t action = input();
        switch(action) {
            case ACTION_PAUSE:
                break;
            case ACTION_QUIT:
                goto quit_game;
        }

        frames++;

        update();
        draw();

        if(level.brick_count == 0) {
            msleep(250);
            reset(false);
            load_level(++level.index);
            msleep(250);
        }
    }

quit_game:
    deinit();

    return 0;
}

void init(void) {
    zos_err_t err;
    err = keyboard_init();
    if(err != ERR_SUCCESS) {
        printf("Failed to init keyboard: %d\n", err);
        exit(1);
    }
    err = controller_init();
    if(err != ERR_SUCCESS) {
        printf("Failed to init controller: %d", err);
    }
    // verify the controller is actually connected
    uint16_t test = controller_read();
    // if unconnected, we'll get back 0xFFFF (all buttons pressed)
    if(test & 0xFFFF) {
        controller_mode = 0;
    }

    /* Disable the screen to prevent artifacts from showing */
    gfx_enable_screen(0);

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    if (err) exit(1);

    err = load_palette(&vctx);
    if(err) exit(1);

    gfx_tileset_options options = {
        .compression = TILESET_COMP_RLE,
    };

    err = load_tiles(&vctx, &options);
    if (err) exit(1);

    ascii_map(',', 16, 160);
    ascii_map('a', 28, 176);

    gfx_enable_screen(1);

    sound_init();
}

void reset(uint8_t player_reset) {
    zos_err_t err;
    err = player_init(player_reset);
    if(err) exit(1);

    err = ball_init(player_reset);
    if(err) exit(1);
}

error load_level(uint8_t which) {
    if(which > LEVEL_COUNT) return 1;

    level.index = which;

    uint8_t line[LEVEL_WIDTH*2];
    const uint8_t width = LEVEL_WIDTH;
    for(uint8_t y = 0; y < LEVEL_HEIGHT; y++) {
        for(uint8_t x = 0; x < width; x++) {
            uint8_t offset = (y * width) + x;
            uint8_t type = LEVEL_TILES[which][offset];

            uint8_t tilex = x * 2;
            Brick *brick = &level.bricks[(y * LEVEL_WIDTH) + x];
            if(type > 0) {
                brick->health = BRICKS[type-1].health;
                brick->l = BRICKS[type-1].l;
                brick->r = BRICKS[type-1].r;
                brick->x = tilex;
                brick->y = y;
                level.brick_count++;
            } else {
                brick->health = 0;
                brick->l = EMPTY_TILE;
                brick->r = EMPTY_TILE;
                brick->x = 0;
                brick->y = 0;
            }
            // level.tiles[(y * LEVEL_WIDTH) + x] = brick;
            line[tilex] = brick->l;
            line[tilex+1] = brick->r;
        }
        gfx_tilemap_load(&vctx, line, LEVEL_WIDTH*2, LEVEL_LAYER, 0, y);
    }

    return 0;
}

void deinit(void) {
    zvb_ctrl_l0_scr_x_low = 0;
    zvb_ctrl_l0_scr_x_high = 0;
    zvb_ctrl_l0_scr_y_low = 0;
    zvb_ctrl_l0_scr_y_high = 0;
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    sound_deinit();

    // TODO: clear tilesets

    // TODO: clear sprites
    gfx_error err;
    for(uint8_t i = 0; i < PLAYER_MAX_WIDTH + 2; i++) {
        err = gfx_sprite_set_tile(&vctx, player.sprite_index+i, EMPTY_TILE);
    }
    err = gfx_sprite_set_tile(&vctx, ball.sprite_index, EMPTY_TILE);
    // // TODO: error checking

}

uint8_t input(void) {
    uint16_t input = keyboard_read();
    if(controller_mode == 1) {
        input |= controller_read();
    }


    player.direction = 0; // not moving
    if(input & BUTTON_LEFT) player.direction = DIRECTION_LEFT;
    if(input & BUTTON_RIGHT) player.direction = DIRECTION_RIGHT;
    if(input & BUTTON_START ) return ACTION_PAUSE;
    if(input & (BUTTON_START | BUTTON_SELECT) ) return ACTION_QUIT;


    /**** TESTING ****/
    if(input & BUTTON_UP && player.width < PLAYER_MAX_WIDTH) player_set_width(player.width+1);
    if(input & BUTTON_DOWN && player.width > 1) player_set_width(player.width-1);
    /**** TESTING ****/

    return 0;
}

void update(void) {
    player_move();
    ball_move();

    if(player_collide(ball.x, ball.y, BALL_OFFSET)) {
        ball_bounce(0,-1);
    }

    // get the balls current tile coords
    uint16_t ballx = ball.x;
    uint16_t bally = ball.y;
    tile_get(&ballx, &bally);
    uint16_t brick_offset = (bally * LEVEL_WIDTH) + (ballx/2);
    Brick *brick = &level.bricks[brick_offset];
    if(brick->health > 0) {
        brick->health--;
        ball_bounce(0,-1);
        if(brick->health == 0) {
            // remove brick
            level.brick_count--;
            gfx_tilemap_place(&vctx, EMPTY_TILE, LEVEL_LAYER, brick->x, brick->y);
            gfx_tilemap_place(&vctx, EMPTY_TILE, LEVEL_LAYER, brick->x+1, brick->y);
        }
    }
}

void draw(void) {
    gfx_wait_vblank(&vctx);

    player_draw();
    ball_draw();


    uint16_t ballx = ball.x;
    uint16_t bally = ball.y;
    tile_get(&ballx, &bally);
    char text[10];
    sprintf(text, "%02d", ballx);
    nprint_string(&vctx, text, strlen(text), WIDTH - 3, HEIGHT - 1);
    sprintf(text, "%02d", bally);
    nprint_string(&vctx, text, strlen(text), WIDTH - 6, HEIGHT - 1);

    gfx_wait_end_vblank(&vctx);
}

