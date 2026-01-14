# C Game Engine Plan — Rules & Required Modules (for SpaceDrift)

> Purpose: A practical “engine rulebook + module map” for building a custom C engine that can ship this game **and** support headless RL simulation.
> This is not a task list; it’s **how the engine should be structured** and **which modules must exist**.

---

## 0) Non-Negotiables (Engine Rules)

### Determinism & Repeatability
- The simulation must be able to run with a **fixed seed** and produce identical results in headless mode.
- All randomness goes through a single **RNG service**: `rng_next_u32(seed_state*)`.
- No frame-time dependent physics in gameplay logic; use **fixed timestep**.

### Fixed Timestep Simulation
- Simulation runs at `SIM_DT = 1/60` (or 1/120 for smoother mining).
- Render is decoupled from sim; render can interpolate using `alpha`.
- Never put gameplay state changes in render code.

### Data-Oriented Layout
- Prefer **SoA** (Structure of Arrays) for hot components (pos/vel/hp).
- Avoid deep pointer trees; keep memory contiguous.
- One global allocator policy: arenas for transient, pools for entities, heap only for assets.

### Separation of Concerns
- **Core** doesn’t know about game-specific rules.
- **Game** layer implements: mining, waves, modules, economy.
- Engine provides: input, render, audio, file IO, entity storage, timing, math, assets.

### Headless Mode First-Class
- Engine must run with:
  - `--headless` (no window, no GPU calls)
  - `--steps N` (simulate N steps)
  - `--seed X` (determinism)
- This is mandatory for RL training throughput.

### No Hidden Global State
- One “world” struct owns all state:
  - `World` contains entity arrays, systems, time, RNG, event queues.
- Systems operate on `World*` only.

---

## 1) Engine Layer Map (Modules You Must Have)

### 1.1 Platform & Core
**Platform Abstraction**
- Window creation / message pump
- High-res timer
- File IO (read/write)
- Threading primitives (mutex, atomic) — optional MVP
- Controller/gamepad optional later

**Core Utilities**
- Logging (`log_info/warn/error`)
- Assertions & crash handler
- Profiling markers (even minimal)
- Command line flags + config loading

---

### 1.2 Memory System
**Allocators**
- Linear arena (frame/temp allocations)
- Pool allocator (entities, bullets, particles)
- Asset allocator (long-lived)

**Rules**
- Every module documents its allocation strategy.
- No silent `malloc` inside hot loops.

---

### 1.3 Math & Geometry
**Required**
- `Vec2`, `Mat3` (2D transforms), `Rect`, `AABB`, `Ray2D`
- Angle helpers, lerp, clamp, smoothstep
- Spatial queries need solid math (nearest, dot, normalize)

---

### 1.4 Time & Game Loop
**Game Loop Contract**
- `engine_tick(real_dt)` accumulates into fixed sim steps:
  - `while (accum >= SIM_DT) { sim_step(SIM_DT); accum -= SIM_DT; }`
- `render(alpha)` uses interpolation for visuals only

**Event Timing**
- A small scheduler for timed events (wave timers, cooldown timers)

---

### 1.5 Input
**Input Layer**
- Action mapping (WASD, mouse, buttons)
- Edge detection (pressed/released)
- Mouse world position conversion

**Rule**
- Input is sampled once per frame; sim consumes stable input snapshots.

---

### 1.6 Rendering (2D)
**Renderer Requirements**
- Sprite batching (texture atlas-friendly)
- Camera2D (position, zoom)
- Basic primitives (lines, circles) for debug
- Text rendering (debug HUD)

**Rule**
- Renderer never mutates gameplay.
- Debug draw is optional but strongly recommended.

---

### 1.7 Audio (Minimal)
- One-shot SFX (laser, explosions)
- Looping ambience/music
- Simple mixer; advanced DSP not required

---

### 1.8 Assets & Content Pipeline
**Asset Types**
- Textures (sprites)
- Fonts
- Audio clips
- Data configs (JSON/TOML)

**Hot Reload (Nice-to-have)**
- Reload data configs without restart (huge balancing speed)

---

### 1.9 Entity & Component Storage
You don’t need full ECS, but you need ECS-like data rules.

**Minimum**
- Entity IDs (index + generation)
- Component arrays (pos, vel, health, faction, etc.)
- Free-list for destroyed entities
- Query helpers (iterate all ships, all asteroids, etc.)

**Rules**
- Systems are pure functions: `system_update(World*, dt)`
- No cross-system direct calls that cause ordering chaos; use events.

---

