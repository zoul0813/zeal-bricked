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
    ball.rect.x = ball.sprite.x + BALL_OFFSET;
    ball.rect.y = ball.sprite.y + BALL_OFFSET;
    ball.rect.w = BALL_WIDTH;
    ball.rect.h = BALL_HEIGHT;
    ball.direction.x = DIRECTION_RIGHT;
    ball.direction.y = DIRECTION_UP;
    ball.speed = BALL_SPEED;
    ball.sprite_index = PLAYER_MAX_WIDTH + 2;
    err = gfx_sprite_render(&vctx, ball.sprite_index, &ball.sprite);
    if(err) return ERR_FAILURE; // err;

    return ERR_SUCCESS;
}

void ball_bounce(Edge edge) {
    // top/bottom
    if(edge & EdgeBottom) ball.direction.y = DIRECTION_DOWN;
    else if(edge & EdgeTop) ball.direction.y = DIRECTION_UP;

    if(edge & EdgeRight) ball.direction.x = DIRECTION_LEFT;
    else if(edge & EdgeLeft) ball.direction.x = DIRECTION_RIGHT;
}

void ball_move(void) {

    uint16_t x = ball.sprite.x;
    uint16_t y = ball.sprite.y;

    x += ball.direction.x * ball.speed;
    y += ball.direction.y * ball.speed;

    if((y <= SPRITE_HEIGHT - BALL_OFFSET)) {
        ball_bounce(EdgeBottom); //ball.direction.y = 1;
        y = SPRITE_HEIGHT - BALL_OFFSET;
    }
    if((y >= SCREEN_HEIGHT + BALL_OFFSET)) {
        ball_bounce(EdgeTop); //ball.direction.y = -1;
        y = SCREEN_HEIGHT + BALL_OFFSET;
    }

    if((x <= SPRITE_WIDTH - BALL_OFFSET)) {
        ball_bounce(EdgeLeft); // ball.direction.x = 1;
        x = SPRITE_WIDTH - BALL_OFFSET;
    }

    if((x >= SCREEN_WIDTH + BALL_OFFSET)) {
        ball_bounce(EdgeRight); // ball.direction.x = -1;
        x = SCREEN_WIDTH + BALL_OFFSET;
    }

    ball.sprite.x = x;
    ball.sprite.y = y;
    ball.rect.x = x + BALL_OFFSET;
    ball.rect.y = y + BALL_OFFSET;
}

void ball_draw(void) {
  gfx_sprite_render(&vctx, ball.sprite_index, &ball.sprite);
}
