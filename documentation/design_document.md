**Pokémon Helix**

_Current Ground-Truth Design Document_

A concise high-level design reference for AI-assisted development. This version includes the current core systems, island progression, inheritance model, and Nexus-based run structure.


## DEVLEOPER's NOTE: I'm not sure whether the below demo build note is still accurate in this version, please confirm.
> **Demo build note**: The values in this document describe the intended design. The current build ships with several rates temporarily boosted for a fast demo (much higher starting food, ~90% breeding, far more egg-move/ability inheritance). See *Demo Build Tuning* below and `docs/architecture.md` for the exact overrides.

# Project Overview

This project is a Pokémon ROM hack built around a type-locked generational roguelike structure.

The player builds a persistent island ecosystem, improves Pokémon bloodlines through inheritance, sends parties on branching biome runs, and grows long-term infrastructure between expeditions.

The core identity is: one primary type per save file, breeding-driven progression, run-based expeditions with attrition, persistent home-base growth, and late-game expansion into a second type via a second island.

# Core Gameplay Loop

The player begins with a type assigned through a three-question personality quiz during the intro cutscene. The quiz uses weighted random probabilities, so the same answers can produce different types across playthroughs — some types are rarer than others.

Two progenitor Pokémon from that type's starter pool are chosen by the player and form the foundation of the island population.

Days are advanced by sleeping, which consumes food and resolves island-side systems such as offspring generation and stray arrivals.

The player forms a party from island inhabitants, enters the Nexus, and starts a branching expedition.

Runs provide experience, money, food-related resources, held items, and route progression.

The player can return home after a zone or continue deeper toward lesser legendaries and, eventually, the three longest endgame paths.

# Intro and Guide Character

The intro cutscene is narrated by Deoxys, who serves as the game's guide figure in place of a traditional Pokémon professor. Deoxys introduces the themes of DNA, bloodlines, and the island challenge before administering the personality quiz.

An Omanyte is shown during the intro as a demonstration of how life can be restored — a thematic tie to the game's fossil and genetics motifs.

# Island Guide NPC and Day-1 Tutorial

## Guide NPC

Each type has a unique guide NPC who lives on the island. The NPC is a gym leader or notable character thematically linked to the type (e.g., Norman for Normal). The only functional difference between the 18 NPCs is their sprite and dialogue — all game logic is shared. The NPC's sprite is set dynamically via `setobjectgraphics` based on `VAR_HELIX_PLAYER_TYPE`.

After day 1, the guide stands near the finished boat at (28, 20) and provides context, reminders, and future gameplay hooks when spoken to.

## Day-1 Tutorial Sequence

The tutorial fires on the player's first map load via `MAP_SCRIPT_ON_FRAME_TABLE` (gated by `VAR_HELIX_INTRO_STATE == 2`). It is a fully scripted cutscene — the player cannot move freely except during the ball pickup segment.

### Flow

