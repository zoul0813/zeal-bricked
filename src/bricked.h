#include <stdint.h>
#include <zvb_gfx.h>

#ifndef BRICKED_H
#define BRICKED_H

#define ACTION_NONE     0
#define ACTION_PAUSE    1
#define ACTION_CONTINUE 1
#define ACTION_QUIT     10

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define WIDTH         20
#define HEIGHT        15

#define TILEMAP_OFFSET 0x00U
#define EMPTY_TILE     0x00U

#define MAX_SPRITES   32

typedef uint8_t error;

void on_error(zos_err_t err);
void init(void);
void deinit(void);
void reset(void);
uint8_t input(void);
error load_level(uint8_t which);

void update(void);
void draw_paused(uint8_t paused);
void draw_gameover(uint8_t gameover);
void draw(void);

extern gfx_context vctx;

#endif
