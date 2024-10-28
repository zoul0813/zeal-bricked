#include <zgdk.h>
#include <zos_errors.h>
#include "bricked.h"
#include "ball.h"

Ball ball;

zos_err_t ball_init(uint8_t reset) {
    (void *)reset;
    zos_err_t err;
    ball.sprite.tile = 1;
    ball.sprite.x = SCREEN_WIDTH / 2;
    ball.sprite.y = SCREEN_HEIGHT - (SPRITE_HEIGHT * 5);
    ball.x = ball.sprite.x + BALL_OFFSET;
    ball.y = ball.sprite.y + BALL_OFFSET;
    ball.xd = 1;
    ball.yd = 1;
    ball.speed = BALL_SPEED;
    ball.sprite_index = PLAYER_MAX_WIDTH + 2;
    err = gfx_sprite_render(&vctx, ball.sprite_index, &ball.sprite);
    if(err) return ERR_FAILURE; // err;

    return ERR_SUCCESS;
}

void ball_bounce(int8_t cx, int8_t cy) {
  if(cx != 0) {
    if(ball.xd > 0) ball.xd = -1;
    else ball.xd = 1;
  }

  if(cy != 0) {
    if(ball.yd > 0) ball.yd = -1;
    else ball.yd = 1;
  }
}

void ball_move(void) {

    uint16_t x = ball.sprite.x;
    uint16_t y = ball.sprite.y;

    x += ball.xd;
    y += ball.yd;

    if((x > SCREEN_WIDTH + BALL_OFFSET)) ball.xd = -1;
    if((x <= SPRITE_WIDTH - BALL_OFFSET)) ball.xd = 1;

    if((y > SCREEN_HEIGHT + BALL_OFFSET)) ball.yd = -1;
    if((y <= SPRITE_HEIGHT - BALL_OFFSET)) ball.yd = 1;

    ball.sprite.x = x;
    ball.sprite.y = y;
    ball.x = x+BALL_OFFSET;
    ball.y = y+BALL_OFFSET;
}

void ball_draw(void) {
  gfx_sprite_render(&vctx, ball.sprite_index, &ball.sprite);
}
