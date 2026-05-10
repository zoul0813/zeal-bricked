#include <zgdk.h>
#include <zos_errors.h>
#include "bricked.h"
#include "ball.h"

Ball ball;

static void ball_sync_position(void)
{
    ball.x_fp = (int16_t)ball.sprite->x << BALL_FP_SHIFT;
    ball.y_fp = (int16_t)ball.sprite->y << BALL_FP_SHIFT;
}

static void ball_set_velocity(int8_t velocity_x, int8_t velocity_y)
{
    ball.velocity_x = velocity_x;
    ball.velocity_y = velocity_y;

    if (velocity_x < 0)
        ball.direction.x = DIRECTION_LEFT;
    else if (velocity_x > 0)
        ball.direction.x = DIRECTION_RIGHT;

    if (velocity_y < 0)
        ball.direction.y = DIRECTION_UP;
    else if (velocity_y > 0)
        ball.direction.y = DIRECTION_DOWN;
}

zos_err_t ball_init(void)
{
    ball.rect.w       = BALL_WIDTH;
    ball.rect.h       = BALL_HEIGHT;

    gfx_sprite sprite = {
        .tile = 1,
    };
    ball.sprite = sprites_register(sprite);

    ball_reset();

    return ERR_SUCCESS;
}

void ball_reset(void)
{
    // offset by 3 so it's not "perfectly center"
    ball.sprite->x    = ((SCREEN_WIDTH / 2) - SPRITE_WIDTH) + 3;
    ball.sprite->y    = SCREEN_HEIGHT - (SPRITE_HEIGHT * 5);
    ball.rect.x      = ball.sprite->x;
    ball.rect.y      = ball.sprite->y;
    ball.speed       = BALL_SPEED;
    ball.edge        = EdgeNone;
    ball_sync_position();
    ball_set_velocity(BALL_SPEED << BALL_FP_SHIFT, -(BALL_SPEED << BALL_FP_SHIFT));
}

void ball_bounce(Edge edge)
{
    ball_sync_position();

    // top/bottom
    if (edge & EdgeBottom)
        ball.velocity_y = ball.velocity_y < 0 ? -ball.velocity_y : ball.velocity_y;
    else if (edge & EdgeTop)
        ball.velocity_y = ball.velocity_y > 0 ? -ball.velocity_y : ball.velocity_y;

    if (edge & EdgeRight)
        ball.velocity_x = ball.velocity_x < 0 ? -ball.velocity_x : ball.velocity_x;
    else if (edge & EdgeLeft)
        ball.velocity_x = ball.velocity_x > 0 ? -ball.velocity_x : ball.velocity_x;

    ball_set_velocity(ball.velocity_x, ball.velocity_y);
    ball.edge = edge;
}

void ball_paddle_bounce(uint16_t ball_center_x, uint16_t paddle_l, uint16_t paddle_w, int8_t paddle_direction)
{
    static const int8_t velocity_x[] = { -40, -34, -26, -14, 14, 26, 34, 40 };
    static const int8_t velocity_y[] = { -22, -30, -37, -43, -43, -37, -30, -22 };
    int16_t hit_x = (int16_t)ball_center_x - (int16_t)paddle_l;
    int8_t zone;

    if (hit_x < 0)
        hit_x = 0;
    if (hit_x >= paddle_w)
        hit_x = paddle_w - 1;

    zone = (int8_t)((hit_x * 8) / paddle_w);
    if (zone < 0)
        zone = 0;
    if (zone > 7)
        zone = 7;

    if (paddle_direction == DIRECTION_LEFT && zone > 0)
        zone--;
    else if (paddle_direction == DIRECTION_RIGHT && zone < 7)
        zone++;

    ball_set_velocity(velocity_x[zone], velocity_y[zone]);
    ball.edge = EdgeTop;
}

void ball_nudge(int8_t direction)
{
    ball.sprite->x += direction * 2;
    ball.rect.x    = ball.sprite->x;
    ball_sync_position();
}

Edge ball_move(void)
{

    int16_t x;
    int16_t y;

    ball.x_fp += ball.velocity_x;
    ball.y_fp += ball.velocity_y;
    x = ball.x_fp >> BALL_FP_SHIFT;
    y = ball.y_fp >> BALL_FP_SHIFT;
    ball.edge = EdgeNone;

    Edge edge = EdgeNone;

    if ((y <= SPRITE_HEIGHT - (SPRITE_HEIGHT - BALL_HEIGHT))) {
        y    = SPRITE_HEIGHT - (SPRITE_HEIGHT - BALL_HEIGHT);
        ball.sprite->x = x;
        ball.sprite->y = y;
        ball.rect.x    = x;
        ball.rect.y    = y;
        ball_sync_position();
        ball_bounce(EdgeBottom); // ball.direction.y = 1;
        edge = EdgeTop;
    }

    // if((y >= SCREEN_HEIGHT)) {
    //     ball_bounce(EdgeTop); //ball.direction.y = -1;
    //     y = SCREEN_HEIGHT;
    // }

    if ((x <= SPRITE_WIDTH - (SPRITE_WIDTH - BALL_WIDTH))) {
        x    = SPRITE_WIDTH - (SPRITE_WIDTH - BALL_WIDTH);
        ball.sprite->x = x;
        ball.sprite->y = y;
        ball.rect.x    = x;
        ball.rect.y    = y;
        ball_sync_position();
        ball_bounce(EdgeRight); // ball.direction.x = 1;
        edge = EdgeLeft;
    }

    if ((x >= SCREEN_WIDTH)) {
        x    = SCREEN_WIDTH;
        ball.sprite->x = x;
        ball.sprite->y = y;
        ball.rect.x    = x;
        ball.rect.y    = y;
        ball_sync_position();
        ball_bounce(EdgeLeft); // ball.direction.x = -1;
        edge = EdgeRight;
    }

    ball.sprite->x = x;
    ball.sprite->y = y;
    ball.rect.x   = x;
    ball.rect.y   = y;

    return edge;
}
