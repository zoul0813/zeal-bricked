#include <core.h>
#include <zos_errors.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <zvb_gfx.h>
#include <zvb_hardware.h>

#define MAX_RECORDS 2048

#include <zgdk.h>

#include "assets.h"
#include "ball.h"
#include "bricked.h"
#include "bricks.h"
#include "levels.h"
#include "player.h"

gfx_context vctx;
static Level level;
gfx_sprite SPRITE_ARENA[MAX_SPRITES];

// static uint16_t frames = 0;
static uint8_t paused   = 0;
static uint8_t launched = 0;
static uint8_t nudge    = 0; // nudge counter
// static Track track;
pattern_t pattern0;
pattern_t pattern1;
pattern_t pattern2;
pattern_t pattern3;
pattern_t pattern4;
pattern_t pattern5;
pattern_t pattern6;
pattern_t pattern7;
track_t track = {
    .title    = "Track 1",
    .patterns = {
                 &pattern0,
                 &pattern1,
                 &pattern2,
                 &pattern3,
                 &pattern4,
                 &pattern5,
                 &pattern6,
                 &pattern7,
                 }
};
const size_t TRACK_SIZE = sizeof(track);

// #define BREAK
// #define DEBUG
// #define FRAMELOCK
#ifdef DEBUG
uint8_t DEBUG_TILE_INDEX = 100;
gfx_sprite DEBUG_TILE;
#endif

void handle_error(zos_err_t err, const char* message, uint8_t fatal)
{
    if (err != ERR_SUCCESS) {
        if (fatal)
            deinit();
        put_s("\nError[");
        put_hex8(err);
        put_s("] (");
        put_hex8(err);
        put_s(") ");
        put_s(message);
        if (fatal)
            exit(err);
    }
}