### 1.10 Physics / Collision (2D Simple)
**MVP Collision**
- Broadphase: uniform grid / spatial hash
- Narrowphase: circle vs circle, circle vs AABB
- Raycast for beam mining (beam → asteroid hit)

**Rules**
- Arcade movement: no heavy rigid body sim required.
- Collision responses should be deterministic and stable.

---

### 1.11 Spatial Partitioning (Critical for this Game)
**Need**
- Spatial hash grid keyed by cell coords
- Insert/move/remove entities per sim step
- Queries:
  - nearest asteroid
  - enemies within turret range
  - objects within beam range

This single module makes everything fast and clean.

---

### 1.12 Save/Load
**MVP**
- Save meta-progression (blueprints, tech points)
- Save basic run snapshot optional
- Use versioned binary or JSON with version tags

---

## 2) Game Layer Modules (Specific to This Game)

### 2.1 World Generation (Infinite Sectors)
- Procedural spawner using deterministic RNG
- Streaming strategy:
  - keep entities within active radius
  - despawn far entities
- Stations as special spawn nodes

### 2.2 Ship Controller
- Arcade movement system
- Boost system
- Cargo rules
- Death/respawn at base

### 2.3 Mining System
- Beam raycast → asteroid hit
- Asteroid HP / hardness / yield table
- Heat accumulation → overheat → cooldown
- Auto-aim assist during mining

### 2.4 Base System (Portable)
- Dock detection + UI trigger
- Power grid (capacity, module power draw)
- Module placement (grid or sockets)
- Build menu data driven

### 2.5 Industry (Refinery & Storage)
- Input ore → output ingots
- Processing queues
- Inventory model (stacked resources)

### 2.6 Defense (Turrets & Shield)
- Turret targeting (nearest/priority)
- Projectile system (or hitscan)
- Shield HP + recharge rules
- Base HP, lose condition

### 2.7 Enemy AI (Pirates)
- Steering: seek base, orbit, attack range behavior
- Two types MVP: scout + bomber
- Spawned via wave system

### 2.8 Wave Director
- Timer-based waves
- Threat curve over time
- Budget-based spawn selection

### 2.9 Station Economy + Research
- Shop items & costs
- Blueprint unlock tree
- Meta-progression persistence

---

## 3) Data-Driven Design Rules (So Balancing Is Fast)

### Config Files
- `asteroids.*`: type, hardness, thermal, yield tables
- `enemies.*`: hp, speed, dmg, rate, rewards
- `modules.*`: power draw, build cost, stats
- `waves.*`: interval, threat step, budgets, mixes
- `upgrades.*`: per-level deltas

### Rule
- The game must be playable with **only config edits** for balancing, no recompiles.

---

## 4) Debug & Tooling (Mandatory for Not Losing Your Mind)

### In-Game Debug Overlay
- FPS, sim steps, entity counts
- RNG seed display
- Player heat/cargo/beam target
- Wave timer + threat level

### Debug Commands (Console)
- spawn asteroid/enemy
- give resources
- toggle god mode
- teleport to station/base
- freeze time / step one sim tick

---

## 5) RL / Headless Interface Contract

### Execution Modes
- `--play` normal game
- `--headless` no window
- `--rl` exposes step/reset interface

### RL API (Conceptual)
- `env_reset(seed) -> obs`
- `env_step(action) -> obs, reward, done, info`

### Observations
- Stored in a fixed-size float buffer for speed:
  - `float obs[OBS_DIM];`
- Actions are discrete ints:
  - `int action;`

### Rule
- RL mode must not depend on renderer, audio, or OS input.

---

## 6) Ordering & System Pipeline (Canonical Update Order)

1. Input snapshot (play mode only)
2. Spawn/Despawn streaming (infinite world)
3. Player movement + boost
4. Enemy steering
5. Beam mining (raycast + heat)
6. Cargo + docking + unloading
7. Industry processing (refinery)
8. Target acquisition (turrets)
9. Combat resolution (projectiles/hits)
10. Damage, deaths, despawns
11. Wave director tick (timers/spawns)
12. Save meta (on events)
13. Render (if not headless)

**Rule:** This order stays consistent to keep determinism and reduce bugs.

---

## 7) “Good Engine Hygiene” Checklist (Rules of Thumb)
- Every system has:
  - deterministic inputs
  - no hidden IO
  - clear ownership of data
- Never mix:
  - simulation state with rendering state
- Use:
  - event queues for “spawn”, “damage”, “deposit”, “upgrade bought”
- Keep:
  - unit tests for math, RNG, serialization (tiny but high value)

---

