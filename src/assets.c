#include <zos_errors.h>
#include <zvb_gfx.h>
#include <zgdk/sound/tracker.h>
#include <zgdk/assets.h>
#include "assets.h"

gfx_error load_palette(gfx_context* ctx)
{
    // Load the palette
    const size_t palette_size = &_palette_end - &_palette_start;
    return gfx_palette_load(ctx, &_palette_start, palette_size, 0);
}

gfx_error load_tiles(gfx_context* ctx)
{
    const size_t size = &_tiles_end - &_tiles_start;
    return gfx_tileset_load_rle(ctx, &_tiles_start, size, 0, 0, 0);
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
    INCLUDE_ASSET("palette", "assets/bricked.ztp");
    INCLUDE_ASSET("tiles", "assets/bricked.zts");
    INCLUDE_ASSET("zmt_track1", "assets/bricked.zmt");
    INCLUDE_ASSET("zmt_track2", "assets/mario.zmt");
}
