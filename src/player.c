#include <zgdk.h>
#include <zos_errors.h>
#include "player.h"

Player player;

void player_set_x(uint16_t x) {

    uint16_t max_screen = SCREEN_WIDTH - (SPRITE_WIDTH * (player.width+1));
    if(x < SPRITE_WIDTH) x = SPRITE_WIDTH;
    if(x > max_screen) x = max_screen;

    player.x = x;
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

    player_set_x(player.x);
}

zos_err_t player_init(uint8_t player_reset) {
    zos_err_t err;
    player.speed = PLAYER_SPEED;

    uint8_t sprite_index = 0;
    player.sprite_index = sprite_index;

    player.spritel.tile = PADDLE1;
    player.spritel.y = PLAYER_Y;
    err = gfx_sprite_render(&vctx, sprite_index, &player.spritel);
    if(err) return err;

    for(uint8_t i = 0; i < PLAYER_MAX_WIDTH; i++) {
        player.spritem[i].tile = PADDLE1+1;
        player.spritem[i].y = SCREEN_HEIGHT + SPRITE_WIDTH;
        sprite_index++;
        err = gfx_sprite_render(&vctx, sprite_index, &player.spritem[i]);
        if(err) return err;
    }

    player.spriter.tile = PADDLE1+2;
    player.spriter.y = player.spritel.y;
    sprite_index++;
    err = gfx_sprite_render(&vctx, sprite_index, &player.spriter);
    if(err) return err;

    if(player_reset) {
        player_set_width(1);
    }

    player_set_x((SCREEN_WIDTH / 2) - SPRITE_WIDTH);

    return ERR_SUCCESS;
}

void player_move(void) {
    if(player.direction == 0) return;
    int16_t x = player.x;
    x += player.direction * player.speed;
    player_set_x(x);
}

bool player_collide(uint16_t x, uint16_t y, uint8_t offset) {
    uint16_t miny = y - offset - offset + SPRITE_HEIGHT;
    uint16_t maxy = miny + SPRITE_HEIGHT;

    if(miny < PLAYER_Y) return false;
    if(maxy > PLAYER_Y + SPRITE_HEIGHT) return false;

    uint16_t minx = player.x + offset;
    uint16_t maxx = minx + (SPRITE_WIDTH * (player.width + 2));
    if((x > minx) && (x < maxx)) return true;
    return false;
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
