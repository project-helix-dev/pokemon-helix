# Pokémon Helix — Architecture Overview

This document maps all Helix-specific files, describes the build workflow, and documents key architectural patterns used in the project.

## Base Engine

Pokémon Helix is built on top of **pokeemerald-expansion**, a community-maintained decompilation of Pokémon Emerald with extended features including expanded species, abilities, items, and modern QoL improvements.

All Helix-specific changes are additive — new files and targeted modifications to existing engine files. The base expansion is not forked; changes are applied on top.

## Helix-Specific File Map

### Core Source

| File | Purpose |
|------|---------|
| `include/helix.h` | Type IDs, struct definitions (including `StrayEntry`), rarity constants, function declarations |
| `src/helix.c` | Type names, starter pools, quiz probability table, RNG type computation, stash/apply new-game state, breeding system (mother-driven pairing, egg creation with full inheritance), stray pools (per-type × per-rarity), rarity roll, generation/give specials |

### Modified Engine Files

| File | Modification |
|------|-------------|
| `src/main_menu.c` | Intro cutscene: Deoxys speech, 3-question quiz, starter selection with sprite preview, dialogue flow |
| `src/overworld.c` | `CB2_NewGame`: calls `HelixApplyNewGameState()` after save init; collapsed dead `if/else` |
| `src/new_game.c` | `WarpToTruck` changed to warp to `MAP_HELIX_ISLAND`; default text speed set to FAST |
| `src/pokemon_storage_system.c` | `IsRemovingLastPartyMon` always returns FALSE; `CanShiftMon` last-mon check removed — allows empty party for island population model |
| `src/strings.c` | `gText_ThisIsAPokemon` changed to reference Omanyte |
| `data/text/birch_speech.inc` | All intro dialogue rewritten for Deoxys/Helix theme, quiz questions, starter selection text |
| `data/specials.inc` | Registered all Helix specials (see Custom Data Reference) |
| `include/constants/vars.h` | 9 custom vars added (see Custom Data Reference) |
| `include/constants/flags.h` | 5 custom stray flags + `FLAG_HELIX_TUTORIAL_DONE` added |
| `include/constants/event_objects.h` | 3 new ball OW gfx IDs (`OBJ_EVENT_GFX_GREAT_BALL/ULTRA_BALL/MASTER_BALL`) |
| `include/constants/map_event_ids.h` | 4 stray local IDs per rarity + `LOCALID_HELIX_ISLAND_GUIDE` |
| `src/data/object_events/object_event_graphics.h` | FRLG sprite INCBINs force-included (`#if 1` instead of `#if IS_FRLG`) for guide NPC sprites; `GraphicsInfo` structs for Great/Ultra/Master Ball OW sprites |
| `src/data/object_events/object_event_graphics_info.h` | FRLG `ObjectEventGraphicsInfo` structs force-included; `GraphicsInfo` structs for Great/Ultra/Master Ball OW sprites |
| `src/data/object_events/object_event_graphics_info_pointers.h` | FRLG pointer entries force-included; pointer table entries + externs for ball graphics |
| `src/data/object_events/object_event_pic_tables.h` | FRLG pic table arrays force-included for guide NPC sprites |
| `src/event_object_movement.c` | FRLG NPC palettes force-included in `sObjectEventSpritePalettes` |
| `src/data/pokemon/species_info/gen_1_families.h` | Charmander hidden ability → Drought |
| `include/pokemon.h` | Added `MON_DATA_ABILITY_OVERRIDE` to data enum; repurposed 9 unused bits in `PokemonSubstruct0` for ability override storage (no struct size change) |
| `src/pokemon.c` | Added get/set cases for `MON_DATA_ABILITY_OVERRIDE`; `GetMonAbility()` checks override before species lookup; `PokemonToBattleMon` uses `GetMonAbility()`; `TryBoxMonFormChange` respects override |
| `src/pokemon_summary_screen.c` | `PrintMonAbilityName` and `PrintMonAbilityDescription` use `GetMonAbility()` to display overridden abilities correctly; `ExtractMonSkillStatsData` shows IVs instead of stats; `BufferLeftColumnStats` uses IV layout (no HP slash); egg memo text branches on `METLOC_HELIX_BRED` |
| `include/constants/region_map_sections.h` | Added `METLOC_HELIX_BRED` (0xFC) special met location for island-bred eggs |
| `src/strings.c` | Added `gText_HelixEggHatchTomorrow` and `gText_HelixBredEgg` for bred-egg summary text |

