# Custom Data Reference

This document catalogs all custom variables, flags, script specials, maps, and constants added by Pokémon Helix on top of pokeemerald-expansion.

## Custom Variables

Defined in `include/constants/vars.h`:

| Variable | Address | Purpose | Set By | Read By |
|----------|---------|---------|--------|---------|
| `VAR_HELIX_PLAYER_TYPE` | `0x40F7` | Assigned Pokémon type (0–17) | `HelixApplyNewGameState` | HelixIsland food sign, any future type-dependent logic |
| `VAR_HELIX_FOOD` | `0x40F8` | Current food supply | `HelixApplyNewGameState` (init: `HELIX_STARTING_FOOD` = 200 in the demo build), HelixIsland sleep script | HelixIsland sleep script, new-day message |
| `VAR_HELIX_DAY_COUNT` | `0x40F9` | Current day number | `HelixApplyNewGameState` (init: 1), HelixIsland sleep script | HelixIsland food sign, sleep script |
| `VAR_HELIX_INTRO_STATE` | `0x40FA` | Tutorial state machine (2 = needs tutorial, 3 = ball pickup phase, 4 = done) | `HelixApplyNewGameState` (init: 2), tutorial script | `MAP_SCRIPT_ON_FRAME_TABLE`, coord_event boundaries, guide interaction |
| `VAR_HELIX_ISLAND_POP` | `0x40FB` | Island population count | `HelixApplyNewGameState` (init: 2), `HelixSpecial_CountPopulation` | Food consumption, sleep script |
| `VAR_HELIX_COMFORT` | `0x40FC` | Island comfort level (affects breeding) | Breeding system | Breeding chance modifier |
| `VAR_HELIX_STRAY_SPECIES` | `0x40FD` | Species of today's stray Pokémon | `HelixSpecial_GenerateStray`, `HelixSpecial_GenerateDay1Stray` | Stray interaction script (`showmonpic`, `bufferspeciesname`), `HelixSpecial_GiveStray` |
| `VAR_HELIX_STRAY_ABILITY_OVERRIDE` | `0x40FE` | Stray ability slot override (0xFF = default) | `HelixSpecial_GenerateStray`, `HelixSpecial_GenerateDay1Stray` | `HelixSpecial_GiveStray` |
| `VAR_HELIX_STRAY_GENDER` | `0x40FF` | Stray gender override (MON_GENDER_RANDOM = no override) | `HelixSpecial_GenerateStray`, `HelixSpecial_GenerateDay1Stray` | `HelixSpecial_GiveStray` |

### Initial Values (set by `HelixApplyNewGameState`)

```
VAR_HELIX_PLAYER_TYPE  = (computed from quiz)
VAR_HELIX_FOOD         = 200  (HELIX_STARTING_FOOD — demo value, was 20)
VAR_HELIX_DAY_COUNT    = 1
VAR_HELIX_ISLAND_POP   = 2
VAR_HELIX_INTRO_STATE  = 2
VAR_HELIX_COMFORT      = 50   (HELIX_DEFAULT_COMFORT)
```

`HelixApplyNewGameState` also sets `FLAG_SYS_POKEMON_GET`, `FLAG_SYS_POKEDEX_GET`, and `FLAG_SYS_B_DASH`.

### Notes

- All vars are `u16` (0–65535). Subtraction below 0 causes unsigned underflow — this was the root cause of the food display bug before the stash/apply fix.
- `VAR_HELIX_ISLAND_POP` is updated dynamically by `HelixSpecial_CountPopulation` during the sleep script.
- Food consumption uses `VAR_HELIX_ISLAND_POP` via `subvar`.

## Flags

Set by `HelixApplyNewGameState` during new game init:

| Flag | Purpose |
|------|---------|
| `FLAG_SYS_POKEMON_GET` | Player has Pokémon (enables party menu) |
| `FLAG_SYS_POKEDEX_GET` | Player has Pokédex |
| `FLAG_SYS_B_DASH` | Player can run (B button dash) |

### Custom Helix flags

Defined in `include/constants/flags.h`:

| Flag | Address | Purpose |
|------|---------|--------|
| `FLAG_HELIX_STRAY_HIDE_COMMON` | `0x493` | Hides the common (Poké Ball) stray object. |
| `FLAG_HELIX_STRAY_HIDE_UNCOMMON` | `0x494` | Hides the uncommon (Great Ball) stray object. |
| `FLAG_HELIX_STRAY_HIDE_RARE` | `0x495` | Hides the rare (Ultra Ball) stray object. |
| `FLAG_HELIX_STRAY_HIDE_LEGENDARY` | `0x496` | Hides the legendary (Master Ball) stray object. |
| `FLAG_HELIX_STRAY_MSG_SHOWN` | `0x497` | Tracks whether the "washed up on shore" intro line has been shown today. Cleared each day by the sleep script. |
| `FLAG_HELIX_TUTORIAL_DONE` | `0x498` | Set when day-1 tutorial completes. Gates the guide's day-2+ boat prompt dialogue. |

## Script Specials

Registered in `data/specials.inc`:

| Special | Purpose | Used By |
|---------|---------|---------|
| `HelixSpecial_BufferTypeName` | Copies the player's type name to `STR_VAR_1` | Available for future use (food sign removed) |
| `HelixSpecial_BufferStarterNames` | Copies all 3 starter names to `STR_VAR_1/2/3` | Available for future use |
| `HelixSpecial_CountPopulation` | Counts alive party + PC Pokémon, stores in `VAR_HELIX_ISLAND_POP` | Sleep script (food check + consumption) |
| `HelixSpecial_HatchEggs` | Clears egg flag, sets nickname to species name, and sets language on all eggs in PC boxes (moveset baked in at creation) | Sleep script |
| `HelixSpecial_BreedBoxes` | Mother-driven breeding: females then genderless roll for eggs, pairing with males/genderless in the same box. Egg creation applies full inheritance (species, nature, IVs, shiny, ability, egg moves) | Sleep script |
| `HelixSpecial_GenerateStray` | Rolls rarity (70/22/7/1), picks species from per-type pool, manages 4 hide flags, spawns the correct ball object | Sleep script |
| `HelixSpecial_GenerateDay1Stray` | Picks uncommon slot 0 for the player's type; forces opposite gender if both starters are same-gender; spawns uncommon ball | `OnTransition` (first visit only) |
| `HelixSpecial_GiveStray` | Creates stray mon with neutral nature, ability override, gender override; gives to party/PC | Stray interaction script |
| `HelixSpecial_SetGuideGraphics` | Sets guide NPC sprite based on `VAR_HELIX_PLAYER_TYPE` via 18-entry lookup table (unique sprite per type); updates both object event template and live object event | `MAP_SCRIPT_ON_TRANSITION` |

### Removed Specials (dead code, cleaned up)

- `HelixSpecial_GiveStarters` — replaced by `HelixApplyNewGameState`
- `HelixSpecial_ComputeTypeFromQuiz` — replaced by direct C call in intro cutscene

## Constants

Defined in `include/helix.h`:

| Constant | Value | Purpose |
|----------|-------|---------|
| `HELIX_TYPE_NORMAL` through `HELIX_TYPE_FAIRY` | 0–17 | Type IDs matching `constants/types.h` |
| `HELIX_NUM_TYPES` | 18 | Total type count |
| `HELIX_STARTERS_PER_TYPE` | 3 | Species per starter pool |
| `HELIX_STARTING_FOOD` | 200 | Initial food supply (**demo value** — was 20) |
| `HELIX_QUIZ_ANSWERS` | 3 | Choices per quiz question |
| `HELIX_QUIZ_COMBOS` | 27 | Total quiz answer combinations (3³) |
| `HELIX_MAX_TYPE_WEIGHTS` | 4 | Max weighted type options per combo |
| `HELIX_RARITY_COMMON` through `HELIX_RARITY_LEGENDARY` | 0–3 | Stray rarity tiers (70/22/7/1 drop rates) |
| `HELIX_NUM_RARITIES` | 4 | Total rarity count |
| `STRAY_ABILITY_DEFAULT` | 0xFF | Sentinel: use personality-based ability (no override) |
| `HELIX_IV_MAX` | 5 | Max IV per stat for starters/strays (min is always 1) |
| `HELIX_DEFAULT_COMFORT` | 50 | Default island comfort (out of 100) |
| `HELIX_BREED_CHANCE_BASE` | 90 | Base breed % per mother roll (**demo value** — was 30) |
| `HELIX_EGG_HATCH_LEVEL` | 5 | Level of hatched mons / bred eggs |