int main(void)
{
    init();
reset:
    player.lives = PLAYER_LIVES;
    draw_gameover(false);
    reset();

    Sound* sound = sound_play(VOICE3, 220, 0);
    msleep(75);
    sound_stop(sound);

    load_level(0);

    uint8_t process_frame = 0;
    // music_transport(T_PLAY, 0);

    while (true) {
        sound_loop();
        // if(music_state() != T_PLAY) music_transport(T_PLAY, 0);
        // music_loop(1);
        if (!paused)
            zmt_tick(&track, 1);
        uint8_t action = input();
        switch (action) {
            case ACTION_NONE:
                // debounce the pause button
                if (paused == 1) {
                    paused = 2; // released pause
                    draw_paused(true);
                }
                if (paused == 3) {
                    paused = 0; // released unpause
                    draw_paused(false);
                }
                break;
            case ACTION_PAUSE:
                if (paused == 0) {
                    paused = 1; // requested pause
                    zmt_sound_off();
                }
                if (paused == 2)
                    paused = 3; // requested unpause
                break;
            case ACTION_QUIT: goto quit_game;
        }

        if (paused > 0)
            continue;

#ifdef FRAMELOCK
        bool force_process = controller_pressed(BUTTON_Y);
        if (force_process || controller_pressed(BUTTON_X)) {
            if (force_process || process_frame == 0) {
                process_frame = 1;
#endif
                // frames++;

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

        if (level.brick_count == 0) {
            msleep(10);
            sound_stop_all();
            msleep(250);
            reset();
            load_level(level.index);
            msleep(250);
        }

        if (player.lives < 1) {
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

void init(void)
{
    zos_err_t err;

    err = input_init(true);
    handle_error(err, "Failed to init input", 1);

    sprites_register_arena(SPRITE_ARENA, MAX_SPRITES);

    /* Disable the screen to prevent artifacts from showing */
    gfx_enable_screen(0);

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    handle_error(err, "Failed to init graphics", 1);

    tilemap_fill(&vctx, LAYER1, EMPTY_TILE, 0, 0, WIDTH, HEIGHT);

    err = load_palette(&vctx);
    handle_error(err, "Failed to load palette", 1);

    err = load_tiles(&vctx);
    handle_error(err, "Failed to load tiles", 1);

#ifdef DEBUG
    DEBUG_TILE.tile = 79;
    DEBUG_TILE.x    = SCREEN_HEIGHT / 2;
    DEBUG_TILE.y    = SCREEN_WIDTH / 2;
#endif

    ascii_map(' ', 1, EMPTY_TILE); // space
    ascii_map(',', 16, 48);        // numbers and extras
    ascii_map('A', 26, 64);        // A-Z to a-z
    ascii_map('a', 28, 64);        // a-z + snes icon

    sound_set(0, WAV_SAWTOOTH);
    sound_set(1, WAV_SQUARE);

    err = player_init();
    handle_error(err, "Failed to init player", 1);

    err = ball_init();
    handle_error(err, "Failed to init ball", 1);

    gfx_enable_screen(1);

    sound_init();
}

void reset(void)
{
    player_reset();
    ball_reset();
}

error load_level(uint8_t which)
{
    level.index = which + 1;
    level.brick_count = 0;
    launched    = 0;

    which %= LEVEL_COUNT;

    /** debug */
    char text[10];
    itoa_pad(level.index, text, 10, 'A', '0', 2);
    nprint_string(&vctx, text, 2, WIDTH - 2, 0);
    /** /debug */

    uint8_t line[LEVEL_WIDTH * 2];
    for (uint8_t y = 0; y < LEVEL_HEIGHT; y++) {
        for (uint8_t x = 0; x < LEVEL_WIDTH; x++) {
            uint8_t offset = (y * LEVEL_WIDTH) + x;
            uint8_t type   = LEVEL_TILES[which][offset];

            uint8_t tilex = x * 2;
            Brick* brick  = &level.bricks[(y * LEVEL_WIDTH) + x];
            brick->x      = tilex;
            brick->y      = y;
            if (type > 0) {
                // memcpy(brick, &BRICKS[type-1], sizeof(Brick));
                brick->health = BRICKS[type - 1].health;
                brick->points = brick->health;
                brick->l      = BRICKS[type - 1].l;
                brick->r      = BRICKS[type - 1].r;
                level.brick_count++;
            } else {
                brick->health = 0;
                brick->points = 0;
                brick->l      = EMPTY_TILE;
                brick->r      = EMPTY_TILE;
            }
            line[tilex]     = brick->l;
            line[tilex + 1] = brick->r;
        }
        gfx_tilemap_load(&vctx, line, LEVEL_WIDTH * 2, LAYER0, 0, y);
    }

#ifdef DEBUG
    itoa_pad(level.brick_count, text, 10, 'A', '0', 2);
    nprint_string(&vctx, text, 2, WIDTH - 2, 1);
#endif

    uint8_t track_index = which % 2;
    zmt_reset(VOL_50);
    zos_err_t err = load_zmt(&track, track_index);
    handle_error(err, "Failed to load ZMT track", 1);

    return 0;
}

void deinit(void)
{
    zvb_ctrl_l0_scr_x_low  = 0;
    zvb_ctrl_l0_scr_x_high = 0;
    zvb_ctrl_l0_scr_y_low  = 0;
    zvb_ctrl_l0_scr_y_high = 0;
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    sound_deinit();

    // TODO: clear tilesets

    sprites_deregister();
    // // TODO: error checking
}

uint8_t input(void)
{
    uint16_t input = input_get();

    player.direction.x = DIRECTION_NONE; // not moving
    if ((input & BUTTON_LEFT))
        player.direction.x = DIRECTION_LEFT;
    if ((input & BUTTON_RIGHT))
        player.direction.x = DIRECTION_RIGHT;
    if ((input & BUTTON_START))
        return ACTION_PAUSE;
    if ((input & BUTTON_SELECT))
        return ACTION_QUIT;
    if ((input & BUTTON_B))
        launched = 1;

    // /**** TESTING ****/
    // if(input & BUTTON_UP && player.width < PLAYER_MAX_WIDTH) player_set_width(player.width+1);
    // if(input & BUTTON_DOWN && player.width > 1) player_set_width(player.width-1);
    // /**** TESTING ****/

    return 0;
}

void update(void)
{
    player_move();
    Edge edge = EdgeNone;
    if (!launched)
        return;
    edge = ball_move();
    if ((edge & EdgeTop)) {
        nudge++;
        if (nudge > 2) {
            nudge = 0;
            ball_nudge(ball.direction.x);
        }
    }

    uint16_t ball_top = rect_top(&ball.rect);
    if (ball_top > SCREEN_HEIGHT + SPRITE_HEIGHT) {
        sound_play(VOICE3, 294, 3);
        player.lives--;
        launched = 0;
        zmt_sound_off();
        msleep(750);
        zmt_track_reset(&track, 1);
        ball_reset();
        return;
    }

    edge = EdgeNone;
    char text[10];

#ifdef DEBUG
    text[0] = 'W';
    itoa_pad(ball.rect.w, text + 1, 10, 'A', '0', 3);
    nprint_string(&vctx, text, 5, 0, HEIGHT - 3);
    text[0] = 'H';
    itoa_pad(ball.rect.h, text + 1, 10, 'A', '0', 3);
    nprint_string(&vctx, text, 5, 5, HEIGHT - 3);
    text[0] = 'X';
    itoa_pad(ball.rect.x, text + 1, 10, 'A', '0', 3);
    nprint_string(&vctx, text, 5, 0, HEIGHT - 4);
    text[0] = 'Y';
    itoa_pad(ball.rect.y, text + 1, 10, 'A', '0', 3);
    nprint_string(&vctx, text, 5, 5, HEIGHT - 4);
#endif

    edge = player_collide(&ball.rect);
    if (edge != EdgeNone) {
        sound_play(VOICE3, 400, 4);
        switch (edge) {
            // uno reverse
            case EdgeLeft: ball_bounce(edge | EdgeTop); break;
            case EdgeRight: ball_bounce(edge | EdgeTop); break;
            case EdgeTop: {
                uint16_t player_l   = rect_left(&player.rect);
                uint16_t player_r   = rect_right(&player.rect);
                uint16_t zone_width = player.rect.w / 3;
                uint16_t ball_x     = rect_left(&ball.rect) + (ball.rect.w / 2);

                if (ball_x < player_l + zone_width)
                    edge |= EdgeLeft;
                else if (ball_x > player_r - zone_width)
                    edge |= EdgeRight;
                else if (player.direction.x == DIRECTION_LEFT)
                    edge |= EdgeLeft;
                else if (player.direction.x == DIRECTION_RIGHT)
                    edge |= EdgeRight;

                ball_bounce(edge);
                break;
            }
            case EdgeBottom: ball_bounce(edge); break;
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
    text[0] = 'W';
    itoa_pad(tile.rect.w, text + 1, 10, 'A', '0', 3);
    nprint_string(&vctx, text, 5, WIDTH - 9, HEIGHT - 3);
    text[0] = 'H';
    itoa_pad(tile.rect.h, text + 1, 10, 'A', '0', 3);
    nprint_string(&vctx, text, 5, WIDTH - 4, HEIGHT - 3);
    text[0] = 'X';
    itoa_pad(tile.rect.x, text + 1, 10, 'A', '0', 3);
    nprint_string(&vctx, text, 5, WIDTH - 9, HEIGHT - 4);
    text[0] = 'Y';
    itoa_pad(tile.rect.y, text + 1, 10, 'A', '0', 3);
    nprint_string(&vctx, text, 5, WIDTH - 4, HEIGHT - 4);
#endif
    /** /debug */

    uint8_t brick_x = tile.x >> 1;
    if (tile.y >= LEVEL_HEIGHT || brick_x >= LEVEL_WIDTH)
        return;

    uint16_t brick_offset = (tile.y * LEVEL_WIDTH) + brick_x;
    Brick* brick = &level.bricks[brick_offset];

    if (brick->health < 1)
        return;

    sound_play(VOICE3, 220, 2);

    /** DEBUG */
#ifdef DEBUG
    text[0] = 'E';
    text[1] = 'B';
    itoa_pad(edge, text + 2, 10, 'A', '0', 2);
    nprint_string(&vctx, text, 4, 0, HEIGHT - 5);
#endif
    /** /DEBUG */

    uint8_t seam_edge = (tile.x % 2) == 1 ? EdgeLeft : EdgeRight;
    uint8_t masked_edge = edge & (0xFF ^ seam_edge);
    if (masked_edge != EdgeNone)
        edge = masked_edge;

    /** DEBUG */
#ifdef DEBUG
    text[0] = 'E';
    text[1] = 'A';
    itoa_pad(edge, text + 2, 10, 'A', '0', 2);
    nprint_string(&vctx, text, 4, 0, HEIGHT - 6);
#endif
    /** /DEBUG */

    if (edge == EdgeNone)
        return;

    // move the ball so it doesn't go "inside" the brick
    if ((edge & EdgeTop) > 0) {
        uint16_t tile_top = rect_top(&tile.rect);
        ball.rect.y       = tile_top > ball.rect.h ? tile_top - 1 : ball.rect.h;
        ball.sprite->y = ball.rect.y;
    } else if ((edge & EdgeBottom) > 0) {
        ball.rect.y   = rect_bottom(&tile.rect) + ball.rect.h + 1;
        if (ball.rect.y > SCREEN_HEIGHT)
            ball.rect.y = SCREEN_HEIGHT;
        ball.sprite->y = ball.rect.y;
    }

    if ((edge & EdgeRight) > 0) {
        ball.rect.x   = rect_right(&tile.rect) + ball.rect.w + 1;
        if (ball.rect.x > SCREEN_WIDTH)
            ball.rect.x = SCREEN_WIDTH;
        ball.sprite->x = ball.rect.x;
    } else if ((edge & EdgeLeft) > 0) {
        uint16_t tile_left = rect_left(&tile.rect);
        ball.rect.x        = tile_left > ball.rect.w ? tile_left - 1 : ball.rect.w;
        ball.sprite->x = ball.rect.x;
    }

    ball_bounce(edge);

    brick->health--;
    nudge = 0;
    if (brick->health < 1) {
        // remove brick
        level.brick_count--;
        player.score += brick->points; // TODO: point modifier???
        itoa_pad(player.score, text, 10, 'A', '0', 3);
        nprint_string(&vctx, text, 3, 0, 0);

        gfx_tilemap_place(&vctx, EMPTY_TILE, LAYER0, brick->x, brick->y);
        gfx_tilemap_place(&vctx, EMPTY_TILE, LAYER0, brick->x + 1, brick->y);
    }
}

void draw(void)
{
    gfx_wait_vblank(&vctx);
    // player_draw();
    // ball_draw();
    sprites_render(&vctx);

#ifdef DEBUG
    gfx_sprite_render(&vctx, DEBUG_TILE_INDEX, &DEBUG_TILE);
#endif

    gfx_wait_end_vblank(&vctx);
}

void draw_paused(uint8_t paused)
{
    if (paused)
        nprint_string(&vctx, "PAUSED", 6, WIDTH / 2 - 3, HEIGHT / 2);
    else
        nprint_string(&vctx, "      ", 6, WIDTH / 2 - 3, HEIGHT / 2);
}

void draw_gameover(uint8_t gameover)
{
    if (gameover)
        nprint_string(&vctx, "GAME  OVER", 10, WIDTH / 2 - 5, HEIGHT / 2);
    else
        nprint_string(&vctx, "          ", 10, WIDTH / 2 - 5, HEIGHT / 2);
}