### Map Data

| Path | Purpose |
|------|---------|
| `data/maps/HelixIsland/` | Player's home island — sleep/food/day cycle, PC storage terminal, stray system (4 ball objects per rarity), bed signs, day-1 tutorial cutscene, guide NPC with type-based sprite, boat tile transitions, boundary enforcement |
| `data/maps/HelixIntroRoom/` | Legacy stub — quiz logic removed, kept for map registration |
| `data/layouts/HelixIsland/` | Tilemap and layout data for HelixIsland |
| `data/layouts/HelixIntroRoom/` | Tilemap and layout data for HelixIntroRoom (legacy) |

### Tools

| File | Purpose |
|------|---------|
| `tools/create_helix_maps.py` | Helper script for generating map boilerplate |

### Documentation

| File | Purpose |
|------|---------|
| `docs/design_document.md` | Game design direction (non-technical) |
| `docs/architecture.md` | This file — file map, build workflow, architectural patterns |
| `docs/intro_and_quiz_system.md` | Technical reference for the intro cutscene and quiz system |
| `docs/custom_data_reference.md` | Custom vars, flags, specials, maps, constants, and stray system technical reference |
| `docs/strays_type_data/` | CSV files defining stray species per type (18 files, `Name,Rarity` format) |

## Build Workflow

The project is developed on Windows with source files in OneDrive, built via WSL.

### Sync and Build

```bash
# Copy changed files from Windows to WSL
cp /mnt/c/Users/.../pokeemerald-expansion/src/helix.c ~/pokegenics/pokeemerald-expansion/src/helix.c
# (repeat for each changed file)

# Build
cd ~/pokegenics/pokeemerald-expansion && make -j4

# Copy ROM back to Windows
cp ~/pokegenics/pokeemerald-expansion/pokeemerald.gba /mnt/c/Users/.../pokegenics/pokeemerald.gba
```

Individual `cp` commands are preferred over rsync for speed — only changed files are synced.

## Key Architectural Patterns

### Stash/Apply Pattern (Save Wipe Survival)

The intro cutscene runs before `CB2_NewGame`, which calls `NewGameInitData()` — this wipes the entire save block (vars, flags, party, etc.). Any game state set during the cutscene would be lost.

**Solution**: Helix uses static EWRAM variables to stash choices made during the cutscene. After the save wipe, `HelixApplyNewGameState()` is called in `CB2_NewGame` to write the stashed values into the fresh save block.

```
Intro cutscene (main_menu.c)
  → HelixStashNewGameState(typeId, starter1, starter2)  [EWRAM stash]
  → CB2_NewGame (overworld.c)
    → NewGameInitData()  [wipes save]
    → HelixApplyNewGameState()  [restores from stash]
```

This pattern should be used for any future data that needs to survive the new-game save wipe.

### Demo Tuning Values

The current build is balanced as a fast-paced demo. Several constants and inheritance rates in `src/helix.c` and `include/helix.h` are temporarily boosted and clearly flagged with `// DEMO:` comments. These should be reverted to design values before a balanced release:

| Value | Demo | Design | Location |
|-------|------|--------|----------|
| `HELIX_STARTING_FOOD` | 200 | 20 | `include/helix.h` |
| `HELIX_BREED_CHANCE_BASE` | 90 | 30 | `include/helix.h` |
| Ability inheritance roll | 25/15/30/30 | 50/30/10/10 | `CreateBreedingEgg` |
| Egg-move category roll | 10/30/30/30 | 75/10/10/5 | `InheritEggMoves` |
| Same-species extra egg-move | 40% | 10% | `InheritEggMoves` |

**Side effect**: because base breed chance (90) already hits the 90% cap, `VAR_HELIX_COMFORT` currently has no observable effect on breeding.

### Task Data Slot Management

The Birch speech task uses all 16 `data[]` slots. Helix quiz state is stored in:

| Slot | Define | Purpose |
|------|--------|---------|
| `data[3]` | `tHelixQuizQ3` | Quiz answer 3 (reused — set to 0xFF at init, never read before quiz) |
| `data[12]` | `tHelixQuizQ1` | Quiz answer 1 |
| `data[13]` | `tHelixQuizQ2` | Quiz answer 2 |
| `data[14]` | `tHelixTypeId` | Computed type ID |
| `data[15]` | `tHelixStarter1` | First starter choice index |

When adding new task data, check all existing defines carefully — there are no free high slots remaining.

### Script Specials vs C-Callable Functions

Functions called from map scripts must be registered in `data/specials.inc`. Functions called only from C code (e.g., from `main_menu.c` tasks) do not need special registration.

Currently registered Helix specials:
- `HelixSpecial_BufferTypeName` — buffers player type name to `STR_VAR_1` (food sign removed; available for future use)
- `HelixSpecial_BufferStarterNames` — available for future script use
- `HelixSpecial_CountPopulation` — counts party + PC mons into `VAR_HELIX_ISLAND_POP`
- `HelixSpecial_HatchEggs` — hatches eggs in PC boxes during sleep (clears egg flag and nickname; moveset is baked in at creation)
- `HelixSpecial_BreedBoxes` — mother-driven breeding: females then genderless roll for eggs, pairing with males/genderless in the same box
- `HelixSpecial_GenerateStray` — daily stray: rolls rarity, picks species, manages flags, spawns ball
- `HelixSpecial_GenerateDay1Stray` — scripted day-1 uncommon stray with gender awareness
- `HelixSpecial_GiveStray` — creates stray mon with neutral nature + overrides, gives to player
- `HelixSpecial_SetGuideGraphics` — sets guide NPC sprite based on player type; updates both the object event template (for spawn) and live object event (for in-place refresh)

### FRLG Sprite Inclusion

Many guide NPC sprites (Lt. Surge, Brock, Misty, Erika, Sabrina, Koga, Giovanni, Bruno, Agatha, Lorelei, Lance, Bug Catcher FRLG) are FRLG-exclusive assets guarded by `#if IS_FRLG` in the base expansion. Since Helix builds as Emerald, these would be compiled out — causing crashes when the guide sprite is loaded.

**Fix**: Changed `#if IS_FRLG` → `#if 1` in 5 files to unconditionally include all FRLG NPC sprites and palettes:
- `src/data/object_events/object_event_graphics.h` — raw sprite INCBINs
- `src/data/object_events/object_event_pic_tables.h` — frame tables
- `src/data/object_events/object_event_graphics_info.h` — `ObjectEventGraphicsInfo` structs
- `src/data/object_events/object_event_graphics_info_pointers.h` — pointer array entries
- `src/event_object_movement.c` — palette table (`sObjectEventSpritePalettes`)

ROM cost: ~247KB. All changes are marked with `// Helix:` comments for easy grep.

### Per-Type Dialogue Routing

Guide NPC dialogue is fully personalized per type. The system uses a routing pattern:

```
call HelixIsland_GuideMsg_[Beat]
  → switch VAR_HELIX_PLAYER_TYPE
    → goto HelixIsland_Handler_[Beat]_[Type]
      → msgbox HelixIsland_Text_[Beat]_[Type]
      → return
```