### Demo Tuning

The current build is balanced for a fast demo. Several rates are temporarily boosted and are marked with `// DEMO:` comments in `src/helix.c`:

| Value | Demo | Original | Effect |
|-------|------|----------|--------|
| `HELIX_STARTING_FOOD` | 200 | 20 | Long runway before food pressure matters |
| `HELIX_BREED_CHANCE_BASE` | 90 | 30 | Breed chance is effectively always 90% (capped), so `VAR_HELIX_COMFORT` currently has no visible effect |
| Ability inheritance roll | 25 / 15 / 30 / 30 | 50 / 30 / 10 / 10 | Far more hidden-ability and cross-species transfers |
| Egg-move category roll | 10 / 30 / 30 / 30 | 75 / 10 / 10 / 5 | Most eggs inherit egg moves |
| Same-species extra egg-move chance | 40% | 10% | More egg moves per parent |

## Maps

### HelixIsland (active)

- **Location**: `data/maps/HelixIsland/`
- **Map ID**: `MAP_HELIX_ISLAND`
- **Warp destination**: Set in `src/new_game.c` `WarpToTruck` — coordinates (41, 27)
- **Scripts**: Sleep/day cycle (`EventScript_BedNPC`), PC terminal (`EventScript_PC`), stray interaction (`EventScript_StrayBall`), stray collection (`EventScript_StrayHideAll`), tutorial cutscene (`EventScript_Tutorial`, `EventScript_TutorialPostBall`), guide interaction (`EventScript_GuideInteract`), boundary enforcement (`EventScript_BoundaryLeft`, `EventScript_BoundaryTop`)
- **Object events**: PC (shrub), 4 stray ball objects (common/uncommon/rare/legendary), Guide NPC (`LOCALID_HELIX_ISLAND_GUIDE` = 8, flag "0" = always visible, Norman sprite)
- **Coord events**: 10 boundary triggers for ball pickup zone (gated on `VAR_HELIX_INTRO_STATE == 3`)
- **BG events**: 2 bed signs at (33,25) and (33,26)
- **Map scripts**: `MAP_SCRIPT_ON_LOAD` (boat tile changes per day), `MAP_SCRIPT_ON_TRANSITION` (guide graphics/position, stray gen), `MAP_SCRIPT_ON_FRAME_TABLE` (tutorial trigger)
- **Food sign**: Removed. `HelixSpecial_BufferTypeName` is still registered but currently unused.

### Empty Party Support

`IsRemovingLastPartyMon()` in `src/pokemon_storage_system.c` always returns `FALSE`. The inline check in `CanShiftMon()` is also removed. This allows the player to deposit all party Pokémon into PC boxes, which is required for the island population model where PC boxes represent the habitat.

### HelixIntroRoom (legacy stub)

- **Location**: `data/maps/HelixIntroRoom/`
- **Status**: Unreachable in normal gameplay. Map registration kept, scripts gutted to a minimal stub.
- **History**: Originally contained the script-based quiz before it moved to `main_menu.c`.

## Ability Override (Engine Extension)

Added to `include/pokemon.h` and `src/pokemon.c`.

| Field | Location | Bits | Purpose |
|-------|----------|------|---------|
| `MON_DATA_ABILITY_OVERRIDE` | `PokemonSubstruct0` | 9 (split: `abilityOverrideLo:6` + `abilityOverrideHi:3`) | Stores a literal ability ID that overrides the species-based lookup. 0 = no override (`ABILITY_NONE`). Supports 0–511 (covers all 310+ abilities). |

