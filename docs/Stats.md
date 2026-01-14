# Player & Enemy Stats + Upgrade Catalog (MVP → Scalable)

> Intent: Define **baseline combat/mining stats** + a **clear upgrade surface** that drives the loop:
> mine → deposit → build/upgrade → survive longer → mine faster.

---

## 1) Common Stat Conventions

### Units
- **Distance:** world units (WU)
- **Speed:** WU/sec
- **Time:** seconds
- **Damage:** HP per shot (or DPS if continuous)
- **Heat:** 0..100 normalized

### Scaling Philosophy
- **Difficulty** scales mainly via **enemy budget/counts** (stable stats = fair feel).
- **Power** scales via **incremental upgrades** (many small steps) + **meta blueprints**.

---

## 2) Player (Ship) — Base Stats (MVP)

### Survivability
| Stat | Key | Default | Notes |
|---|---|---:|---|
| Max HP | `hp_max` | 100 | If 0 → respawn at base (if base alive) |
| HP Regen | `hp_regen` | 0 | MVP: none (base repair module) |

### Movement (Arcade)
| Stat | Key | Default | Notes |
|---|---|---:|---|
| Max Speed | `speed_max` | 9.0 | Responsive roaming |
| Acceleration | `accel` | 45.0 | |
| Deceleration | `decel` | 60.0 | |
| Boost Mult | `boost_mult` | 1.6 | While boost held |
| Boost Duration | `boost_duration` | 2.0 | Optional stamina model |
| Boost Cooldown | `boost_cd` | 3.0 | Optional |

### Cargo
| Stat | Key | Default | Notes |
|---|---|---:|---|
| Cargo Capacity | `cargo_cap` | 100 | Full → must return base |
| Cargo Speed Penalty | `cargo_speed_penalty` | 0.15 | At 100% cargo, -15% speed |

### Mining (Beam + Heat)
| Stat | Key | Default | Notes |
|---|---|---:|---|
| Beam Range | `beam_range` | 7.5 | WU |
| Beam Power | `beam_power` | 18 | Mining DPS vs asteroid HP |
| Beam Tick | `beam_tick` | 10 | ticks/sec |
| Mining Auto-Aim Cone | `mining_aim_cone_deg` | 18 | Auto lock helper |
| Heat Max | `heat_max` | 100 | normalized |
| Heat Gain/sec | `heat_gain` | 22 | while beam active |
| Heat Cool/sec | `heat_cool` | 16 | passive |
| Overheat Cooldown | `overheat_cd` | 2.8 | forced downtime |

### Ship Combat (Minimal / Optional)
> Base is primary defense. Ship gun is an emergency tool.
| Stat | Key | Default |
|---|---|---:|
| Ship Gun Enabled | `ship_gun` | true |
| Ship Gun DPS | `ship_gun_dps` | 6 |
| Ship Gun Range | `ship_gun_range` | 9 |
| Ship Gun Rate | `ship_gun_rate` | 3 |

---

## 3) Enemy Stats (MVP)

### Enemy 01 — Pirate Scout
| Stat | Key | Default |
|---|---|---:|
| Max HP | `hp_max` | 30 |
| Speed | `speed` | 7.8 |
| Turn Rate | `turn_rate` | 720 |
| Attack Range | `atk_range` | 8.5 |
| Damage/Shot | `dmg` | 3 |
| Fire Rate | `rate` | 1.6 |
| Reward Value | `reward` | 6 |

### Enemy 02 — Pirate Bomber
| Stat | Key | Default |
|---|---|---:|
| Max HP | `hp_max` | 85 |
| Speed | `speed` | 4.6 |
| Turn Rate | `turn_rate` | 360 |
| Attack Range | `atk_range` | 10.0 |
| Damage/Shot | `dmg` | 9 |
| Fire Rate | `rate` | 0.7 |
| Reward Value | `reward` | 14 |

---

## 4) Base Core Stats (Needed for Loop)

| Stat | Key | Default | Notes |
|---|---|---:|---|
| Base Max HP | `base_hp_max` | 500 | Lose at 0 |
| Dock Radius | `dock_radius` | 6.0 | Auto dock threshold |
| Power Capacity | `power_cap` | 60 | Module limit driver |

---

## 5) Turrets & Defense Modules (MVP)

### Turret Mk1 — General
| Stat | Key | Default |
|---|---|---:|
| DPS | `dps` | 14 |
| Range | `range` | 11 |
| Rate | `rate` | 2.0 |
| Power Draw | `power` | 8 |
| Targeting | `targeting` | nearest |

### Turret Mk2 — Anti-Fast
| Stat | Key | Default |
|---|---|---:|
| DPS | `dps` | 10 |
| Range | `range` | 13 |
| Rate | `rate` | 4.0 |
| Power Draw | `power` | 10 |
| Targeting | `targeting` | fast/low_hp (TBD) |

### Shield / Charge Module
| Stat | Key | Default | Notes |
|---|---|---:|---|
| Shield HP | `shield_hp` | 180 | Buffer layer |
| Recharge Delay | `recharge_delay` | 3.0 | After damage |
| Recharge Rate | `recharge_rate` | 25 | HP/sec |
| Power Draw | `power` | 12 | |