15 routing scripts handle all tutorial and post-tutorial dialogue beats:
- Greeting, FollowMe, BallHint, GoodCatch, BoatIntro, ShopsIntro, PCIntro, Rest
- ComeBack, RemindBall, Idle, RestReminder, ReadyPrompt (YESNO), BoatYes, BoatNo

Norman (Normal type) retains the original full dialogue. All other 17 types have unique personality-driven text. Labels follow the pattern `HelixIsland_Text_[Beat]_[Type]` (e.g., `HelixIsland_Text_Greeting_Fire`).

This architecture scales cleanly — adding new dialogue beats requires only one new routing script and 18 handler+text label pairs.

### EWRAM_DATA Initialization

GBA EWRAM variables must be zero-initialized. If code expects a sentinel value (e.g., `SPRITE_NONE = 0xFF`), it must be set at runtime before first use. Always document this dependency.

### Ability Override System

The engine stores abilities as a slot number (0/1/2) in `MON_DATA_ABILITY_NUM`, with the actual ability derived from `gSpeciesInfo[species].abilities[slot]`. To support cross-species ability transfer via breeding, a 9-bit ability override was added using unused bits in `PokemonSubstruct0`:

- `abilityOverrideLo:6` (replaces `unused_02`) + `abilityOverrideHi:3` (replaces `unused_04`) = 9 bits (0–511, covers all 310+ abilities)
- When `MON_DATA_ABILITY_OVERRIDE` is non-zero (`!= ABILITY_NONE`), `GetMonAbility()` returns the override directly instead of looking up from species data
- Zero = no override (default for all mons)
- **No struct size change** — uses previously unused bits, so save layout is unaffected

This override persists through evolution. It is currently set only by the breeding system's 10% cross-species ability transfer, but can be used by any future system (type-pool second abilities, disorder abilities, etc.).

### Known Issue: Guide NPC Walking Animation

12 of 18 guide NPCs lack walking animation frames — they slide along the ground instead of animating when walking. This is because the base game only gave these characters 3-frame standing spritesheets (south, north, west). They were designed as stationary gym/E4 NPCs and never needed walking frames.

**Affected (3-frame standing only — slides):**
Flannery, Winona, Sidney, Misty, Lance, Bruno, Koga, Agatha, Brock, Erika, Sabrina, Lt. Surge

**Unaffected (full 9-frame walking):**
Norman, Steven, Wally, Lorelei, Giovanni, Bug Catcher FRLG

**Root cause**: The `sAnimTable_Standard` walk animations reference frame indices 3–8 for walking poses. The 3-frame pic tables (e.g., `sPicTable_Flannery`) map indices 3–8 back to the same standing frames 0–2, producing no visible animation.

**How to fix once custom walking sprites are created:**

1. **Create the sprite sheet** (16×32 per frame, 9 frames in a row = 144×32 PNG):
   - Frame 0: face south (standing)
   - Frame 1: face north (standing)
   - Frame 2: face west (standing)
   - Frame 3: walk south (left foot)
   - Frame 4: walk south (right foot)
   - Frame 5: walk north (left foot)
   - Frame 6: walk north (right foot)
   - Frame 7: walk west (left foot)
   - Frame 8: walk west (right foot)
   - East-facing is auto-generated by horizontally flipping the west frames.

2. **Replace the PNG and 4bpp** in `graphics/object_events/pics/people/` (or the appropriate subdirectory, e.g., `gym_leaders/`).