When non-zero, `GetMonAbility()` returns the override directly. This is also respected by:
- `PokemonToBattleMon` (battle ability)
- `TryBoxMonFormChange` (form change checks)
- `PrintMonAbilityName` / `PrintMonAbilityDescription` (summary screen)

The override persists through evolution. Currently set only by breeding (cross-species transfer — 30% in the demo, 10% by original design), but available for any future system.

**No struct size change** — uses previously unused bits in `PokemonSubstruct0`.

## Breeding & Inheritance System

### Gender Mechanics

Breeding is **mother-driven**. Per box, per day:
1. Each **female** rolls for an egg (breed chance = `HELIX_BREED_CHANCE_BASE` + comfort/4, capped at 90%). On success, picks a random **male or genderless** partner from the same box.
2. Each **genderless** mon rolls for an egg. On success, picks a random **male or genderless** partner (excluding itself).

Each mother produces at most **one egg per day**. Males never roll — they are only selected as partners. If the box is full, no eggs are produced.

> **Demo note**: `HELIX_BREED_CHANCE_BASE` is currently 90 (was 30), so the chance is always capped at 90% and `VAR_HELIX_COMFORT` has no visible effect. The day-1 comfort boost to 240 (in the tutorial) is therefore also a no-op for now.

### Inherited Traits

| Trait | Mechanic | Chances |
|-------|----------|---------|
| Species | 50/50 from each parent | — |
| Gender | Random, determined naturally by species gender ratio | — |
| Nature | 45% parent 1, 45% parent 2, 10% fully random (any of 25 natures including non-neutral) | 45/45/10 |
| IVs | Each of 6 stats independently picks 50/50 from one parent's IV | Per-stat coin flip |
| Shiny | Roll against threshold based on parents' shiny status | No shiny parents: 1/1000. One: 1/100. Both: 1/10 |
| Ability | Identify same-species parent and other parent, then roll | **Demo:** 25% same-species parent ability, 15% natural (personality), 30% hidden (slot 2), 30% cross-species transfer (*original design: 50/30/10/10*) |
| Egg Moves | Single category roll, then per-category logic | **Demo:** 10% none, 30% random from pool, 30% same-species parent, 30% cross-species (*original design: 75/10/10/5*) |

### Ability Inheritance Detail

Percentages below are the **demo** values; original design values are in parentheses.

- **25% (50%) — Same-species parent**: If the parent has an ability override, it is passed directly. Otherwise, the parent's ability slot number is copied (same species = same ability).
- **15% (30%) — Natural**: Ability slot determined by personality (already set by `CreateMon`). No action needed.
- **30% (10%) — Hidden ability**: Sets ability slot to 2. Only applies if the species has a hidden ability defined.
- **30% (10%) — Cross-species transfer**: Resolves the other parent's actual ability. If the egg's species has it in a slot, uses that slot. If not, stores as `MON_DATA_ABILITY_OVERRIDE` (foreign ability persists through evolution).

### Egg Move Inheritance Detail

A single roll determines the category (demo values; original design in parentheses):
- **10% (75%)**: No moves inherited.
- **30% (10%)**: One random egg move from the child's species egg move pool (via `GetEggMovesBySpecies`).
- **30% (10%)**: Same-species parent transfer — first eligible egg move the parent knows is auto-inherited (the category roll IS the roll for this move). Each subsequent eligible egg move gets an independent **40% (10%)** roll. Stops at 4 moves.
- **30% (5%)**: Cross-species transfer — picks a random move from the other parent that is an egg move of that parent's species. This move won't be passed down further in future breeding (it's not in the child's egg move pool).

Egg moves are only added to empty move slots (if the mon already has 4 level-up moves, no egg moves are inherited).

### Moveset Baking

The complete moveset (level-up + egg moves) is baked into the egg at creation time via `GiveMonInitialMoveset` followed by `InheritEggMoves`. `HelixSpecial_HatchEggs` does **not** call `GiveBoxMonInitialMoveset` — it only clears the egg flag, sets the nickname to the species name, and sets the language to `gGameLanguage` so the name renders correctly.

### Bred-Egg Identity & Summary Text

Eggs created by the island breeding system are tagged so the summary screen describes them distinctly:

