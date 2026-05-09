#include <zos_sys.h>
#include <zos_errors.h>
#include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sprite.h>
#include <zgdk.h>
#include "player.h"

Player player;

void player_set_x(uint16_t x)
{

    uint16_t mmin_screen = SPRITE_WIDTH + (SPRITE_WIDTH * (player.width + 1));
    if (x < mmin_screen)
        x = mmin_screen;
    if (x > SCREEN_WIDTH)
        x = SCREEN_WIDTH;

    player.spriter->x = x;
    player.rect.x    = x;
    for (uint8_t i = 0; i < player.width; i++) {
        x                   -= SPRITE_WIDTH;
        player.spritem[i]->x  = x;
    }
    x                -= SPRITE_WIDTH;
    player.spritel->x  = x;
}

void player_set_width(uint8_t width)
{
    player.width = width;
    for (uint8_t i = 0; i < PLAYER_MAX_WIDTH; i++) {
        if (i < width) {
            player.spritem[i]->y = PLAYER_Y;
        } else {
            player.spritem[i]->y = SCREEN_HEIGHT + SPRITE_HEIGHT;
        }
    }

    player.rect.w = SPRITE_WIDTH * (player.width + 2);
    player_set_x(player.rect.x);
}

zos_err_t player_init(void)
{
    player.rect.h = SPRITE_HEIGHT;
    player.lives  = PLAYER_LIVES;


    gfx_sprite sprite = {
        .tile = PADDLE1,
        .y = PLAYER_Y,
    };

    player.spritel = sprites_register(sprite);
    player.rect.y       = PLAYER_Y;

    player.width = 1;
    for (uint8_t i = 0; i < PLAYER_MAX_WIDTH; i++) {
        sprite.tile = PADDLE1 + 1;
        player.spritem[i] = sprites_register(sprite);
    }

    sprite.tile = PADDLE1 + 2;
    player.spriter = sprites_register(sprite);

    player_reset();

    return ERR_SUCCESS;
}

void player_reset(void)
{
    player.speed = PLAYER_SPEED;
    player_set_width(player.width);
    player_set_x((SCREEN_WIDTH / 2) - SPRITE_WIDTH);
}

void player_move(void)
{
    if (player.direction.x == DIRECTION_NONE)
        return;
    int16_t x  = player.rect.x;
    x         += player.direction.x * player.speed;
    player_set_x(x);
}

Edge player_collide(Rect* rect)
{
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
    if (rect_b < player_t)
        return EdgeNone;
    if (rect_t > player_b)
        return EdgeNone;

    // // is rect within the players X region
    if (rect_r < player_l)
        return EdgeNone;
    if (rect_l > player_r)
        return EdgeNone;

    int16_t overlap_top    = (int16_t)rect_b - (int16_t)player_t;
    int16_t overlap_bottom = (int16_t)player_b - (int16_t)rect_t;
    int16_t overlap_left   = (int16_t)rect_r - (int16_t)player_l;
    int16_t overlap_right  = (int16_t)player_r - (int16_t)rect_l;

    Edge edge       = EdgeTop;
    int16_t overlap = overlap_top;

    if (overlap_bottom < overlap) {
        edge    = EdgeBottom;
        overlap = overlap_bottom;
    }
    if (overlap_left < overlap) {
        edge    = EdgeLeft;
        overlap = overlap_left;
    }
    if (overlap_right < overlap)
        edge = EdgeRight;

    return edge;
}
