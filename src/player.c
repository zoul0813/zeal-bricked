#include <zos_sys.h>
#include <zos_errors.h>
#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sprite.h>
#include <zgdk.h>
#include "player.h"

Player player;

void player_set_x(uint16_t x) {

    uint16_t max_screen = SCREEN_WIDTH - (SPRITE_WIDTH * (player.width+1));
    if(x < SPRITE_WIDTH) x = SPRITE_WIDTH;
    if(x > max_screen) x = max_screen;

    player.rect.x = x;
    player.spritel.x = x;
    for(uint8_t i = 0; i < player.width; i++) {
        x+= SPRITE_WIDTH;
        player.spritem[i].x = x;
    }
    x += SPRITE_WIDTH;
    player.spriter.x = x;
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

zos_err_t player_init(uint8_t player_reset) {
    zos_err_t err;
    player.speed = PLAYER_SPEED;
    player.rect.y = PLAYER_Y;
    player.rect.h = SPRITE_HEIGHT;

    if(player_reset) {
        uint8_t sprite_index = 0;
        player.sprite_index = sprite_index;

        player.spritel.tile = PADDLE1;
        player.spritel.y = PLAYER_Y;
        err = gfx_sprite_render(&vctx, sprite_index, &player.spritel);
        if(err) return err;

        player.width = 1;
        for(uint8_t i = 0; i < PLAYER_MAX_WIDTH; i++) {
            player.spritem[i].tile = PADDLE1+1;
            sprite_index++;
            err = gfx_sprite_render(&vctx, sprite_index, &player.spritem[i]);
            if(err) return err;
        }

        player.spriter.tile = PADDLE1+2;
        player.spriter.y = player.spritel.y;
        sprite_index++;
        err = gfx_sprite_render(&vctx, sprite_index, &player.spriter);
        if(err) return err;
    }

    player_set_width(player.width);
    player_set_x((SCREEN_WIDTH / 2) - SPRITE_WIDTH);

    return ERR_SUCCESS;
}

void player_move(void) {
    if(player.direction.x == DIRECTION_NONE) return;
    int16_t x = player.rect.x;
    x += player.direction.x * player.speed;
    player_set_x(x);
}

Edge player_collide(Rect *rect) {
    // TODO: because 320x240, the Y region could be uint8_t?

    // Y regions
    const uint16_t player_t = PLAYER_Y;
    const uint16_t player_b = PLAYER_Y + SPRITE_HEIGHT;
    const uint16_t target_t = rect_top(rect);
    const uint16_t target_b = rect_bottom(rect);

    // is target within the players Y region
    if(player_t > target_b) return EdgeNone;
    if(player_b < target_t) return EdgeNone;

    // X regions
    const uint16_t player_l = rect_left(&player.rect);
    const uint16_t player_r = rect_right(&player.rect);
    const uint16_t target_l = rect_left(rect);
    const uint16_t target_r = rect_right(rect);

    // is target within the players X region
    if(player_l > target_r) return EdgeNone;
    if(player_r < target_l) return EdgeNone;


    Edge edge = EdgeNone;
    if(target_r < player_l + TILE_QUARTER) edge = EdgeLeft;
    else if(target_l > player_r - TILE_QUARTER) edge = EdgeRight;
    else if(target_t > player_b - TILE_QUARTER) edge = EdgeBottom;
    else if(target_b < player_t + TILE_QUARTER) edge = EdgeTop;

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