- **Met location**: `METLOC_HELIX_BRED` (`0xFC`, defined in `include/constants/region_map_sections.h`). Sits in the special-location value space alongside `METLOC_SPECIAL_EGG`/`METLOC_IN_GAME_TRADE`.
- **Egg nickname**: stored as the Japanese egg name `タマゴ` with `LANGUAGE_JAPANESE`, which makes the engine display the localized "Egg" name like a normal egg.
- **Summary text** (in `src/strings.c`, branched on `METLOC_HELIX_BRED` in `src/pokemon_summary_screen.c`):
  - While still an egg: `gText_HelixEggHatchTomorrow` — "This egg will hatch tomorrow!" (eggs always hatch on the next sleep).
  - After hatching: `gText_HelixBredEgg` — "Who knows what this will hatch into?" memo line.

## Structs

Defined in `include/helix.h`:

```c
struct HelixStarterPool {
    u16 species[HELIX_STARTERS_PER_TYPE];  // 3 species per type
};

struct HelixTypeWeight {
    u8 typeId;   // HELIX_TYPE_* constant
    u8 weight;   // Probability weight (0 = sentinel/end)
};

struct HelixQuizOutcome {
    struct HelixTypeWeight options[HELIX_MAX_TYPE_WEIGHTS];  // Up to 4 weighted options
};

struct StrayEntry {
    u16 species;    // SPECIES_NONE = sentinel (end of pool)
    u8  abilityNum; // 0/1/2 = force ability slot; STRAY_ABILITY_DEFAULT (0xFF) = personality-based
};
```

## C Functions (non-special)

Defined in `src/helix.c`, declared in `include/helix.h`:

| Function | Signature | Purpose |
|----------|-----------|---------|
| `HelixGetStarterSpecies` | `u16 (u8 typeId, u8 index)` | Look up a starter species from the pool |
| `HelixGiveStartersForType` | `void (u8 typeId, u8 s1, u8 s2)` | Give two starters with neutral natures, random gender |
| `HelixComputeTypeFromQuiz` | `u8 (u8 q1, u8 q2, u8 q3)` | Weighted RNG type selection from quiz answers |
| `HelixStashNewGameState` | `void (u8 typeId, u8 s1, u8 s2)` | Stash choices in EWRAM (pre-wipe) |
| `HelixApplyNewGameState` | `void (void)` | Apply stashed state to save block (post-wipe) |

## Stray System

### Overview

Each day a stray Pokémon appears on the beach in a ball whose type indicates rarity. Interacting shows the mon's front sprite and a yes/no prompt. Strays are given at level 5 with a random neutral nature. On day 1, a scripted uncommon stray appears with gender-awareness to prevent breeding deadlocks.

### Architecture: 4-object approach

Four object events sit at the same map position (55, 24), each with a **static** ball graphic and its own hide flag. Only one is ever visible at a time.

| Object | Local ID | Graphics | Hide Flag |
|--------|----------|----------|-----------|
| Common | `LOCALID_HELIX_ISLAND_STRAY_COMMON` | `OBJ_EVENT_GFX_ITEM_BALL` | `FLAG_HELIX_STRAY_HIDE_COMMON` |
| Uncommon | `LOCALID_HELIX_ISLAND_STRAY_UNCOMMON` | `OBJ_EVENT_GFX_GREAT_BALL` | `FLAG_HELIX_STRAY_HIDE_UNCOMMON` |
| Rare | `LOCALID_HELIX_ISLAND_STRAY_RARE` | `OBJ_EVENT_GFX_ULTRA_BALL` | `FLAG_HELIX_STRAY_HIDE_RARE` |
| Legendary | `LOCALID_HELIX_ISLAND_STRAY_LEGENDARY` | `OBJ_EVENT_GFX_MASTER_BALL` | `FLAG_HELIX_STRAY_HIDE_LEGENDARY` |

All four share the same interaction script (`HelixIsland_EventScript_StrayBall`). The Great/Ultra/Master Ball OW graphics were registered from existing expansion follower-ball spritesheets.

### Flow