1. **Player spawns** at (38, 23) facing down. Movement is locked.
2. **NPC walks down the stairs** from (42, 17) to (42, 23), turns left toward the player, and shows a `!` exclamation emote.
3. **NPC speaks** one line, then walks to (40, 23). Player turns right to face them. More dialogue (welcome, surprise, comment on the player's two mons).
4. **Scripted walk to the stray ball**: NPC and player walk simultaneously. NPC: (40,23)→(45,23)→(45,25)→(53,25)→(53,23), faces right. Player: (38,23)→(45,23)→(45,25)→(54,25)→(54,24), faces right.
5. **Ball pickup segment**: Player is released facing the ball at (54, 24). The stray ball is at (55, 24). A perimeter of `coord_event` tiles prevents the player from walking away — stepping on a boundary tile triggers NPC `!` emote, a "come back" line, and the player is walked back toward the ball. If the player declines the stray, the NPC nags and the ball interaction can be re-attempted.
6. **After accepting the stray**, the player walks back to (54, 24) if they wandered. Movement locks again and boundaries are deactivated. Both walk to the boat area: NPC to (47, 24) faces up, player to (49, 24) faces up. NPC comments on the under-construction boat ("should be ready tomorrow").
7. **Walk to shops**: NPC to (42, 7), player to (42, 9). NPC comments that shopkeepers were supposed to arrive but never did.
8. **Walk to PC**: NPC to (51, 7) faces down, player to (50, 9). NPC explains PC breeding/inheritance briefly and mentions the player can deposit all mons.
9. **NPC tells player to rest** at the beach chair they passed earlier. Movement unlocks. NPC stays at PC position and gives reminder dialogue if spoken to again.
10. **When the player sleeps**: comfort is temporarily maxed (set to 240 → 90% breed chance) for the first day. The under-construction boat tiles are replaced with walkable grass. The finished boat tiles appear at the dock. The NPC moves to their permanent day-2+ position.

### Boundary Enforcement (Ball Pickup)

The stray ball sits near the ocean at (55, 24). The ocean provides a natural wall on two sides. `coord_event` triggers form the remaining boundary:
- **Left wall** (bg_ids 2–6): x=52, y=24 through y=28 — player turns right and walks one step back
- **Top wall** (bg_ids 7–11): x=54 through x=58, y=22 — player turns down and walks one step back

These coord_events are gated behind `VAR_HELIX_INTRO_STATE == 3` so they are only active during the ball pickup phase.

### Boat Tile Changes (Day 2 Transition)

**Under-construction boat** (removed): 3×2 region at x=47–49, y=22–23. All tiles become metatile 0x001 ("Grass"), collision changed from impassable to elevation 3.

**Finished boat** (added): 3×3 region at x=23–25, y=22–24. Tiles start as 0x170 ("CalmWater") with impassable collision. On day 2, specific metatiles are placed:
- (23,22) stays CalmWater, (24,22)→0x339, (25,22)→0x33A
- (23,23)→0x340, (24,23)→0x341, (25,23)→0x342
- (23,24)→0x348, (24,24)→0x349, (25,24)→0x34A

All from the Slateport secondary tileset.

## Implementation Status

**Implemented.** All tutorial systems are functional:
- Guide NPC object event (local ID 8, flag "0" = always visible) with sprite set via `HelixSpecial_SetGuideGraphics` based on `VAR_HELIX_PLAYER_TYPE` (unique sprite per type: Norman, Bruno, Winona, Koga, Giovanni, Brock, Bug Catcher, Agatha, Steven, Flannery, Misty, Erika, Lt. Surge, Sabrina, Lorelei, Lance, Sidney, Wally)
- `FLAG_HELIX_TUTORIAL_DONE` (0x498) marks tutorial completion
- `VAR_HELIX_INTRO_STATE` state machine: 2 = needs tutorial, 3 = ball pickup phase, 4 = done
- `MAP_SCRIPT_ON_FRAME_TABLE` triggers cutscene when state == 2
- `MAP_SCRIPT_ON_LOAD` handles boat tile changes (fires before rendering)
- `MAP_SCRIPT_ON_TRANSITION` handles guide graphics, stray generation, and day-2+ guide repositioning
- 10 `coord_event` boundary triggers (converted from bg_event placeholders), gated on state 3
- `setmetatile` in `ON_LOAD` for day-1 CalmWater and day-2+ boat placement
- `setmetatile` + `DrawWholeMapView` in DoSleep for same-map tile updates
- `setobjectxyperm` + `setobjectxy` for guide repositioning (template + active sprite)
- Day-1 comfort boost to 240 (90% breed chance), reset to 50 on day 2
- Walk-to-target loop in post-ball segment (handles any player position in boundary zone)
- Per-type dialogue fully implemented: 15 routing scripts × 18 types, each NPC has unique personality-driven text (Norman is fatherly, Lt. Surge is military, Agatha is cryptic, Bug Catcher is enthusiastic, etc.); textboxes close before walks

# Starting Types and Progenitors

At the start of a save file, the player is assigned one Pokémon type. That type defines the early ecosystem and starter pool.

Two starters are chosen from the assigned type's pool and act as the initial progenitors of the player's bloodline.

| **Type** | **Starter Pool**          | **Type** | **Starter Pool**             |
| -------- | ------------------------- | -------- | ---------------------------- |
| Normal   | Rattata, Aipom, Skitty    | Fire     | Vulpix, Houndour, Numel      |
| Water    | Krabby, Wooper, Wailmer   | Electric | Voltorb, Chinchou, Electrike |
| Grass    | Paras, Sunkern, Cacnea    | Ice      | Cubchoo, Smoochum, Snorunt   |
| Fighting | Mankey, Tyrogue, Makuhita | Poison   | Ekans, Spinarak, Gulpin      |
| Ground   | Diglett, Gligar, Baltoy   | Flying   | Doduo, Hoothoot, Taillow     |
| Psychic  | Drowzee, Natu, Spoink     | Bug      | Venonat, Ledyba, Surskit     |
| Rock     | Dwebble, Bonsly, Nosepass | Ghost    | Golett, Misdreavus, Shuppet  |
| Dragon   | Noibat, Applin, Duraludon | Dark     | Murkrow, Sneasel, Poochyena  |
| Steel    | Bronzor, Cufant, Varoom   | Fairy    | Snubbull, Cottonee, Swirlix  |

# Island Structure

The island is the player's persistent home base. It houses the population, handles breeding and stray arrivals, supports party selection, and anchors long-term infrastructure growth.

It is intended to feel like a living habitat rather than a storage box.

Implementation note: Island population is stored in standard PC boxes. Initially only one box is available; additional boxes are unlocked through progression, giving the player control over which Pokémon can breed together. The party is handled normally as in the base game.

# Breeding and Inheritance

Breeding is the primary long-term progression system. When a day passes, island Pokémon can produce offspring.

Breeding is mother-driven: each female and genderless Pokémon independently rolls to produce an egg, selecting a male or genderless partner from the same PC box. Each mother produces at most one egg per day. Males are never initiators — they can only be selected as partners.

An offspring is one of the parents' species (50/50) and inherits a mix of traits from the parents.

The currently implemented inherited categories are: species, nature, IVs, shiny status, primary ability, and egg moves.

Nature has a 45% chance to come from each parent and a 10% chance to be completely random, including non-neutral natures. Non-neutral natures function as mutation traits that can appear and be passed down.

IVs are inherited per-stat with a 50/50 chance from each parent. They begin relatively low and matter more than they do in the mainline games, making bloodline improvement a major source of power growth.

Shiny status is slightly inheritable: 1/1000 with no shiny parents, 1/100 with one shiny parent, 1/10 with two shiny parents. Shiny Pokémon will eventually grant a small all-stat bonus.

Primary abilities are usually inherited from the parent that matches the offspring species (50%), with a 30% chance for natural generation, 10% chance for the hidden ability, and 10% chance for cross-parent inheritance. Cross-parent ability transfer can give a Pokémon an ability foreign to its species, which persists through evolution. *(Demo build uses 25/15/30/30.)*

Egg moves have a 75% chance of no inheritance, 10% chance to gain a random egg move from the species pool, 10% chance to inherit egg moves from a same-species parent, and 5% chance to inherit a move from the other parent's species (which cannot be passed down further). *(Demo build uses 10/30/30/30.)*

Each type also has a pool of inheritable second abilities that any Pokémon of that type can gain. There is also a rare universal pool of negative 'disorder' second abilities and an extremely rare universal pool of especially strong positive second abilities. These are planned but not yet implemented.

# Demo Build Tuning

The shipping demo temporarily boosts several rates so progression is visible within a short play session. These are flagged with `// DEMO:` comments in code and should be reverted before a balanced release:

| System | Demo | Design |
|--------|------|--------|
| Starting food | 200 | 20 |
| Base breeding chance | 90% (always at cap) | 30% + comfort |
| Ability inheritance | 25/15/30/30 | 50/30/10/10 |
| Egg-move inheritance | 10/30/30/30 | 75/10/10/5 |

A side effect of the 90% breeding base is that island comfort currently has no visible impact.

# Day Progression and Food

A day passes when the player sleeps / ends the day.

Each day consumes one food for every present island inhabitant. The demo starts the player with a large food stockpile (200) so food pressure does not gate the early experience.

Food is the island's core upkeep resource. It can be found during runs or purchased from the shop using money earned on runs.

Food pressure makes population size, breeding pace, stray acceptance, and day advancement strategically meaningful.

# Strays

Each day a single stray Pokémon appears on the beach inside a Poké Ball. The player may accept it (adding it to the party or PC) or leave it until the next day resets the system.

Strays serve three major roles: kickstarting options, injecting variety, and preventing softlocks in a game built around nuzlocke-style attrition.

## Rarity Tiers

The stray rolls one of four rarity tiers, shown on the map by the type of ball sprite used:

| **Rarity**    | **Drop Rate** | **Ball**    | **Pool Size / Type** | **Character**                                                   |
| ------------- | ------------- | ----------- | -------------------- | --------------------------------------------------------------- |
| Common        | 70%           | Poké Ball   | 7                    | Mostly single-typed, not especially strong or popular.           |
| Uncommon      | 22%           | Great Ball  | 5                    | More dual-typed; bread-and-butter team members, slightly stronger. |
| Rare          | 7%            | Ultra Ball  | 3                    | Most powerful species; may carry atypical abilities.              |
| Legendary     | 1%            | Master Ball | 1                    | One legendary per type. Extremely rare.                           |

Island infrastructure (planned) will affect the potential quality of strays — either by biasing the rarity roll or by expanding the pool.

## Interaction Flow

1. Player talks to the ball.
2. On the first interaction of that day, the message "A Pokémon has washed up on shore!" is shown. Subsequent interactions on the same day skip this.
3. The Pokémon's front sprite is displayed and the player is prompted "Add \<name\> to your party?"
4. **Yes** → the Pokémon is given at level 5 (party first, then PC if full). The ball disappears for the day.
5. **No** → the ball stays; the player can return to it until the day ends.
6. Sleeping rolls a fresh rarity and species and re-places the ball for the new day.

## Day-1 Scripted Stray

On the player's first arrival on the island, a scripted uncommon (Great Ball) stray always appears. It is the first species in the type's uncommon pool, chosen to introduce variety beyond the two starters. If both starters happen to be the same gender, the stray is forced to be the opposite gender to prevent breeding deadlocks.

## Implementation Status

- Core loop fully implemented: daily appearance, interaction, accept/decline, daily refresh via sleep.
- 4 rarity tiers with distinct ball sprites (Poké/Great/Ultra/Master Ball).
- Real species populated for all 18 types (7 common / 5 uncommon / 3 rare / 1 legendary each). Source data in `docs/strays_type_data/*.csv`.
- Per-stray ability overrides supported (e.g., stray Charmander spawns with Drought).
- Tatsugiri randomly spawns as any of its three color forms.
- All strays arrive at level 5 with a random neutral nature.
- Day-1 scripted uncommon stray with gender-awareness.

# Expeditions and the Nexus

The player forms a party on the island and sends it out on an expedition.

Initially, parties come from the first island only. Later, the player can send a party from the second island instead. In the very late game, mixed-island parties become available.

Every run begins at the Nexus, where the player chooses one of three initial paths: Beach, Forest, or Mountains.

The Beach path branches into zones such as ocean, islands, and jungle. Its furthest destination is Birth Island, home of Deoxys.

The Forest path branches into its own set of zones and eventually leads to the lab where Genesect resides.

The Mountains path branches into mountain-based zones and ultimately leads to Spear Pillar, where Arceus resides.

Zones do not branch the same way every run. After finishing a zone, the player may be offered different route options depending on the run and current progression unlocks.

# Run Progression and Endpoints

Most runs end in one of four ways: party wipe, voluntary return home after a zone, completion of a route by defeating a lesser legendary, or-on the deepest unlocked paths-defeat of one of the three major endgame bosses.

Navigating all the way to Birth Island, the lab, or Spear Pillar is a late-game goal that requires many runs and route-expanding unlocks.

Runs are the main source of experience, money, food-related rewards, held items, and route progression.

# Befriending and Population Growth

During standard encounters and legendary battles, the player can attempt to befriend the opponent if it matches the player's allowed type context.

Success is based on the base stat total of the currently active player Pokémon.

If successful, the opposing Pokémon returns to the island. Befriending is a core non-breeding way to grow the population.

# Attrition, Money, and Meta Progression

Runs operate under nuzlocke-style attrition assumptions, so loss matters across the campaign.

Money earned on runs feeds a shop that sells at least food and infrastructure upgrades.

Infrastructure and island expansion are the main forms of persistent meta progression. They support a larger ecosystem, improve island quality, and feed into systems such as stray quality and long-term stability.

Gym / challenger events at the island are part of the home-base progression structure, though their detailed rules are not yet finalized.

# Late-Game Second Island

A major late-game unlock is access to a second island tied to a second chosen type.

This expands team variety, reduces long-term FOMO from the initial type assignment, and preserves the importance of the first island by delaying cross-type access until later in the game.

Pokémon can move between islands if they are dual-typed with the other island's type. For example, if the player starts with Fire and later unlocks Ground, Numel can move between the two islands.

Dual-types therefore function as bridge species between ecosystems.

# Design Pillars

The project is built around six core pillars: type identity, generational progression, a persistent home base, branching roguelike expeditions, meaningful attrition with recovery tools, and late-game expansion without fully dissolving the game's type-based structure.
