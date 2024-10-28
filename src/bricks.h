#include <stdio.h>
#include <zvb_gfx.h>

#ifndef BRICKS_H
#define BRICKS_H

#define BRICK01      8
#define BRICK02     10
#define BRICK03     12
#define BRICK04     14
#define BRICK05     16
#define BRICK06     18
#define BRICK07     20
#define BRICK08     22
#define BRICK09     24
#define BRICK10     26
#define BRICK11     28
#define BRICK12     30
#define BRICK13     32
#define BRICK14     34
#define BRICK15     36
#define BRICK16     38
#define BRICK17     40
#define BRICK18     42
#define BRICK19     44
#define BRICK20     46
#define BRICK21     48
#define BRICK22     50
#define BRICK23     52
#define BRICK24     54
#define BRICK25     56
#define BRICK26     58
#define BRICK27     60
#define BRICK28     52

#define BRICK_COUNT 28

typedef struct {
  uint8_t health;
  uint8_t l; // left tile
  uint8_t r; // right tile
  uint8_t x;
  uint8_t y;
} Brick;

extern const Brick BRICKS[BRICK_COUNT];

#endif