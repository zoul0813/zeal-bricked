cmake_minimum_required(VERSION 3.16)

# -------------------------------
# Toolchain
# -------------------------------
set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

# -------------------------------
# Project
# -------------------------------
project(bricked C ASM)

# -------------------------------
# Executable
# -------------------------------
add_executable(bricked
    src/bricked.c
    src/assets.c
    src/ball.c
    src/bricks.c
    src/levels.c
    src/player.c
)

# # -------------------------------
# # Include and link SDKs
# # -------------------------------
zos_use(zgdk)
zos_use(zvb)

find_package(ZVB_SDK REQUIRED)
target_link_libraries(bricked PUBLIC zvb_gfx zvb_sound)

find_package(ZGDK REQUIRED)
target_link_libraries(bricked PUBLIC zgdk)

# -------------------------------
# Assets
# -------------------------------

gif2zeal(bricked FILES
    "assets/bricked.gif"
    COMPRESSED
    )


add_asset(bricked FILES
    "assets/bricked.zmt"
    "assets/mario.zmt"
    )


# -------------------------------
# Convert to binary
# -------------------------------
zos_add_outputs(bricked)