1. **New game** → `HelixIsland_OnTransition` detects `VAR_HELIX_STRAY_SPECIES == 0` → calls `HelixSpecial_GenerateDay1Stray` which spawns the uncommon ball with gender-awareness.
2. **Sleep** (`HelixIsland_EventScript_DoSleep`) → clears `FLAG_HELIX_STRAY_MSG_SHOWN` → calls `special HelixSpecial_GenerateStray` which rolls rarity, picks species, manages all 4 hide flags, and spawns the correct ball from C.
3. **Interaction** (`HelixIsland_EventScript_StrayBall`) — first interaction per day shows "washed up" message. Then `showmonpic` + yes/no prompt.
4. **Accept** → `special HelixSpecial_GiveStray` creates the mon in C with neutral nature + ability/gender overrides. `StrayHideAll` sets all 4 hide flags + removes all 4 objects.
5. **Decline** → ball stays until next sleep.

### Day-1 scripted stray

`HelixSpecial_GenerateDay1Stray` always picks **uncommon pool slot 0** for the player's type. It checks both starters' genders:
- If both are the same gender and the stray species supports the opposite → forces opposite gender via `VAR_HELIX_STRAY_GENDER`
- Otherwise → random gender

This prevents breeding deadlocks where both starters happen to be the same gender.

### Rarity roll

`RollStrayRarity()` in `src/helix.c` rolls 0–99:

| Roll | Rarity | Rate |
|------|--------|------|
| 0    | Legendary | 1% |
| 1–7  | Rare      | 7% |
| 8–29 | Uncommon  | 22% |
| 30–99 | Common   | 70% |

### Pool entries (`struct StrayEntry`)

Each pool entry is a `struct StrayEntry { u16 species; u8 abilityNum; }`. Arrays are terminated by `{ SPECIES_NONE, 0 }`. The `S(species)` macro uses the default ability; `S_AB(species, slot)` forces a specific ability slot.

Real species are populated for all 18 types from `docs/strays_type_data/*.csv`. Species data in CSV format: `Name,Rarity` where rarity 1–4 maps to common–legendary.

### Per-stray customization

- **Ability override**: Use `S_AB(SPECIES_CHARMANDER, 2)` to force hidden ability slot. If the desired ability isn't native, also modify the species' `abilities[]` in `src/data/pokemon/species_info/`. Example: Charmander's hidden ability was changed from Solar Power → Drought.
- **Form randomization**: Tatsugiri is stored as `SPECIES_TATSUGIRI` in the pool; `HelixSpecial_GiveStray` randomly picks Curly/Droopy/Stretchy at give time.
- **Gender override**: `VAR_HELIX_STRAY_GENDER` can force `MON_MALE` or `MON_FEMALE`. `GiveStray` includes a safety check — if the forced gender is impossible for the species, it falls back to random.

### Files touched by the stray system

- `include/helix.h` — `HELIX_RARITY_*` constants, `StrayEntry` struct, `STRAY_ABILITY_DEFAULT`
- `include/constants/flags.h` — `FLAG_HELIX_STRAY_HIDE_COMMON/UNCOMMON/RARE/LEGENDARY`, `FLAG_HELIX_STRAY_MSG_SHOWN`
- `include/constants/vars.h` — `VAR_HELIX_STRAY_SPECIES`, `VAR_HELIX_STRAY_ABILITY_OVERRIDE`, `VAR_HELIX_STRAY_GENDER`
- `include/constants/map_event_ids.h` — `LOCALID_HELIX_ISLAND_STRAY_COMMON/UNCOMMON/RARE/LEGENDARY`
- `include/constants/event_objects.h` — `OBJ_EVENT_GFX_GREAT_BALL`, `OBJ_EVENT_GFX_ULTRA_BALL`, `OBJ_EVENT_GFX_MASTER_BALL`
- `src/helix.c` — pools, rarity roll, `GenerateStray`, `GenerateDay1Stray`, `GiveStray`
- `src/data/object_events/object_event_graphics_info.h` — `GraphicsInfo` structs for Great/Ultra/Master Ball
- `src/data/object_events/object_event_graphics_info_pointers.h` — pointer table entries + externs
- `src/data/pokemon/species_info/gen_1_families.h` — Charmander hidden ability → Drought
- `data/specials.inc` — special registration
- `data/maps/HelixIsland/map.json` — 4 object events (one per rarity)
- `data/maps/HelixIsland/scripts.inc` — `EventScript_StrayBall`, `StrayHideAll`, sleep hooks, OnTransition day-1 hook

