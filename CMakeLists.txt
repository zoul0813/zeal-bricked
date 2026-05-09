cmake_minimum_required(VERSION 3.16)

# -------------------------------
# CMake Setup
# -------------------------------
# Application Version
set(APP_VERSION_MAJOR 0)
set(APP_VERSION_MINOR 2)
set(APP_VERSION_PATCH 0)
set(APP_NAME "bricked")
set(target_name "bricked")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

# -------------------------------
# Toolchain
# -------------------------------
set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

# -------------------------------
# Project
# -------------------------------
project(${target_name} C ASM)

# -------------------------------
# Executable
# -------------------------------
add_executable(${target_name}
    src/main.c
    src/assets.c
    src/ball.c
    src/bricks.c
    src/levels.c
    src/player.c
)

# # -------------------------------
# # Include and link SDKs
# # -------------------------------
zos_use(zvb)
find_package(ZVB_SDK REQUIRED)
target_link_libraries(${target_name} PUBLIC zvb)

zos_use(zgdk)
find_package(ZGDK REQUIRED)
target_link_libraries(${target_name} PUBLIC zgdk)

# -------------------------------
# Assets
# -------------------------------

gif2zeal(${target_name} FILES
    "assets/bricked.gif"
    COMPRESSED
    )


add_asset(${target_name} FILES
    "assets/bricked.zmt"
    "assets/mario.zmt"
    )


# -------------------------------
# Convert to binary
# -------------------------------
zgdk_add_outputs(${target_name})
