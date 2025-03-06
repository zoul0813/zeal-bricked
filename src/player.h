#include <stdint.h>
#include <zvb_sprite.h>
#include <zgdk.h>
#include "bricked.h"

#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_SPEED     3
#define PLAYER_Y         (SCREEN_HEIGHT - (SPRITE_HEIGHT / 2))
#define PLAYER_MAX_WIDTH 3
#define PLAYER_LIVES     3

typedef struct {
        Rect rect;
        uint8_t width; // width of middle segments
        uint8_t speed;
        uint8_t score;
        uint8_t lives;
        Direction direction;

        /* sprites */
        gfx_sprite* spritel;
        gfx_sprite* spritem[3];
        gfx_sprite* spriter;
} Player;

#define PADDLE1 3

extern Player player;

void player_set_x(uint16_t x);
void player_set_width(uint8_t width);
zos_err_t player_init(void);
void player_reset(void);
void player_move(void);
Edge player_collide(Rect* rect);
#endif
