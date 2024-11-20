#include <zos_sys.h>
#include <zos_errors.h>
#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sprite.h>
#include <zgdk.h>
#include "player.h"

Player player;

void player_set_x(uint16_t x) {

    uint16_t mmin_screen = SPRITE_WIDTH + (SPRITE_WIDTH * (player.width+1));
    if(x < mmin_screen) x = mmin_screen;
    if(x > SCREEN_WIDTH) x = SCREEN_WIDTH;

    player.spriter.x = x;
    player.rect.x = x;
    for(uint8_t i = 0; i < player.width; i++) {
        x-= SPRITE_WIDTH;
        player.spritem[i].x = x;
    }
    x -= SPRITE_WIDTH;
    player.spritel.x = x;
}

void player_set_width(uint8_t width) {
    player.width = width;
    for(uint8_t i = 0; i < PLAYER_MAX_WIDTH; i++) {
        if(i < width) {
            player.spritem[i].y = PLAYER_Y;
        } else {
            player.spritem[i].y = SCREEN_HEIGHT + SPRITE_HEIGHT;
        }
    }

    player.rect.w = SPRITE_WIDTH * (player.width + 2);
    player_set_x(player.rect.x);
}

zos_err_t player_init(void) {
    zos_err_t err;
    player.rect.h = SPRITE_HEIGHT;

    uint8_t sprite_index = 0;
    player.sprite_index = sprite_index;

    player.spritel.tile = PADDLE1;
    player.spritel.y = PLAYER_Y;
    player.rect.y = PLAYER_Y;
    err = gfx_sprite_render(&vctx, sprite_index, &player.spritel);
    if(err) return err;

    player.width = 1;
    for(uint8_t i = 0; i < PLAYER_MAX_WIDTH; i++) {
        player.spritem[i].tile = PADDLE1+1;
        player.spritem[i].y = PLAYER_Y;
        sprite_index++;
        err = gfx_sprite_render(&vctx, sprite_index, &player.spritem[i]);
        if(err) return err;
    }

    player.spriter.tile = PADDLE1+2;
    player.spriter.y = PLAYER_Y;
    sprite_index++;
    err = gfx_sprite_render(&vctx, sprite_index, &player.spriter);
    if(err) return err;

    player_reset();

    return ERR_SUCCESS;
}

void player_reset(void) {
    player.speed = PLAYER_SPEED;
    player_set_width(player.width);
    player_set_x((SCREEN_WIDTH / 2) - SPRITE_WIDTH);
}

void player_move(void) {
    if(player.direction.x == DIRECTION_NONE) return;
    int16_t x = player.rect.x;
    x += player.direction.x * player.speed;
    player_set_x(x);
}

Edge player_collide(Rect *rect) {
    // TODO: because 320x240, the Y region could be uint8_t?

    const uint16_t player_t = rect_top(&player.rect);
    const uint16_t player_b = rect_bottom(&player.rect);
    const uint16_t player_l = rect_left(&player.rect);
    const uint16_t player_r = rect_right(&player.rect);

    const uint16_t rect_t = rect_top(rect);
    const uint16_t rect_b = rect_bottom(rect);
    const uint16_t rect_l = rect_left(rect);
    const uint16_t rect_r = rect_right(rect);

    // // is rect within the players Y region
    if(player_t > rect_b + 1) return EdgeNone;
    if(player_b < rect_t - 1) return EdgeNone;

    // // is rect within the players X region
    if(player_l > rect_r + 1) return EdgeNone;
    if(player_r < rect_l - 1) return EdgeNone;


    Edge edge = EdgeNone;
    if(rect_r <= player_l) edge |= EdgeLeft;
    else if(rect_l >= player_r) edge |= EdgeRight;

    if(rect_t >= player_b) edge |= EdgeBottom;
    else if(rect_b <= player_t) edge |= EdgeTop;

    // no collision
    return edge;
}

void player_draw(void) {
    uint8_t sprite_index = player.sprite_index;
    gfx_sprite_set_x(&vctx, sprite_index, player.spritel.x);
    for(uint8_t i = 0; i < PLAYER_MAX_WIDTH; i++) {
        sprite_index++;
        gfx_sprite_render(&vctx, sprite_index, &player.spritem[i]);
    }
    sprite_index++;
    gfx_sprite_set_x(&vctx, sprite_index, player.spriter.x);
}
