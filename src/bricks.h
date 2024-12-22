#include <stdio.h>
#include <zvb_gfx.h>
#include <zgdk.h>

#ifndef BRICKS_H
#define BRICKS_H

#define BRICK_HEIGHT TILE_HEIGHT
#define BRICK_WIDTH  (TILE_WIDTH * 2)

#define BRICK01 6U
#define BRICK02 8U
#define BRICK03 10U
#define BRICK04 12U
#define BRICK05 14U
#define BRICK06 16U
#define BRICK07 18U
#define BRICK08 20U
#define BRICK09 22U
#define BRICK10 24U
#define BRICK11 26U
#define BRICK12 28U
#define BRICK13 30U
#define BRICK14 32U
#define BRICK15 34U
#define BRICK16 36U
#define BRICK17 38U
#define BRICK18 40U
#define BRICK19 42U
#define BRICK20 44U
#define BRICK21 46U

#define BRICK_COUNT 21

typedef struct {
        uint8_t health;
        uint8_t l; // left tile
        uint8_t r; // right tile
        uint8_t x;
        uint8_t y;
        uint8_t points;
} Brick;

extern const Brick BRICKS[BRICK_COUNT];

static uint16_t brick_px(uint8_t x)
{
    return (x * TILE_WIDTH) + TILE_WIDTH;
}

static uint16_t brick_py(uint8_t y)
{
    return (y * TILE_HEIGHT) + TILE_HEIGHT;
}

#endif