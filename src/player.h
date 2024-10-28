#include <stdio.h>
#include <zos_sys.h>

#include <stdint.h>
#include <zvb_sprite.h>

#include "bricked.h"

#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_SPEED        3
#define PLAYER_Y            (SCREEN_HEIGHT - SPRITE_HEIGHT - SPRITE_HEIGHT)
#define PLAYER_MAX_WIDTH    3

typedef struct {
    uint8_t sprite_index;
    uint16_t x;
    gfx_sprite spritel;
    gfx_sprite spritem[3];
    gfx_sprite spriter;
    uint8_t width;
    uint8_t speed;
    uint8_t score;
    int8_t direction;
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
zos_err_t player_init(uint8_t player_reset);
void player_move(void);
bool player_collide(uint16_t x, uint16_t y, uint8_t offset);
void player_draw(void);
#endif