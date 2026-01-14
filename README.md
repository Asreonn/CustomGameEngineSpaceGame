# Custom Game Engine Space Game

Lightweight C/Raylib space prototype focused on a modular codebase: player, planets, spritesheets, asteroids, and beam mining.

## Features
- Top-down ship movement with mouse aim + RMB boost
- Infinite tiled background with a bounded starter map
- Animated planet spritesheet (500x500 grid frames)
- Asteroid field with random drift, pixel-perfect collisions, and despawn
- Auto beam mining within range + HP damage + minimal damage popups
- Mouse wheel zoom

## Build & Run
```bash
cmake -S . -B build
cmake --build build
./build/space_game
```

Or use the helper script:
```bash
./run.sh
```

## Controls
- Move: WASD / Arrow keys
- Aim: Mouse
- Boost: Right mouse button
- Zoom: Mouse wheel

## Project Structure
```
src/
  main.c           - game loop + wiring
  player.c/.h      - ship movement + engine effects
  planet.c/.h      - planet spritesheet animation
  spritesheet.c/.h - spritesheet + animation helpers
  asteroids.c/.h   - asteroids, masks, collisions, popups
Assets/
  Textures/        - all 2D art assets
docs/
  Overview.md      - design overview
  Stats.md         - tuning reference
```

## Notes
- Asteroid collisions use cached alpha masks (per texture) for pixel-perfect overlap.
- Beam is procedurally generated (no external texture needed).

