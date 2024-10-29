#include <stdint.h>
#include <zvb_sprite.h>
#include "bricked.h"
#include "player.h"

#ifndef BALLS_H
#define BALLS_H

#define BALL_SPEED  1
#define BALL_OFFSET 4
#define BALL_WIDTH  8
#define BALL_HEIGHT 8

typedef struct {
  uint8_t speed;
  uint8_t sprite_index;
  int8_t xd;
  int8_t yd;
  Rect rect;
  gfx_sprite sprite;
} Ball;

extern Ball ball;

zos_err_t ball_init(uint8_t reset);
void ball_bounce(Edge edge);
void ball_move(void);
void ball_draw(void);

#endif