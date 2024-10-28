#include <stdint.h>
#include <zvb_sprite.h>
#include "bricked.h"
#include "player.h"

#ifndef BALLS_H
#define BALLS_H

#define BALL_SPEED  2
#define BALL_OFFSET 4

typedef struct {
  uint8_t speed;
  uint8_t sprite_index;
  uint16_t x;
  uint16_t y;
  int8_t xd;
  int8_t yd;
  gfx_sprite sprite;
} Ball;

extern Ball ball;

zos_err_t ball_init(uint8_t reset);
void ball_bounce(int8_t cx, int8_t cy);
void ball_move(void);
void ball_draw(void);

#endif