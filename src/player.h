#include <stdint.h>
#include <zvb_sprite.h>
#include <zgdk.h>
#include "bricked.h"

#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_SPEED        3
#define PLAYER_Y            (SCREEN_HEIGHT - (SPRITE_HEIGHT / 2))
#define PLAYER_MAX_WIDTH    3

typedef struct {
    uint8_t sprite_index;
    gfx_sprite spritel;
    gfx_sprite spritem[3];
    gfx_sprite spriter;
    Rect rect;
    uint8_t width; // width of middle segments
    uint8_t speed;
    uint8_t score;
    Direction direction;
} Player;

#define PADDLE1 64
#define PADDLE2 67
#define PADDLE3 70
#define PADDLE4 73
#define PADDLE5 76
#define PADDLE6 80
#define PADDLE7 83

extern Player player;

void player_set_x(uint16_t x);
void player_set_width(uint8_t width);
zos_err_t player_init(void);
void player_reset(void);
void player_move(void);
Edge player_collide(Rect *rect);
void player_draw(void);
#endif