3. **Update the pic table** in `src/data/object_events/object_event_pic_tables.h`. Change the explicit `overworld_frame()` entries to use `overworld_ascending_frames()`:
   ```c
   // Before (3-frame, no walk animation):
   static const struct SpriteFrameImage sPicTable_Flannery[] = {
       overworld_frame(gObjectEventPic_Flannery, 2, 4, 0),
       overworld_frame(gObjectEventPic_Flannery, 2, 4, 1),
       overworld_frame(gObjectEventPic_Flannery, 2, 4, 2),
       overworld_frame(gObjectEventPic_Flannery, 2, 4, 0),  // duplicates
       overworld_frame(gObjectEventPic_Flannery, 2, 4, 0),
       overworld_frame(gObjectEventPic_Flannery, 2, 4, 1),
       overworld_frame(gObjectEventPic_Flannery, 2, 4, 1),
       overworld_frame(gObjectEventPic_Flannery, 2, 4, 2),
       overworld_frame(gObjectEventPic_Flannery, 2, 4, 2),
   };
   
   // After (9-frame, full walk animation):
   static const struct SpriteFrameImage sPicTable_Flannery[] = {
       overworld_ascending_frames(gObjectEventPic_Flannery, 2, 4),
   };
   ```

4. **No changes needed** to the graphics info struct or animation table — they already use `sAnimTable_Standard` and `sOamTables_16x32`, which are correct for 9-frame 16×32 NPC sprites.

5. **Repeat** for each affected NPC. The INCBIN in `object_event_graphics.h` does not specify a size, so it auto-adjusts to the new file.

### IV System

Helix uses a custom IV system designed for generational progression:

- **Starters and strays** roll each IV in [1, `HELIX_IV_MAX`] (currently 5). Defined in `include/helix.h`.
- **Bred eggs** inherit IVs per-stat 50/50 from parents — no random floor/ceiling applied, so offspring naturally improve over generations as higher-IV parents are selected.
- **Summary screen** displays IVs in place of calculated stats on the skills page (`ExtractMonSkillStatsData` in `pokemon_summary_screen.c` reads `MON_DATA_*_IV` instead of `MON_DATA_*`). HP displays as a single number (no current/max slash), using `sStatsLeftIVEVColumnLayout`.

Key files:
- `include/helix.h` — `HELIX_IV_MAX` constant
- `src/helix.c` — `HelixSetRandomIVs()` helper (sets IVs and recalculates stats)
- `src/pokemon_summary_screen.c` — `ExtractMonSkillStatsData()` modified, `BufferLeftColumnStats()` uses IV layout

### Breeding System Architecture

Breeding is **mother-driven**: each female and genderless mon rolls independently per day.

**Pairing flow** (per box):
1. Phase 1: Each female rolls → on success, picks a random male or genderless partner
2. Phase 2: Each genderless mon rolls → on success, picks a random male or genderless partner (excluding itself)

Each mother produces **at most one egg per day**. Males never initiate rolls — they are only selected as partners.

**Egg creation** (`CreateBreedingEgg`) applies inheritance in order:
1. Species — 50/50 from each parent
2. Nature — 45% parent 1, 45% parent 2, 10% fully random (any of 25 natures)
3. IVs — per-stat 50/50 from each parent (6 independent rolls)
4. Shiny — 1/1000 base, 1/100 one shiny parent, 1/10 both shiny parents
5. Ability — (**demo** 25/15/30/30; *design* 50/30/10/10) same-species parent's ability, natural (personality-based), hidden ability (slot 2), cross-species transfer (uses ability override if foreign to species)
6. Moveset — level-up moves via `GiveMonInitialMoveset`, then egg move inheritance
7. Egg moves — (**demo** 10/30/30/30; *design* 75/10/10/5) none, random from species pool, same-species parent transfer, cross-species parent transfer

The complete moveset (level-up + egg moves) is **baked into the egg at creation time**. `HelixSpecial_HatchEggs` does not call `GiveBoxMonInitialMoveset` — it only clears the egg flag, sets the nickname to the species name, and sets the language.

Bred eggs are tagged with met location `METLOC_HELIX_BRED` (0xFC) and the Japanese egg nickname (so they display as a normal "Egg"). The summary screen branches on this met location to show Helix-specific egg text (`gText_HelixEggHatchTomorrow` before hatching, `gText_HelixBredEgg` after).
