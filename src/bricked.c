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
    /** START */
#ifdef DEBUG
    Tile tile;
    // Rect rect = {
    //     .x = 16,
    //     .y = 16,
    //     .w = 8,
    //     .h = 8,
    // };
    // bool found = tile_get(&rect, &tile);
    tile_at(0, 0, &tile);
    // printf("Found: %d\n", found);
    // printf("OX: %03d, OY: %03d\n", rect.x, rect.y);
    printf("TX: %03d, TY: %03d\n", tile.x, tile.y);
    printf("RX: %03d, RY: %03d\n", tile.rect.x, tile.rect.y);
    exit(2);
#endif
    /** END */

    init();

    Sound* sound = sound_play(0, 220, 0);
    msleep(75);
    sound_stop(sound);

    reset(true);

    load_level(0);


    uint8_t process_frame = 0;
    while(true) {
        sound_loop();
        uint8_t action = input();
        switch(action) {
            case ACTION_PAUSE:
                break;
            case ACTION_QUIT:
                goto quit_game;
        }

#ifdef FRAMELOCK
        bool force_process = controller_pressed(BUTTON_Y);
        if(force_process || controller_pressed(BUTTON_B)) {
            if(force_process || process_frame == 0) {
                process_frame = 1;
#endif
                frames++;

                TSTATE_LOG(1);
                update();
                TSTATE_LOG(1);
                draw();
#ifdef FRAMELOCK
            }
        } else {
            process_frame = 0;
        }
#endif

        if(level.brick_count == 0) {
            msleep(250);
            reset(false);
            load_level(level.index);
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
    err = keyboard_flush();
    if(err != ERR_SUCCESS) {
        printf("Failed to flush keyboard: %d\n", err);
        exit(1);
    }

    err = controller_init();
    if(err != ERR_SUCCESS) {
        printf("Failed to init controller: %d", err);
    }
    err = controller_flush();
    if(err != ERR_SUCCESS) {
        printf("Failed to flush controller: %d", err);
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

    ascii_map(' ', 1, EMPTY_TILE); // space
    ascii_map(',', 16, 160); // numbers and extras
    ascii_map('A', 26, 176); // A-Z to a-z
    ascii_map('a', 28, 176); // a-z + snes icon

    /** DEBUG */
    // gfx_sprite debug_tile = {
    //     .tile = BRICK25,
    //     .x = 16,
    //     .y = 16,
    // };
    // gfx_sprite_render(&vctx, 96, &debug_tile);
    // uint8_t tiles[1] = { BRICK26+1 };
    // gfx_tilemap_load(&vctx, tiles, sizeof(tiles), UI_LAYER, 1, 0);
    /** /DEBUG */

    sound_set(0, WAV_SAWTOOTH);
    sound_set(1, WAV_SQUARE);

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
    level.index = which + 1;

    which %= LEVEL_COUNT;


    /** debug */
    char text[10];
    sprintf(text, "%02d", level.index);
    nprint_string(&vctx, text, strlen(text), WIDTH - 2, 0);
    /** /debug */

    uint8_t line[LEVEL_WIDTH*2];
    for(uint8_t y = 0; y < LEVEL_HEIGHT; y++) {
        for(uint8_t x = 0; x < LEVEL_WIDTH; x++) {
            uint8_t offset = (y * LEVEL_WIDTH) + x;
            uint8_t type = LEVEL_TILES[which][offset];

            uint8_t tilex = x * 2;
            Brick *brick = &level.bricks[(y * LEVEL_WIDTH) + x];
            brick->x = tilex;
            brick->y = y;
            if(type > 0) {
                // memcpy(brick, &BRICKS[type-1], sizeof(Brick));
                brick->health = BRICKS[type-1].health;
                brick->points = brick->health;
                brick->l = BRICKS[type-1].l;
                brick->r = BRICKS[type-1].r;
                level.brick_count++;
            } else {
                brick->health = 0;
                brick->points = 0;
                brick->l = EMPTY_TILE;
                brick->r = EMPTY_TILE;
            }
            line[tilex] = brick->l;
            line[tilex+1] = brick->r;
        }
        gfx_tilemap_load(&vctx, line, LEVEL_WIDTH*2, LEVEL_LAYER, 0, y);
    }

    sprintf(text, "%02d", level.brick_count);
    nprint_string(&vctx, text, strlen(text), WIDTH - 2, 1);

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


    player.direction.x = DIRECTION_NONE; // not moving
    if(input & BUTTON_LEFT) player.direction.x = DIRECTION_LEFT;
    if(input & BUTTON_RIGHT) player.direction.x = DIRECTION_RIGHT;
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

    Edge edge = EdgeNone;
    char text[10];

    // sprintf(text, "PX%03d", player.rect.x);
    // nprint_string(&vctx, text, 5, 0, HEIGHT - 3);
    // sprintf(text, "PY%03d", player.rect.y);
    // nprint_string(&vctx, text, 5, 6, HEIGHT - 3);
    // sprintf(text, "BX%03d", ball.rect.x);
    // nprint_string(&vctx, text, 5, 0, HEIGHT - 4);
    // sprintf(text, "BY%03d", ball.rect.y);
    // nprint_string(&vctx, text, 5, 6, HEIGHT - 4);

    edge = player_collide(&ball.rect);
    if(edge != EdgeNone) {
        sound_play(0, 400, 4);
        switch(edge) {
            // uno reverse
            case EdgeLeft:
                ball_bounce(edge | EdgeTop);
                break;
            case EdgeRight:
                ball_bounce(edge | EdgeTop);
                break;
            case EdgeTop: // fall thru
            case EdgeBottom:
                ball_bounce(edge);
                break;
        }
        // If collides with player, can't possibly collide with bricks ... if/else statement?
        return;
    }


    // /** debug */
    // if(edge != EdgeNone) {
    //     sprintf(text, "PE%02d", edge);
    //     nprint_string(&vctx, text, strlen(text), WIDTH - 4, HEIGHT - 4);
    // }
    // /** /debug */

    // get the balls current tile coords
    Tile tile;
    // tile.rect.h = BRICK_HEIGHT;
    // tile.rect.w = BRICK_WIDTH;
    edge = tile_collide(&ball.rect, &ball.direction, &tile);

    /** debug */
    // sprintf(text, "%03d", ball.sprite.x);
    // nprint_string(&vctx, text, strlen(text), WIDTH - 7, HEIGHT - 2);
    // sprintf(text, "%03d", ball.sprite.y);
    // nprint_string(&vctx, text, strlen(text), WIDTH - 3, HEIGHT - 2);

    // sprintf(text, "%02d", player.direction.x);
    // nprint_string(&vctx, text, strlen(text), WIDTH - 6, HEIGHT - 3);
    // sprintf(text, "%02d", player.width);
    // nprint_string(&vctx, text, strlen(text), WIDTH - 2, HEIGHT - 3);
    /** /debug */

    // /** debug */
    // sprintf(text, "RX%03d", tile.rect.x);
    // nprint_string(&vctx, text, 5, 0, HEIGHT - 4);
    // sprintf(text, "RY%03d", tile.rect.y);
    // nprint_string(&vctx, text, 5, 8, HEIGHT - 4);
    // sprintf(text, "RW%03d", tile.rect.w);
    // nprint_string(&vctx, text, 5, 0, HEIGHT - 4);
    // sprintf(text, "RH%03d", tile.rect.h);
    // nprint_string(&vctx, text, 5, 6, HEIGHT - 4);
    // sprintf(text, "TX%03d", tile.x);
    // nprint_string(&vctx, text, 5, 0, HEIGHT - 5);
    // sprintf(text, "TY%03d", tile.y);
    // nprint_string(&vctx, text, 5, 6, HEIGHT - 5);
    // /** /debug */

    uint16_t brick_offset = (tile.y * LEVEL_WIDTH) + (tile.x >> 1);
    if(brick_offset > LEVEL_TILE_COUNT) return;

    Brick *brick = &level.bricks[brick_offset];
    // sprintf(text, "BX%03d", brick->x);
    // nprint_string(&vctx, text, 5, 0, HEIGHT - 1);
    // sprintf(text, "BY%03d", brick->y);
    // nprint_string(&vctx, text, 5, 6, HEIGHT - 1);

    if(brick->health < 1) return;

    sound_play(1, 220, 2);

    /** DEBUG */
    // sprintf(text, "EB%02d", edge);
    // nprint_string(&vctx, text, 4, 0, HEIGHT - 4);
    /** /DEBUG */

    uint8_t mod = tile.x % 2;
    if(mod == 1) edge &= (0xFF ^ EdgeLeft);  // remove the left edge from the right tile
    else edge &= (0xFF ^ EdgeRight);         // remove the right edge from the left tile

    /** DEBUG */
    // sprintf(text, "EA%02d", edge);
    // nprint_string(&vctx, text, 4, 0, HEIGHT - 3);
    // sprintf(text, "MO%02d", mod);
    // nprint_string(&vctx, text, 4, 6, HEIGHT - 3);
    /** /DEBUG */

    if(edge == EdgeNone) return;


    // move the ball so it doesn't go "inside" the brick
    if(edge & EdgeTop > 0) {
        ball.sprite.y = (brick_py(brick->y) - TILE_HEIGHT);
        ball.rect.y = ball.sprite.y;
    } else if(edge & EdgeBottom > 0) {
        ball.sprite.y = brick_py(brick->y) + BRICK_HEIGHT;
        ball.rect.y = ball.sprite.y;
    }

    if(edge & EdgeLeft > 0) {
        ball.sprite.x = (brick_px(brick->x) - TILE_WIDTH);
        ball.rect.x = ball.sprite.x;
    } else if(edge & EdgeRight > 0) {
        ball.sprite.x = brick_px(brick->x) + BRICK_WIDTH;
        ball.rect.x = ball.sprite.x;
    }

    // sprintf(text, "CX%03d", ball.sprite.x);
    // nprint_string(&vctx, text, 5, 0, HEIGHT - 2);
    // sprintf(text, "CY%03d", ball.sprite.y);
    // nprint_string(&vctx, text, 5, 6, HEIGHT - 2);

    ball_bounce(edge);

    // if(edge == EdgeTop || edge == EdgeBottom) {
    //     msleep(75);
    //     sound_stop_all();
    //     while(1) {}
    // }

    // sprintf(text, "MO%01d", mod);
    // nprint_string(&vctx, text, 4, WIDTH - 3, HEIGHT - 3);
    /** /DEBUG */

    // TODO: debug, uncomment to remove bricks
    // TODO: debug, uncomment to remove bricks
    brick->health--;
    if(brick->health < 1) {
        // remove brick
        level.brick_count--;
        player.score += brick->points; // TODO: point modifier???
        sprintf(text, "%03d", player.score);
        nprint_string(&vctx, text, strlen(text), 0, 0);

        /** DEBUG */
        sprintf(text, "%02d", level.brick_count);
        nprint_string(&vctx, text, strlen(text), WIDTH - 2, 1);
        /** /DEBUG */

        gfx_tilemap_place(&vctx, EMPTY_TILE, LEVEL_LAYER, brick->x, brick->y);
        gfx_tilemap_place(&vctx, EMPTY_TILE, LEVEL_LAYER, brick->x+1, brick->y);
    }
}

void draw(void) {
    gfx_wait_vblank(&vctx);
    player_draw();
    ball_draw();
    gfx_wait_end_vblank(&vctx);
}

