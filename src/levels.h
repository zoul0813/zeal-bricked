#include <stdio.h>
#include <stdint.h>
#include "bricks.h"

#ifndef LEVELS_H
#define LEVELS_H

#define LEVEL_HEIGHT     8
#define LEVEL_WIDTH      10
#define LEVEL_COUNT      16
#define LEVEL_TILE_COUNT (LEVEL_WIDTH * LEVEL_HEIGHT)

typedef struct {
        uint8_t index;
        uint8_t brick_count;
        Brick bricks[LEVEL_TILE_COUNT];
} Level;

extern const uint8_t LEVEL_TILES[LEVEL_COUNT][LEVEL_TILE_COUNT];

#endif