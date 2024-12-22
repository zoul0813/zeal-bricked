#include <stdio.h>
#include <zos_errors.h>
#include <zvb_gfx.h>
#include <sound/tracker.h>
#include "assets.h"

gfx_error load_palette(gfx_context* ctx)
{
    // Load the palette
    const size_t palette_size = &_palette_end - &_palette_start;
    return gfx_palette_load(ctx, &_palette_start, palette_size, 0);
}

gfx_error load_tiles(gfx_context* ctx, gfx_tileset_options* options)
{
    const size_t size = &_tiles_end - &_tiles_start;
    return gfx_tileset_load(ctx, &_tiles_start, size, options);
}

zos_err_t load_zmt(track_t* track, uint8_t index)
{
    zmt_reset(VOL_50);
    switch (index) {
        case 0: {
            const size_t size = &_zmt_track1_end - &_zmt_track1_start;
            return zmt_rom_load(track, &_zmt_track1_start, size);
        } break;
        case 1: {
            const size_t size = &_zmt_track2_end - &_zmt_track2_start;
            return zmt_rom_load(track, &_zmt_track2_start, size);
        } break;
    }
    return ERR_NOT_A_FILE;
}

void __assets__(void) __naked
{
    __asm__(
        // shared palette
        "__palette_start:\n"
        "    .incbin \"assets/bricked.ztp\"\n"
        "__palette_end:\n"

        // tiles
        "__tiles_start:\n"
        "    .incbin \"assets/bricked.zts\"\n"
        "__tiles_end:\n"

        // background music
        // Track 1 - Arkanoid
        "__zmt_track1_start:\n"
        "    .incbin \"assets/bricked.zmt\"\n"
        "__zmt_track1_end:\n"
        // Track 2 - Mario
        "__zmt_track2_start:\n"
        "    .incbin \"assets/mario.zmt\"\n"
        "__zmt_track2_end:\n");
}