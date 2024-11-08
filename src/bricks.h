#include <stdio.h>
#include <zvb_gfx.h>
#include <zgdk.h>

#ifndef BRICKS_H
#define BRICKS_H

#define BRICK_HEIGHT  TILE_HEIGHT
#define BRICK_WIDTH   (TILE_WIDTH * 2)

#define BRICK01      8U
#define BRICK02     10U
#define BRICK03     12U
#define BRICK04     14U
#define BRICK05     16U
#define BRICK06     18U
#define BRICK07     20U
#define BRICK08     22U
#define BRICK09     24U
#define BRICK10     26U
#define BRICK11     28U
#define BRICK12     30U
#define BRICK13     32U
#define BRICK14     34U
#define BRICK15     36U
#define BRICK16     38U
#define BRICK17     40U
#define BRICK18     42U
#define BRICK19     44U
#define BRICK20     46U
#define BRICK21     48U
#define BRICK22     50U
#define BRICK23     52U
#define BRICK24     54U
#define BRICK25     56U
#define BRICK26     58U
#define BRICK27     60U
#define BRICK28     52U

#define BRICK_COUNT 28

typedef struct {
  uint8_t health;
  uint8_t l; // left tile
  uint8_t r; // right tile
  uint8_t x;
  uint8_t y;
  uint8_t points;
} Brick;

extern const Brick BRICKS[BRICK_COUNT];

static uint16_t brick_px(uint8_t x) {
  return (x * TILE_WIDTH) + TILE_WIDTH;
}

static uint16_t brick_py(uint8_t y) {
  return (y * TILE_HEIGHT) + TILE_HEIGHT;
}

#endif