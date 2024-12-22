#include <zgdk.h>
#include <zos_errors.h>
#include "bricked.h"
#include "ball.h"

Ball ball;

zos_err_t ball_init(void)
{
    zos_err_t err;
    ball.sprite.tile  = 1;
    ball.rect.w       = BALL_WIDTH;
    ball.rect.h       = BALL_HEIGHT;
    ball.sprite_index = PLAYER_MAX_WIDTH + 2;

    err = gfx_sprite_render(&vctx, ball.sprite_index, &ball.sprite);
    if (err)
        return ERR_FAILURE; // err;

    ball_reset();

    return ERR_SUCCESS;
}

void ball_reset(void)
{
    // offset by 3 so it's not "perfectly center"
    ball.sprite.x    = ((SCREEN_WIDTH / 2) - SPRITE_WIDTH) + 3;
    ball.sprite.y    = SCREEN_HEIGHT - (SPRITE_HEIGHT * 5);
    ball.rect.x      = ball.sprite.x;
    ball.rect.y      = ball.sprite.y;
    ball.direction.x = DIRECTION_RIGHT;
    ball.direction.y = DIRECTION_UP;
    ball.speed       = BALL_SPEED;
}

void ball_bounce(Edge edge)
{
    // top/bottom
    if (edge & EdgeBottom)
        ball.direction.y = DIRECTION_DOWN;
    else if (edge & EdgeTop)
        ball.direction.y = DIRECTION_UP;

    if (edge & EdgeRight)
        ball.direction.x = DIRECTION_RIGHT;
    else if (edge & EdgeLeft)
        ball.direction.x = DIRECTION_LEFT;
    ball.edge = edge;
}

void ball_nudge(int8_t direction)
{
    ball.sprite.x += direction * 2;
    ball.rect.x    = ball.sprite.x;
}

Edge ball_move(void)
{

    uint16_t x = ball.sprite.x;
    uint16_t y = ball.sprite.y;

    switch (ball.edge) {
        case EdgeLeft:
        case EdgeRight:
            x += ball.direction.x * (ball.speed) + 1;
            y += ball.direction.y * (ball.speed - 1) + 1;
            break;
        case EdgeTop:
        case EdgeBottom:
            x += ball.direction.x * (ball.speed - 1) + 1;
            y += ball.direction.y * (ball.speed) + 1;
            break;
        default: x += ball.direction.x * (ball.speed) + 1; y += ball.direction.y * (ball.speed) + 1;
    }
    ball.edge = EdgeNone;
    x--;
    y--;
    // x += ball.direction.x * ball.speed;
    // y += ball.direction.y * ball.speed;

    Edge edge = EdgeNone;

    if ((y <= SPRITE_HEIGHT - (SPRITE_HEIGHT - BALL_HEIGHT))) {
        ball_bounce(EdgeBottom); // ball.direction.y = 1;
        y    = SPRITE_HEIGHT - (SPRITE_HEIGHT - BALL_HEIGHT);
        edge = EdgeTop;
    }

    // if((y >= SCREEN_HEIGHT)) {
    //     ball_bounce(EdgeTop); //ball.direction.y = -1;
    //     y = SCREEN_HEIGHT;
    // }

    if ((x <= SPRITE_WIDTH - (SPRITE_WIDTH - BALL_WIDTH))) {
        ball_bounce(EdgeRight); // ball.direction.x = 1;
        x    = SPRITE_WIDTH - (SPRITE_WIDTH - BALL_WIDTH);
        edge = EdgeLeft;
    }

    if ((x >= SCREEN_WIDTH)) {
        ball_bounce(EdgeLeft); // ball.direction.x = -1;
        x    = SCREEN_WIDTH;
        edge = EdgeRight;
    }

    ball.sprite.x = x;
    ball.sprite.y = y;
    ball.rect.x   = x;
    ball.rect.y   = y;

    return edge;
}

void ball_draw(void)
{
    gfx_sprite_render(&vctx, ball.sprite_index, &ball.sprite);
}
