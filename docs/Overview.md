# Project Overview — (Working Title: SpaceDrift: Mine & Defend)

## One-liner
A top-down 2D arcade space game where you freely roam infinite sectors, mine asteroids with a heat-limited beam laser, haul resources back to a portable base, and survive time-based pirate waves with factory-defense automation — with a built-in RL environment for training agents (DQN).

---

## Core Pillars
- **Arcade Top-Down Flight:** WASD movement, mouse aim; responsive, low-friction control.
- **Beam Mining + Heat Management:** hold beam to heat/crack asteroids; overheat triggers cooldown.
- **Portable Base + Factory-Defense:** dock, unload, refine, build modules (turrets/shields/power/industry).
- **Endless Progression:** infinite sectors + stations; incremental upgrades + meta-progression via research.
- **AI-Ready by Design:** deterministic headless simulation mode + step/reset API for RL training.

---

## Target Player Experience
- **Minute-to-minute:** roam → find asteroids → beam mine (manage heat) → fill cargo → return to base.
- **Mid-loop:** unload → refine → build/upgrade modules → extend survivability and mining efficiency.
- **Pressure cycle:** pirate waves arrive on a timer; defense performance determines long-run success.
- **Long-run:** research unlocks permanent blueprints; runs get progressively deeper and more optimized.

---

## Core Gameplay Loop
1. **Explore** an infinite sector map (procedural spawn).
2. **Mine** asteroids (Stone/Metal/Ice) using a beam laser.
3. **Manage Heat:** laser increases heat; overheat causes forced cooldown.
4. **Cargo Constraint:** ship has limited cargo; full cargo forces a base return.
5. **Dock & Process:** unload, refine materials, convert into buildable resources.
6. **Build & Upgrade:** add base modules (power/defense/industry/tech).
7. **Defend:** time-based pirate waves attack the base; turrets + automation handle most combat.
8. **Repeat** with increased efficiency and higher threat level.

---

## World & Content
### Space & Sectors
- **Infinite/procedural sectors** with continuous asteroid encounters.
- **Stations** act as hubs: market + research unlocks.

### Asteroids (MVP)
- **Stone:** common; basic materials.
- **Metal:** mid rarity; used for core upgrades and turrets.
- **Ice:** used for cooling/energy tech paths (and/or “crew sustenance” tech bonuses).

### Enemies (MVP)
- **Pirate Scout:** fast, light damage, pressure unit.
- **Pirate Bomber:** slower, heavier damage, base threat.

---

## Player & Controls
- **Movement:** WASD (arcade, instant-ish response).
- **Aim:** mouse.
- **Mining:** auto-aim during mining (locks to nearest valid asteroid target).
- **Boost:** enabled; dash optional (design hook for later).

---

## Mining System
### Beam Laser
- Hold beam on asteroid to **heat and break** it.
- Mining output depends on asteroid type + yield table.

### Heat / Overheat
- Beam increases ship heat.
- Exceed threshold → **forced cooldown** (no mining/fire) until recovered.
- Upgrades can improve heat capacity, cooling rate, and beam efficiency.

### Cargo Constraint
- Limited cargo capacity.
- When cargo is full, player must return to base to unload (hard constraint to enforce loop).

---

## Portable Base System
### Base Concept
- Base is **portable** (implementation: relocate/warp/pack mode TBD).
- Enter/dock to open **upgrade/build menu**.
- Outside docking: base operates as an automated defense/factory node.

### Base Modules (MVP Set)
**Power**
- Generator
- Battery (capacity buffer)

**Defense**
- Turret Mk1 (general)
- Turret Mk2 (anti-fast or higher DPS)
- Shield/Charge module (simple protective layer / burst defense)

**Industry**
- Storage
- Refinery (process ore into usable materials)
- Repair module (ship repair / base repair)

**Tech / “Sustenance”**
- A progression line that provides passive buffs (cooling, refine speed, power efficiency, etc.)

### Energy Constraint (Key Design Rule)
- Each module consumes power.
- Total module count and strength is bounded by **power capacity**, driving meaningful incremental upgrades.

---

## Waves & Difficulty Scaling
- **Wave trigger:** time-based (every T seconds).
- **Threat level:** grows over time.
- **Wave composition:** enemy count + ratios scale with threat.
- **Lose condition:** base HP reaches 0.
- **Death handling:** if player dies while base is alive → respawn at base.

---

## Progression
### Run Progression (Incremental)
- Ship: beam power, cooling, heat capacity, cargo capacity, boost efficiency.
- Base: power capacity, turret DPS/range, shield strength, refinery speed, storage size.

### Meta-Progression (Persistent)
- Research at stations unlocks **blueprints**.
- Tech points earned from survival time + refined value + rare finds.

---

## AI / RL Mode (DQN-Ready)
### Goal
Provide a built-in environment for training agents on:
- **Mining optimization** (heat + cargo + return routing)
- **Defense optimization** (positioning/targeting/priority)

### Simulation Requirements
- **Headless mode:** render off, high-FPS stepping.
- **Deterministic seeds:** reproducible procedural generation for benchmarking.
- **API surface:** `reset(seed)`, `step(action)` → `(obs, reward, done, info)`.

### Observation (Initial Vector Proposal)
- Ship: position, velocity (optional), facing, heat, cooldown remaining, cargo ratio
- Nearest asteroid: type one-hot, distance, relative angle, remaining HP
- Base: distance, relative angle, base HP ratio
- Nearest enemy (if any): type one-hot, distance, relative angle

### Action Space (Discrete-Friendly)
- Movement: 8 directions + idle
- Beam: on/off
- Boost: on/off
- (Optional) context aim snap: asteroid/enemy/base direction

### Reward Shaping (Initial)
- + mined value (ideally on deposit/refine to avoid exploit)
- - overheat penalty
- - idle/time penalty
- + successful deposit bonus
- - base damage penalty
- + wave survived bonus

---

## Technical Direction (C + Custom Engine)
### Engine Priorities
- Fixed timestep simulation (e.g., 60Hz), render decoupled
- Entity-component style data layout (arrays) for performance and simplicity
- Spatial partitioning (grid/hash) for nearest-object queries
- Data-driven tuning via JSON/TOML (asteroids, modules, enemies, waves)
- Save/load for meta-progression

---

## MVP Scope (Vertical Slice Checklist)
- Top-down arcade movement + mouse aim
- Beam mining with heat + cooldown
- 3 asteroid types + basic yield tables
- Cargo capacity + forced return loop
- Portable base with dock UI (basic build menu)
- Refinery + storage + generator + battery
- 2 turret types + base HP + simple shield/charge module
- Time-based waves with 2 enemy types
- 1 station: market + research unlock (basic meta-progression)
- Headless sim + step/reset hooks (minimal RL integration)

---

## Open Decisions (To Lock Next)
1. Base relocation/portability mechanic (warp cooldown? fuel? pack mode?)
2. Docking interaction (auto vs keypress; docking radius)
3. Turret targeting (fully auto vs player-set priority)
4. Station economy depth (simple buy/research vs contracts/missions)

---

## Success Metrics
- 5–10 minutes: player understands loop without tutorial walls.
- 30 minutes: meaningful upgrade path and escalating waves feel fair.
- 60+ minutes: meta-progression provides long-term purpose.
- RL mode: stable training runs with deterministic replay and measurable improvement.

---
