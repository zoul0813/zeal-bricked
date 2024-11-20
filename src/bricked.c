#include <stdio.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <zos_errors.h>
#include <zvb_gfx.h>
#include <zvb_hardware.h>
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
static uint8_t paused = 0;

// #define BREAK
// #define DEBUG
// #define FRAMELOCK
#ifdef DEBUG
uint8_t DEBUG_TILE_INDEX = 100;
gfx_sprite DEBUG_TILE;
#endif

int main(void) {
    init();
reset:
    player.lives = PLAYER_LIVES;
    draw_gameover(false);
    reset();

    Sound* sound = sound_play(0, 220, 0);
    msleep(75);
    sound_stop(sound);

    load_level(0);

    uint8_t process_frame = 0;
    while(true) {
        sound_loop();
        uint8_t action = input();
        switch(action) {
            case ACTION_NONE:
                // debounce the pause button
                if(paused == 1) {
                    paused = 2; // released pause
                    draw_paused(true);
                }
                if(paused == 3) {
                    paused = 0; // released unpause
                    draw_paused(false);
                }
                break;
            case ACTION_PAUSE:
                if(paused == 0) paused = 1; // requested pause
                if(paused == 2) paused = 3; // requested unpause
                break;
            case ACTION_QUIT:
                goto quit_game;
        }

        if(paused > 0) continue;

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
            msleep(10);
            sound_stop_all();
            msleep(250);
            reset();
            load_level(level.index);
            msleep(250);
        }

        if(player.lives < 1) {
            msleep(10);
            sound_stop_all();
            draw_gameover(true);
            msleep(1000);
            goto reset;
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

#ifdef DEBUG
    DEBUG_TILE.tile = 79;
    DEBUG_TILE.x = SCREEN_HEIGHT / 2;
    DEBUG_TILE.y = SCREEN_WIDTH / 2;
#endif

    ascii_map(' ', 1, EMPTY_TILE); // space
    ascii_map(',', 16, 160); // numbers and extras
    ascii_map('A', 26, 176); // A-Z to a-z
    ascii_map('a', 28, 176); // a-z + snes icon

    sound_set(0, WAV_SAWTOOTH);
    sound_set(1, WAV_SQUARE);

    err = player_init();
    if(err) exit(1);

    err = ball_init();
    if(err) exit(1);

    gfx_enable_screen(1);

    sound_init();
}

void reset(void) {
    player_reset();
    ball_reset();
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

#ifdef DEBUG
    sprintf(text, "%02d", level.brick_count);
    nprint_string(&vctx, text, strlen(text), WIDTH - 2, 1);
#endif

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
    if((input & BUTTON_LEFT)) player.direction.x = DIRECTION_LEFT;
    if((input & BUTTON_RIGHT)) player.direction.x = DIRECTION_RIGHT;
    if((input & BUTTON_START)) return ACTION_PAUSE;
    if((input & BUTTON_SELECT)) return ACTION_QUIT;


    // /**** TESTING ****/
    // if(input & BUTTON_UP && player.width < PLAYER_MAX_WIDTH) player_set_width(player.width+1);
    // if(input & BUTTON_DOWN && player.width > 1) player_set_width(player.width-1);
    // /**** TESTING ****/

    return 0;
}

void update(void) {
    player_move();
    ball_move();

    uint16_t ball_top = rect_top(&ball.rect);
    if(ball_top > SCREEN_HEIGHT + SPRITE_HEIGHT) {
        sound_play(1, 294, 3);
        player.lives--;
        msleep(750);
        ball_reset();
        return;
    }

    Edge edge = EdgeNone;
    char text[10];

#ifdef DEBUG
    sprintf(text, "W%03d", ball.rect.w);
    nprint_string(&vctx, text, 5, 0, HEIGHT - 3);
    sprintf(text, "H%03d", ball.rect.h);
    nprint_string(&vctx, text, 5, 5, HEIGHT - 3);
    sprintf(text, "X%03d", ball.rect.x);
    nprint_string(&vctx, text, 5, 0, HEIGHT - 4);
    sprintf(text, "Y%03d", ball.rect.y);
    nprint_string(&vctx, text, 5, 5, HEIGHT - 4);
#endif

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

    // get the balls current tile coords
    Tile tile;
    // tile.rect.h = BRICK_HEIGHT;
    // tile.rect.w = BRICK_WIDTH;
    edge = tile_collide(&ball.rect, &ball.direction, &tile);

#ifdef DEBUG
    DEBUG_TILE.x = tile.rect.x;
    DEBUG_TILE.y = tile.rect.y;
#endif

    /** debug */
#ifdef DEBUG
    sprintf(text, "W%03d", tile.rect.w);
    nprint_string(&vctx, text, strlen(text), WIDTH - 9, HEIGHT - 3);
    sprintf(text, "H%03d", tile.rect.h);
    nprint_string(&vctx, text, strlen(text), WIDTH - 4, HEIGHT - 3);
    sprintf(text, "X%03d", tile.rect.x);
    nprint_string(&vctx, text, strlen(text), WIDTH - 9, HEIGHT - 4);
    sprintf(text, "Y%03d", tile.rect.y);
    nprint_string(&vctx, text, strlen(text), WIDTH - 4, HEIGHT - 4);
#endif
    /** /debug */

    uint16_t brick_offset = (tile.y * LEVEL_WIDTH) + (tile.x >> 1);
    if(brick_offset > LEVEL_TILE_COUNT) return;

    Brick *brick = &level.bricks[brick_offset];

    if(brick->health < 1) return;

    sound_play(1, 220, 2);

    /** DEBUG */
#ifdef DEBUG
    sprintf(text, "EB%02d", edge);
    nprint_string(&vctx, text, 4, 0, HEIGHT - 5);
#endif
    /** /DEBUG */

    uint8_t mod = tile.x % 2;
    if(mod == 1) edge &= (0xFF ^ EdgeLeft);  // remove the left edge from the right tile
    else edge &= (0xFF ^ EdgeRight);         // remove the right edge from the left tile

    /** DEBUG */
#ifdef DEBUG
    sprintf(text, "EA%02d", edge);
    nprint_string(&vctx, text, 4, 0, HEIGHT - 6);
#endif
    /** /DEBUG */

    if(edge == EdgeNone) return;


    // move the ball so it doesn't go "inside" the brick
    if((edge & EdgeTop) > 0) {
        ball.rect.y = rect_top(&tile.rect) - 1;
        ball.sprite.y = ball.rect.y;
    }
    else if((edge & EdgeBottom) > 0) {
        ball.sprite.y = rect_bottom(&tile.rect) + ball.rect.h + 1;
         ball.sprite.y = ball.sprite.y;
    }
    else if((edge & EdgeRight) > 0) {
        ball.rect.x = rect_right(&tile.rect) + ball.rect.w + 1;
        ball.sprite.x = ball.rect.x;
    }
    else if((edge & EdgeLeft) > 0) {
        ball.rect.x = rect_left(&tile.rect) - 1;
        ball.sprite.x = ball.rect.x;
    }

    ball_bounce(edge);

    brick->health--;
    if(brick->health < 1) {
        // remove brick
        level.brick_count--;
        player.score += brick->points; // TODO: point modifier???
        sprintf(text, "%03d", player.score);
        nprint_string(&vctx, text, strlen(text), 0, 0);

        gfx_tilemap_place(&vctx, EMPTY_TILE, LEVEL_LAYER, brick->x, brick->y);
        gfx_tilemap_place(&vctx, EMPTY_TILE, LEVEL_LAYER, brick->x+1, brick->y);
    }
}

void draw(void) {
    gfx_wait_vblank(&vctx);
    player_draw();
    ball_draw();

#ifdef DEBUG
    gfx_sprite_render(&vctx, DEBUG_TILE_INDEX, &DEBUG_TILE);
#endif

    gfx_wait_end_vblank(&vctx);
}

void draw_paused(uint8_t paused) {
    char text[6];
    if(paused) sprintf(text, "PAUSED");
    else sprintf(text, "      ");
    nprint_string(&vctx, text, 6, WIDTH / 2 - 3, HEIGHT / 2);
}

void draw_gameover(uint8_t gameover) {
    char text[10];
    if(gameover) sprintf(text, "GAME  OVER");
    else sprintf(text, "          ");
    nprint_string(&vctx, text, 10, WIDTH / 2 - 5, HEIGHT / 2);
}