## Day-1 Tutorial & Guide NPC

### Overview

A scripted tutorial plays on the player's first arrival on HelixIsland. A type-specific guide NPC leads the player through picking up the day-1 stray, touring the island, and depositing mons in the PC. See `docs/design_document.md` for full design and flow.

### State Machine (`VAR_HELIX_INTRO_STATE`)

| Value | State | Description |
|-------|-------|-------------|
| 2 | Needs tutorial | Set by `HelixApplyNewGameState`. `ON_FRAME_TABLE` triggers cutscene. |
| 3 | Ball pickup | Set at cutscene start. Coord_event boundaries active. Player free to move in zone. |
| 4 | Done | Set at tutorial end. Normal gameplay. |

### Flag

`FLAG_HELIX_TUTORIAL_DONE` (0x498) — set when the tutorial completes. Gates the guide's day-2+ boat prompt.

### Guide NPC Object Event

- **Local ID**: `LOCALID_HELIX_ISLAND_GUIDE` (8) in `include/constants/map_event_ids.h`
- **Flag**: "0" (always visible)
- **Default sprite**: `OBJ_EVENT_GFX_NORMAN`, overridden at runtime by `HelixSpecial_SetGuideGraphics`
- **Day-1 starting position**: (42, 17) — walks to player during cutscene
- **Day-2+ position**: (28, 20) facing down — set via `setobjectxyperm` + `setobjectxy` in `ON_TRANSITION` and `DoSleep`
- **Sprite table**: `sGuideGraphics[18]` in `src/helix.c` maps each type to a unique NPC sprite (Norman, Bruno, Winona, Koga, Giovanni, Brock, Bug Catcher FRLG, Agatha, Steven, Flannery, Misty, Erika, Lt. Surge, Sabrina, Lorelei, Lance, Sidney, Wally)

### Map Script Architecture

```
HelixIsland_MapScripts::
    map_script MAP_SCRIPT_ON_LOAD, HelixIsland_OnLoad
    map_script MAP_SCRIPT_ON_TRANSITION, HelixIsland_OnTransition
    map_script MAP_SCRIPT_ON_FRAME_TABLE, HelixIsland_OnFrame
    .byte 0
```

- **ON_LOAD**: Boat tile changes. Day 1: sets finished boat area (23–25, 22–24) to CalmWater (0x170, impassable). Day 2+: clears under-construction boat to grass + places finished boat tiles. Fires before rendering — no `DrawWholeMapView` needed.
- **ON_TRANSITION**: Sets guide graphics, generates day-1 stray, repositions guide on day 2+ (`setobjectxyperm` + `setobjectxy`).
- **ON_FRAME_TABLE**: Triggers tutorial cutscene when `VAR_HELIX_INTRO_STATE == 2`.

### Boat Tile Changes

Handled in three places:
1. **ON_LOAD** — for initial map load and save/load
2. **DoSleep** — for same-map day transition (screen is black; uses `DrawWholeMapView`)
3. Map layout has the under-construction boat baked in; finished boat area is CalmWater in the layout

**Day 1 (ON_LOAD):**
```
setmetatile 23–25, 22–24 → 0x170 (CalmWater), impassable
```

**Day 2+ (ON_LOAD and DoSleep):**
```
@ Clear under-construction boat
setmetatile 47–49, 22–23 → METATILE_General_Grass (0x001), passable

@ Place finished boat
(24,22)→0x339, (25,22)→0x33A
(23,23)→0x340, (24,23)→0x341, (25,23)→0x342
(23,24)→0x348, (24,24)→0x349, (25,24)→0x34A
All impassable. (23,22) stays CalmWater.
```

### Boundary Coord Events (Ball Pickup Zone)

10 coord_events gated on `VAR_HELIX_INTRO_STATE == 3`:

