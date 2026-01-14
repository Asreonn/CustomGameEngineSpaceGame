cmake_minimum_required(VERSION 3.16)
project(space_game C)

set(CMAKE_C_STANDARD 99)

find_package(raylib 4.0 REQUIRED)

add_executable(space_game
    src/main.c
    src/player.c
    src/planet.c
    src/spritesheet.c
    src/asteroids.c
)

target_link_libraries(space_game raylib m)
