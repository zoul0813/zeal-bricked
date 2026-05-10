#include <stdint.h>
#include <zvb_sprite.h>
#include "bricked.h"
#include "player.h"

#ifndef BALLS_H
#define BALLS_H

#define BALL_WIDTH  8
#define BALL_HEIGHT 8
#define BALL_FP_SHIFT 4

typedef struct {
        Edge edge;
        Direction direction;
        int16_t velocity_x;
        int16_t velocity_y;
        int16_t x_fp;
        int16_t y_fp;
        Rect rect;

        /* sprites*/
        gfx_sprite* sprite;
} Ball;

extern Ball ball;

zos_err_t ball_init(void);
void ball_reset(void);
void ball_bounce(Edge edge);
void ball_paddle_bounce(uint16_t ball_center_x, uint16_t paddle_l, uint16_t paddle_w, int8_t paddle_direction);
void ball_nudge(int8_t direction);
Edge ball_move(void);
#endif