| Position | Boundary | Player Response |
|----------|----------|------------------|
| (52, 24–28) | Left wall (5 tiles) | NPC `!` emote, "come back" dialogue, walk 1 step right |
| (54–58, 22) | Top wall (5 tiles) | NPC `!` emote, "come back" dialogue, walk 1 step down |

### Post-Ball Walk-to-Target

After the player picks up the stray, `TutorialPostBall` uses a script loop with `getplayerxy` to walk the player to (54, 24) one step at a time (X first, then Y). This handles any position within the boundary zone without teleporting.

### Coordinate Reference

| Feature | Position |
|---------|----------|
| Player spawn | (38, 23) |
| Stray ball | (55, 24) |
| PC shrub | (52, 10) |
| Bed signs | (33, 25) and (33, 26) |
| Stairs | x=41–42, y=18–21 |
| Guide day-2+ | (28, 20) |

### Comfort Boost

On day 1, `VAR_HELIX_COMFORT` is set to 240 (breed chance = 30 + 60 = 90%, the cap). Reset to 50 (`HELIX_DEFAULT_COMFORT`) when `VAR_HELIX_DAY_COUNT` reaches 2, handled in the sleep script.

### Per-Type Dialogue System

Guide NPC dialogue is fully personalized per type. The routing pattern in `scripts.inc`:

```
call HelixIsland_GuideMsg_[Beat]
  → switch VAR_HELIX_PLAYER_TYPE
    → goto HelixIsland_Handler_[Beat]_[Type]
      → msgbox HelixIsland_Text_[Beat]_[Type]
      → return
```

15 routing scripts handle all dialogue beats: Greeting, FollowMe, BallHint, GoodCatch, BoatIntro, ShopsIntro, PCIntro, Rest, ComeBack, RemindBall, Idle, RestReminder, ReadyPrompt (YESNO), BoatYes, BoatNo.

Norman (Normal type) retains the original full dialogue. All other 17 types have unique personality-driven text matching their character (e.g., Lt. Surge is military-themed, Agatha is cryptic/dry, Bug Catcher is enthusiastic).

### FRLG Sprite Inclusion

Many guide NPC sprites are FRLG-exclusive assets guarded by `#if IS_FRLG` in the base expansion. Changed to `#if 1` in 5 files to unconditionally include them:
- `src/data/object_events/object_event_graphics.h` — raw sprite INCBINs
- `src/data/object_events/object_event_pic_tables.h` — frame tables
- `src/data/object_events/object_event_graphics_info.h` — `ObjectEventGraphicsInfo` structs
- `src/data/object_events/object_event_graphics_info_pointers.h` — pointer array entries
- `src/event_object_movement.c` — palette table (`sObjectEventSpritePalettes`)

ROM cost: ~247KB. All changes are marked with `// Helix:` comments.

### Files Touched

- `include/constants/flags.h` — `FLAG_HELIX_TUTORIAL_DONE` (0x498)
- `include/constants/map_event_ids.h` — `LOCALID_HELIX_ISLAND_GUIDE` (8)
- `data/maps/HelixIsland/map.json` — Guide NPC object event, 10 coord_events (converted from bg_event placeholders)
- `data/maps/HelixIsland/scripts.inc` — Tutorial cutscene, boundary enforcement, guide interaction, boat tiles, walk-to-target, per-type dialogue routing (15 beat routers × 18 types)
- `src/helix.c` — `sGuideGraphics[18]` lookup table, `HelixSpecial_SetGuideGraphics` (template + live update)
- `include/helix.h` — `HelixSpecial_SetGuideGraphics` declaration
- `data/specials.inc` — `HelixSpecial_SetGuideGraphics` registration
- `src/data/object_events/object_event_graphics.h` — FRLG sprite INCBINs force-included
- `src/data/object_events/object_event_pic_tables.h` — FRLG pic tables force-included
- `src/data/object_events/object_event_graphics_info.h` — FRLG graphics info force-included
- `src/data/object_events/object_event_graphics_info_pointers.h` — FRLG pointer entries force-included
- `src/event_object_movement.c` — FRLG palettes force-included