---

## 6) Upgrade Catalog (Ship + Base) — What Can Be Upgraded?

> Upgrades are **incremental** (many small steps).
> Each upgrade has levels: `L1..L10` (tunable), costs scale exponentially-ish.

### 6.1 Ship Upgrades (Run Progression)

#### A) Mining Efficiency
- **Beam Power** (`beam_power`): +X% mining DPS per level  
- **Beam Range** (`beam_range`): +0.2–0.5 WU per level  
- **Auto-Aim Assist** (`mining_aim_cone_deg`): +1–2° per level (cap to avoid autopilot)
- **Ore Extractor Yield** (new stat): +% chance for extra drop / higher refine output

#### B) Heat / Cooling
- **Heat Capacity** (`heat_max`): +5..10 per level (or +% if normalized)
- **Heat Gain Reduction** (`heat_gain`): -X% per level
- **Cooling Rate** (`heat_cool`): +X% per level
- **Overheat Cooldown** (`overheat_cd`): -X% per level
- **Emergency Vent** (active): instantly dumps heat but disables beam for N sec (late-game tool)

#### C) Logistics & Uptime
- **Cargo Capacity** (`cargo_cap`): +10..25 per level
- **Cargo Handling** (new): faster unload at base / reduced dock time
- **Thruster Efficiency**: reduce cargo speed penalty (`cargo_speed_penalty`)

#### D) Mobility
- **Max Speed** (`speed_max`): +X% per level
- **Acceleration/Deceleration** (`accel`, `decel`): +X% per level
- **Boost System**
  - `boost_mult` +X%
  - `boost_duration` +X
  - `boost_cd` -X%
- **Dash Module** (optional unlock): short burst with cooldown (skill expression)

#### E) Survivability
- **Max HP** (`hp_max`): +10..20 per level
- **Damage Reduction** (new): flat armor or % reduction (keep simple)
- **Repair Drone** (passive): slow regen only when near base (avoids infinite sustain in field)

#### F) Ship Combat (If Enabled)
- **Gun DPS/Rate/Range** (`ship_gun_dps`, `ship_gun_rate`, `ship_gun_range`)
- **Targeting Computer**: soft auto-aim in combat (optional, late)
> Recommendation: keep ship combat upgrades weaker than base defense upgrades.

---

### 6.2 Base Upgrades (Factory-Defense Progression)

#### A) Power Grid (Hard Gate)
- **Generator Output**: increases `power_cap`
- **Battery Capacity**: buffer to prevent power dips
- **Power Efficiency**: reduces module `power` draw by X%

#### B) Defense
- **Turret Mk1/Mk2**
  - DPS, Range, Fire Rate
  - Tracking speed / target switch delay (optional)
- **Shield Module**
  - Shield HP, Recharge rate, Recharge delay
- **Repair Module**
  - Base repair rate (HP/sec)
  - Ship repair rate while docked
- **Decoy / Jammer (post-MVP)**
  - pulls aggro / disrupts pirates

#### C) Industry
- **Refinery Speed**: ore → ingot throughput
- **Refinery Efficiency**: +% output (less waste)
- **Storage Capacity**: buffer for long runs
- **Auto-Sorter**: prioritizes rare ore first (quality-of-life + strategy)

#### D) Tech / Research / “Sustenance”
- **Research Speed**: faster blueprint unlocks
- **Global Buff Nodes** (choose 1 per tier)
  - Cooling bonus
  - Refinery bonus
  - Turret efficiency
  - Cargo bonus
> This becomes your “build identity” system.

#### E) Portability (Your Key Fantasy)
- **Relocation Cooldown**: lower cooldown or cost
- **Relocation Safety**: temporary shield after moving
- **Deployment Speed**: faster “pack/unpack” time

---

## 7) Meta-Progression (Persistent Upgrades)

> Earn Tech Points from survival + refined value + rare finds.
- **Blueprint Unlocks:** new modules (Mk2 turret, shield, vent system, dash)
- **Permanent Perks (small):**
  - +5% cooling baseline
  - +5 cargo baseline
  - +2% refinery yield baseline
- **Station Research Tree:** unlock tiers and specialization branches

---

## 8) Wave Scaling (Starter Defaults)

### Timing
- `WAVE_INTERVAL = 75s`
- `THREAT_STEP_SEC = 120s`

### Budget
- `budget = 8 + ThreatLevel * 3`
- Scout cost = 1, Bomber cost = 3

### Mix
- Early: 80/20 (Scout/Bomber)
- Mid: 60/40
- Late: 45/55

---

## 9) Implementation Notes (So It Stays Tunable)
- Keep all numeric stats in **data files**.
- Separate “upgrade level → stat delta” tables:
  - linear for QoL (range, cargo)
  - mild exponential for power spikes (DPS, power cap)
- Avoid RNG-heavy combat in MVP (helps fairness + RL training stability).

---
