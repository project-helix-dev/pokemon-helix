# Pokemon Helix - Nexus / Caves Run / Level-less IV Progression Implementation Brief

_Final implementation-facing draft for Claude Fable. This file describes the first complete pass of the Nexus/run/level system. The goal is a stable, configurable Caves MVP that plugs into the existing Helix island, day, breeding, stray, and guide systems without overbuilding final aesthetics._

## 1. Purpose

Implement the last missing base gameplay layer before detailed content and aesthetic design:

- A simple Nexus hub entered from the island boat.
- A three-door route-selection room, with only the Caves/Mountains line active for now.
- A party-size-gated Caves route MVP.
- A reusable run state machine.
- Fixed level-5 combat with EXP and level progression disabled.
- IV-based stat progression through post-battle reward choices.
- Trainer, wild, shop, and endpoint room types.
- Nuzlocke-style attrition where fainted Pokemon are removed from the party.
- Run-only money/food that is banked only on successful run completion.
- Retirement marking so Pokemon used on one completed run cannot be sent on another.

This should be treated as a functional skeleton, not a final content pass. Favor stable state, configurable data tables, readable scripts, and simple maps.

## 2. Existing foundation to preserve

Do not rewrite existing island systems unless necessary for integration.

The current project already has:

- Deoxys intro / quiz flow that assigns `VAR_HELIX_PLAYER_TYPE` and gives two starters.
- Helix Island day progression, food consumption, breeding, egg hatching, and daily strays.
- Empty-party PC support so the player can deposit all Pokemon into boxes.
- Day-1 tutorial state via `VAR_HELIX_INTRO_STATE` and `FLAG_HELIX_TUTORIAL_DONE`.
- A finished boat on day 2+ and a guide NPC near the boat.
- Type-specific guide NPC graphics/dialogue using the existing guide sprite lookup.
- Custom vars currently occupying `0x40F7` through `0x40FF`.
- IV display and custom IV-focused progression foundation.
- Ability override system and breeding inheritance, which should remain unaffected by the run MVP.

Build the run system additively on top of these.

## 3. High-level player flow

### Island to Nexus

1. On Helix Island, the day-2+ boat / guide prompt asks whether the player wants to go to the Nexus.
2. If accepted, warp the player to a new map: `MAP_HELIX_NEXUS`.
3. The Nexus is a simple square room for now.
4. The player may return to the island by talking to their guide NPC inside the Nexus.

Important day rule:

- After the player completes or loses a run, the guide should not allow another run until the next island day.
- Days still advance only by sleeping on the island.
- Completing or losing a run does not itself advance the day.

### Nexus room

The Nexus room should contain:

- A door in the middle of the top wall: active Caves route.
- A door in the middle of the left wall: placeholder locked route.
- A door in the middle of the right wall: placeholder locked route.
- A PC in the top-right corner so the player can adjust party size before departing.
- The player's type-specific guide NPC standing in the middle of the room.

The final version will eventually use a cutscene here. For this MVP, the player simply walks up and talks to the guide.

### Nexus guide behavior

On first talk after entering Nexus:

- Explain that there are three routes.
- Explain that only the Caves route is open for now.
- Clarify that Caves is the first stage of the route line that eventually leads toward Mountains / Spear Pillar.
- If the player has more than 3 usable party Pokemon, say they can only bring 3 and should use the PC.
- If the player has 1 or 2 usable party Pokemon, say they may proceed but it will probably be harder without a full group of 3.
- If the player has 0 usable party Pokemon, tell them they need at least one Pokemon to enter.
- Mark the Nexus as introduced so the top door can be used.

After the guide has been spoken to once:

- The top/Caves door is usable if run-entry constraints pass.
- The left and right doors remain closed in this MVP and should show placeholder text if interacted with.
- In the future, choosing a route will lock the other two. For this MVP, only the top door actually starts a run.

### Caves run

1. Player attempts to walk through the top Nexus door.
2. If a run has already been completed or lost today, bounce back and show a message telling the player to rest before setting out again.
3. If usable party count is 0, bounce back and show a no-party message.
4. If usable party count is greater than 3, bounce back and show a deposit-some message.
5. If the party contains retired Pokemon, bounce back and explain that Pokemon who already returned from a run cannot be sent again.
6. If party count is 1 to 3, initialize a Caves run and warp to the first cave room.
7. First cave room is always a fixed Hiker trainer tutorial room.
8. Then the player proceeds through a fixed-length cave route whose room types are randomly selected from:
   - Wild Encounter Room
   - Trainer Room
   - Shop Room
9. After the final random room, the player enters a Regirock endpoint room.
10. Defeating or catching Regirock ends the run, shows a run summary, banks run rewards, marks surviving run participants as retired, gives the Regirock endpoint reward, and warps the player back to the Nexus in front of the Caves door.
11. The player can talk to the Nexus guide to return to Helix Island.

## 4. Non-goals for this pass

Do not spend usage on final polish before core functionality works.

Defer:

- Final Nexus cutscene.
- Final Nexus visuals.
- Final route names and aesthetics beyond Caves.
- Beach/Forest content.
- True Mountains route content beyond the Caves first-stage placeholder.
- Complex procedural map generation.
- Full befriending formula.
- Second island rules.
- Endgame boss routes.
- Fancy run summary UI.
- Full battle HUD redesign unless the core no-EXP behavior is already stable.

## 5. Recommended new vars

Current Helix vars occupy `0x40F7` through `0x40FF`. Add run vars after confirming no collisions.

Suggested names:

```c
#define VAR_HELIX_NEXUS_STATE             0x4100 // 0=not briefed, 1=guide explained routes
#define VAR_HELIX_RUN_ACTIVE              0x4101 // 0=no, 1=yes
#define VAR_HELIX_RUN_BIOME               0x4102 // 0=Caves MVP; later route lines
#define VAR_HELIX_RUN_ROOM_INDEX          0x4103 // 0=fixed intro trainer, 1..N=random rooms, N+1=boss
#define VAR_HELIX_RUN_ROOM_TYPE           0x4104 // wild/trainer/shop/boss/etc.
#define VAR_HELIX_RUN_ROOM_COMPLETE       0x4105 // 0=no, 1=yes
#define VAR_HELIX_RUN_NEXT_EXIT_DIR       0x4106 // door direction for current room
#define VAR_HELIX_RUN_ENTRANCE_DIR        0x4107 // direction player came from
#define VAR_HELIX_RUN_WILD_TARGET         0x4108 // wild room target encounters, 2..4
#define VAR_HELIX_RUN_WILD_DONE           0x4109 // completed wild encounters in current room
#define VAR_HELIX_RUN_WILD_FLED           0x410A // 0=no flee in this room, 1=fled at least once
#define VAR_HELIX_RUN_TRAINER_VARIANT     0x410B // selected trainer table index
#define VAR_HELIX_RUN_MONEY_EARNED        0x410C // unbanked run money
#define VAR_HELIX_RUN_FOOD_EARNED         0x410D // unbanked run food
#define VAR_HELIX_RUN_LAST_KO_PARTY_INDEX 0x410E // 0..5, or 0xFF if none
#define VAR_HELIX_REWARD_STAT_1           0x410F // random IV reward option
#define VAR_HELIX_REWARD_STAT_2           0x4110 // random IV reward option
#define VAR_HELIX_REWARD_STAT_3           0x4111 // random IV reward option
#define VAR_HELIX_RUN_ENDPOINT_REWARD     0x4112 // 0=none, 1=Moon Stone, 2=Rocky Helmet
#define VAR_HELIX_LAST_RUN_DAY            0x4113 // day count when most recent run ended or was lost
```

Use temporary script vars where possible, but explicit vars make the MVP easier to debug.

### Banked versus unbanked rewards

Existing island food is `VAR_HELIX_FOOD`. Run food should not be added to that var until successful completion.

Recommended split:

- `VAR_HELIX_RUN_FOOD_EARNED`: unbanked food gained during the current run.
- `VAR_HELIX_RUN_MONEY_EARNED`: unbanked money gained during the current run.
- On success: add food to `VAR_HELIX_FOOD`; add money to the player's actual money or a later island-bank variable.
- On blackout: discard both run vars.

For money, if engine prize money is automatically added after trainer battles, either:

1. Prefer disabling/neutralizing direct payout and manually tracking run money, or
2. Immediately subtract the amount after battle and store it in `VAR_HELIX_RUN_MONEY_EARNED`.

The desired design is that money and food are not banked unless the run is completed.

## 6. Recommended flags / persistent markers

Only add flags for persistent unlocks or broad state. Prefer vars for volatile run state.

Possible flags:

```c
#define FLAG_HELIX_NEXUS_UNLOCKED
#define FLAG_HELIX_CAVES_UNLOCKED
```

MVP can avoid extra flags by gating:

- Nexus entry using `FLAG_HELIX_TUTORIAL_DONE` or `VAR_HELIX_DAY_COUNT >= 2`.
- Door use using `VAR_HELIX_NEXUS_STATE`.
- Once-per-day run restriction using `VAR_HELIX_LAST_RUN_DAY == VAR_HELIX_DAY_COUNT`.

### Retired Pokemon marker

Pokemon used on one completed run cannot be sent on another run.

Use a ribbon to mark them as retired if practical. This has two advantages:

- It persists naturally on the Pokemon.
- It can be checked at run entry without needing a separate global registry.

Suggested behavior:

- On successful run completion, mark all surviving non-egg Pokemon in the active run party with the chosen retirement ribbon.
- On run entry, reject any non-egg party Pokemon that already has the retirement ribbon.
- The rejection message should be simple: these Pokemon have already returned from an expedition and cannot be sent out again.
- Pokemon that faint during a run are deleted, so they do not need to be retired.

Implementation note:

- Pick an unused or thematically acceptable ribbon constant from the existing engine.
- If using a ribbon is unexpectedly invasive, use another persistent per-Pokemon marker only if it is clearly safe for save compatibility.

## 7. Constants and configuration

Add all tunables as named constants or data tables. Do not bury magic numbers in scripts.

Suggested constants in `include/helix.h` or a new run-specific section:

```c
#define HELIX_RUN_PARTY_MAX                    3
#define HELIX_RUN_FIXED_LEVEL                  5
#define HELIX_RUN_IV_REWARD_AMOUNT             1
#define HELIX_RUN_IV_REWARD_CAP                31
#define HELIX_RUN_TRAINER_REWARD_MONEY         100
#define HELIX_RUN_WILD_FOOD_MIN                2
#define HELIX_RUN_WILD_FOOD_MAX                5
#define HELIX_RUN_WILD_MIN_ENCOUNTERS          2
#define HELIX_RUN_WILD_MAX_ENCOUNTERS          4
#define HELIX_RUN_CAVE_RANDOM_ROOM_COUNT       9
#define HELIX_RUN_ENEMY_IV_VALUE               5
#define HELIX_RUN_REGIROCK_ITEM_CHANCE_PERCENT 50 // Moon Stone vs Rocky Helmet is 50/50
```

Important:

- The existing `HELIX_IV_MAX` is currently the generation cap for starters/strays, not necessarily the absolute progression cap.
- Use a separate reward cap so early generated mons can start low but eventually grow.
- Keep `HELIX_RUN_FIXED_LEVEL` at 5 for now.

Recommended enums:

```c
enum HelixRunBiome
{
    HELIX_RUN_BIOME_CAVES,
    HELIX_RUN_BIOME_COUNT,
};

enum HelixRunRoomType
{
    HELIX_RUN_ROOM_NONE,
    HELIX_RUN_ROOM_FIXED_TRAINER,
    HELIX_RUN_ROOM_WILD,
    HELIX_RUN_ROOM_TRAINER,
    HELIX_RUN_ROOM_SHOP,
    HELIX_RUN_ROOM_BOSS,
};

enum HelixRunDirection
{
    HELIX_RUN_DIR_UP,
    HELIX_RUN_DIR_RIGHT,
    HELIX_RUN_DIR_DOWN,
    HELIX_RUN_DIR_LEFT,
};

enum HelixStatId
{
    HELIX_STAT_HP,
    HELIX_STAT_ATK,
    HELIX_STAT_DEF,
    HELIX_STAT_SPATK,
    HELIX_STAT_SPDEF,
    HELIX_STAT_SPEED,
    HELIX_STAT_COUNT,
};
```

## 8. Data tables

Keep the run configurable through small static tables.

### Cave room type weights

Default to equal chance for each procedural room type.

```c
static const struct HelixWeightedRoomType sCaveRoomTypeWeights[] =
{
    { HELIX_RUN_ROOM_WILD,    33 },
    { HELIX_RUN_ROOM_TRAINER, 33 },
    { HELIX_RUN_ROOM_SHOP,    34 },
    { HELIX_RUN_ROOM_NONE,     0 },
};
```

### Cave route layout

Use a fixed nine-room route after the first fixed trainer room.

The intended provisional sequence is:

```text
up, up, left, up, up, right, right, up, up
```

Clarification:

- The user originally said `straight` and `up`; both mean north / top of the screen.
- `left` and `right` are absolute screen directions for route structure.
- If directional room layouts are expensive, encode this route now but use simple bottom-entry/top-exit maps visually until the state machine works.
- The state table should still exist so proper directional door layouts can be added later.

Suggested table:

```c
static const u8 sCaveRouteExitDirs[HELIX_RUN_CAVE_RANDOM_ROOM_COUNT] =
{
    HELIX_RUN_DIR_UP,
    HELIX_RUN_DIR_UP,
    HELIX_RUN_DIR_LEFT,
    HELIX_RUN_DIR_UP,
    HELIX_RUN_DIR_UP,
    HELIX_RUN_DIR_RIGHT,
    HELIX_RUN_DIR_RIGHT,
    HELIX_RUN_DIR_UP,
    HELIX_RUN_DIR_UP,
};
```

### Trainer variants

Random procedural trainer room picks:

- Sprite type
- Trainer name
- Contact saying
- Defeat saying
- One level-5 Pokemon from the cave pool

Sprites:

```text
Hiker, Camper, Nerd, Old Man
```

Names:

- Use a random male name from a configurable name table.
- Reuse intro fallback names if convenient.
- Keep the table easy to expand.

Pokemon pool:

```text
Diglett, Geodude, Sandshrew, Numel, Zubat
```

All trainer Pokemon:

- Level 5
- Straight 5 IVs across the board
- No special held item unless table later specifies one

Example saying tables:

```c
// Hiker
"The cave keeps what it wants!"
"Step carefully. The stones are listening."
"A good climb starts underground!"

// Camper
"I thought this trail looked safe!"
"Nothing like a battle under the rocks!"
"I packed light, but I packed Pokemon!"

// Nerd / Scientist-style
"A closed ecosystem inside a cave? Fascinating!"
"Your specimens are developing nicely."
"Let us test a practical hypothesis!"

// Old Man
"These caves remember more than we do."
"I've walked this tunnel since before you were born."
"Slow steps still reach the summit."
```

Keep sayings separated by sprite archetype so future content can swap tone by trainer class.

### Wild encounter pool

Use the same pool as trainer enemies for MVP:

```text
Diglett, Geodude, Sandshrew, Numel, Zubat
```

All wild Pokemon:

- Level 5
- Straight 5 IVs across the board
- Catchable only if compatible with the player's type context
- If incompatible, the player may battle, KO, or run, but catching should be blocked or fail with a clear message

For this MVP, type compatibility can be:

- Catchable if the Pokemon has the player's primary type or secondary type.
- Otherwise not catchable.

### Shop stock

Shop room has one Poke Mart clerk in the center.

Stock:

```text
Moon Stone - 100
Poke Ball  - 100
Potion     - 100
```

The player can leave the room at any time.

## 9. Maps to add

### `MAP_HELIX_NEXUS`

Simple square room.

Required objects:

- Guide NPC in center, sprite set based on `VAR_HELIX_PLAYER_TYPE`.
- PC object in top-right.
- Top door for Caves route.
- Left and right placeholder doors.

Door behavior:

- Top door uses coord event or warp script to validate party/run state before starting Caves.
- If invalid, bounce player back and show message.
- Left/right doors show placeholder text.

### `MAP_HELIX_CAVE_TRAINER_INTRO`

Small-medium square cave room.

Required layout:

- Bottom door where player enters.
- Top door locked/blocked until trainer is defeated.
- Hiker trainer in middle.

Behavior:

- Trainer alerts immediately when player walks in.
- Trainer has only a level-5 Diglett.
- Trainer gives 100 unbanked run money.
- After victory, prompt IV reward for the Pokemon that dealt the final blow.
- After reward, mark room complete and enable top door.

### Procedural room maps

For MVP, do not generate real maps dynamically. Use a small number of reusable maps whose contents are configured by vars on transition.

Recommended maps:

- `MAP_HELIX_CAVE_ROOM_WILD`
- `MAP_HELIX_CAVE_ROOM_TRAINER`
- `MAP_HELIX_CAVE_ROOM_SHOP`
- `MAP_HELIX_CAVE_ROOM_BOSS`

Each should be a simple square cave room with door positions matching the current route direction where possible.

Simplest acceptable implementation:

- Bottom entrance.
- Top exit.
- Ignore left/right visual door placement for the first pass, while still storing intended route direction in vars.

Better implementation:

- Reusable cave room with multiple possible exits.
- Hide/enable only the exit direction needed by `VAR_HELIX_RUN_NEXT_EXIT_DIR`.

### `MAP_HELIX_CAVE_ROOM_BOSS`

Endpoint room.

Required layout:

- One entrance, no further route doors.
- Static Regirock object standing in the room.

Behavior:

- Regirock is level 5 with 5 IVs across the board.
- Player can knock it out.
- Player can attempt to catch it only on a Rock-type playthrough.
- Defeating or catching Regirock completes the run.
- Regirock does not grant an IV reward for now.
- Victory gives a 50/50 endpoint reward:
  - Moon Stone
  - Rocky Helmet
- Show run summary and then auto-warp the player back to Nexus in front of the Caves door.

## 10. Party, eggs, and entry validation

### Usable party count

Run entry should count only usable, non-egg, non-retired Pokemon.

Rules:

- 1 to 3 usable non-egg Pokemon: valid.
- 0 usable non-egg Pokemon: invalid.
- More than 3 usable non-egg Pokemon: invalid.
- Eggs are ignored for this count.
- A party of 1 to 3 usable Pokemon plus any number of eggs is valid.
- Retired Pokemon are invalid for run entry.

Examples:

- 3 Pokemon + 0 eggs: valid.
- 1 Pokemon + 3 eggs: valid.
- 0 Pokemon + 6 eggs: invalid.
- 4 Pokemon + 0 eggs: invalid.
- 2 Pokemon + 1 retired Pokemon: invalid until the retired Pokemon is deposited.

### Entry messages

Suggested messages:

- More than 3 usable Pokemon: `You can only bring three Pokemon into the Caves. Deposit some in the PC first.`
- 0 usable Pokemon: `You need at least one Pokemon that can battle before setting out.`
- Retired Pokemon present: `Pokemon that already returned from an expedition can't be sent out again.`
- Run already attempted today: `You should rest before setting out again.`
- Left/right locked doors: `This route is closed for now.`

## 11. Run initialization

When entering Caves:

1. Set `VAR_HELIX_RUN_ACTIVE = 1`.
2. Set `VAR_HELIX_RUN_BIOME = HELIX_RUN_BIOME_CAVES`.
3. Set `VAR_HELIX_RUN_ROOM_INDEX = 0`.
4. Clear run money and run food vars.
5. Clear last-KO tracking.
6. Clear wild-room counters.
7. Optionally normalize all active run Pokemon to level 5.
8. Assign random nicknames to non-nicknamed, non-egg party Pokemon.
9. Warp to the fixed intro trainer cave room.

### Random nickname assignment

When the player enters the first cave room, any non-nicknamed party Pokemon are assigned random nicknames based on gender.

Rules:

- Male Pokemon get a random male fallback intro name.
- Female Pokemon get a random female fallback intro name.
- Genderless Pokemon can receive either.
- Existing nicknames are preserved.
- Eggs are ignored.

Use the same or similar name pools as the intro name fallback system.

## 12. Fixed intro trainer room

The first Caves room is always a trainer room with a Hiker.

Trainer:

- Sprite: Hiker
- Pokemon: Diglett
- Level: 5
- IVs: 5 in every stat
- Reward: 100 unbanked run money

Flow:

1. Player enters room.
2. Hiker alerts and approaches / starts battle immediately.
3. Battle begins.
4. On victory, 100 money is added to `VAR_HELIX_RUN_MONEY_EARNED`, not permanently banked.
5. After prize message, show the IV reward prompt for the Pokemon that dealt the final blow.
6. After reward, room is marked complete and the exit door is enabled.

Suggested placeholder trainer lines:

```text
Before battle: "You won't reach the summit by standing still!"
After battle:  "Hah! The cave likes you better."
```

## 13. IV reward system

The IV reward system appears after eligible trainer rooms and eligible wild rooms.

### Trigger conditions

Trainer room:

- Trigger after victory if a valid `VAR_HELIX_RUN_LAST_KO_PARTY_INDEX` exists.

Wild room:

- Trigger after the room's final encounter only if:
  - All required encounters were completed by KO or capture.
  - The player did not flee from any encounter in the room.
  - A valid last-KO Pokemon exists.

Important wild-room rule:

- Any fleeing disqualifies the room from the IV reward.
- Catching does not disqualify the player.
- KO/catch mixes are valid as long as no encounter was fled.
- If the player catches every encounter and no Pokemon scored a KO, there is no valid last-KO recipient, so skip the IV reward or show a short no-recipient message.

Boss room:

- Regirock does not grant IVs for now.

### Reward choice

When reward triggers:

1. Generate 3 randomly chosen stats.
2. Ideally, avoid duplicates so the menu offers three distinct stats.
3. Store them in `VAR_HELIX_REWARD_STAT_1/2/3`.
4. Show a message along the lines of:

```text
{MON} grew from the battle!
What reward do you want?
```

5. Open a 3-option menu like the intro quiz menus.
6. Menu choices are stat names:

```text
HP
Attack
Defense
Sp. Atk
Sp. Def
Speed
```

7. On selection, increase that IV by `HELIX_RUN_IV_REWARD_AMOUNT` up to `HELIX_RUN_IV_REWARD_CAP`.
8. Recalculate the Pokemon's stats.
9. Show:

```text
{MON}'s {STAT} rose!
```

### Tracking final blow

Need a reliable way to record which party Pokemon dealt the final blow.

Recommended:

- Hook battle-end / faint logic to store the party index of the battler responsible for the opponent's last HP drop.
- Store in `VAR_HELIX_RUN_LAST_KO_PARTY_INDEX`.
- Reset to `0xFF` at the start of each room and before each wild encounter as needed.

If perfect final-blow tracking is expensive, acceptable first-pass fallback:

- Use the currently active party Pokemon at battle win.
- Keep the code isolated so it can be replaced with true final-blow tracking later.

## 14. Random room types

After the fixed intro trainer room, the route has nine random rooms.

Each random room independently rolls one of:

- Wild Encounter Room
- Trainer Room
- Shop Room

Default weights:

- Wild: 33%
- Trainer: 33%
- Shop: 34%

These weights must be configurable in code.

### Room progression

After a room is complete:

1. Player can walk through the exit door.
2. Increment `VAR_HELIX_RUN_ROOM_INDEX`.
3. If there are remaining random rooms, roll the next room type and warp to the matching reusable map.
4. If all random rooms are complete, warp to the boss room.

Shop rooms should probably count as complete immediately on entry, since the player can leave at any time.

## 15. Wild Encounter Room

Layout:

- Small-medium square cave room.
- 4x4 grass patch in the middle.
- Entrance door behind player.
- Exit door locked/blocked until the room is complete.

Behavior:

1. On room entry, roll `VAR_HELIX_RUN_WILD_TARGET` from 2 to 4.
2. Set `VAR_HELIX_RUN_WILD_DONE = 0`.
3. Set `VAR_HELIX_RUN_WILD_FLED = 0`.
4. Grass encounters use the cave wild pool.
5. Each encounter can end by KO, capture, or run.
6. KO gives random 2 to 5 unbanked food after battle, shown similarly to prize money.
7. Capture sends the caught Pokemon directly to the PC, never to the active run party.
8. Running increments completion count but sets `VAR_HELIX_RUN_WILD_FLED = 1`.
9. When `WILD_DONE >= WILD_TARGET`, remove/disable further grass encounters and mark the room complete.
10. If no fleeing occurred, grant an IV reward to the last valid KO Pokemon.
11. Enable exit door.

### Food reward

KO food reward:

- Random amount from 2 to 5.
- Add to `VAR_HELIX_RUN_FOOD_EARNED`, not `VAR_HELIX_FOOD`.
- Show text such as:

```text
You gathered {X} food.
```

### Catching rules

Captured run Pokemon:

- Go directly to PC.
- Do not join active run party.
- Do not count toward the current party size.
- Should not be marked retired immediately unless they are later used in a completed run.

Catch eligibility:

- Player can catch Pokemon that match the player's type context.
- For MVP, allow catch if wild species has the player's type as either primary or secondary type.
- If invalid, block the catch or fail with a clear message.

Possible message:

```text
This Pokemon doesn't resonate with your island's type.
```

## 16. Random Trainer Room

Layout:

- Same basic setup as the fixed intro trainer room.
- Trainer stands in the middle.
- Entrance door behind player.
- Exit door locked/blocked until trainer defeated.

Behavior:

1. On room entry, select random trainer variant.
2. Trainer alerts when player enters.
3. Trainer uses one Pokemon selected randomly from the cave pool.
4. Trainer Pokemon is level 5 with straight 5 IVs.
5. Victory gives 100 unbanked run money.
6. Victory grants IV reward to final-blow Pokemon.
7. Room becomes complete and exit is enabled.

Possible trainer classes:

- Hiker
- Camper
- Nerd
- Old Man

Possible Pokemon:

- Diglett
- Geodude
- Sandshrew
- Numel
- Zubat

## 17. Shop Room

Layout:

- Simple cave room.
- Poke Mart shopkeeper in the center.
- Exit door available immediately.

Behavior:

- Player can shop or leave.
- No battle.
- Room counts as complete immediately.

Shop inventory:

| Item | Price |
|---|---:|
| Moon Stone | 100 |
| Poke Ball | 100 |
| Potion | 100 |

Implementation note:

- If run money is intended to be usable before banking, this needs a separate decision. For this MVP, the safest interpretation is that the player uses existing banked money for shop purchases, while run money remains unbanked until completion.
- If desired, later implement shops that can use unbanked run money as a dungeon currency.

## 18. Regirock endpoint room

Layout:

- No further exits.
- Static Regirock object.
- Entrance behind player.

Regirock:

- Level 5.
- 5 IVs across the board.
- Catchable only on Rock-type playthroughs.
- Can be knocked out by any type.

On victory/capture:

1. Do not grant an IV reward.
2. Roll endpoint item reward:
   - 50% Moon Stone
   - 50% Rocky Helmet
3. Give the item immediately or include it in the run summary payout.
4. Show run summary:
   - Money earned
   - Food gathered
   - Endpoint item received
   - Pokemon retired, if concise to display
5. Bank run rewards:
   - Add run food to island food.
   - Add run money to permanent money.
   - Give endpoint item.
6. Mark surviving non-egg run party Pokemon as retired with a ribbon.
7. Set `VAR_HELIX_LAST_RUN_DAY = VAR_HELIX_DAY_COUNT`.
8. Clear run-active state.
9. Escape-rope-style warp back to `MAP_HELIX_NEXUS`, in front of the Caves door.

Suggested summary text:

```text
Expedition complete!
Money earned: {MONEY}
Food gathered: {FOOD}
You found a {ITEM}.
```

## 19. Faint deletion and blackout

### Faint deletion

If any player Pokemon is knocked out during a run:

- It should be shown as fainted on the normal switch-in menu.
- After the next Pokemon is switched in, the fainted Pokemon should immediately disappear from the party.
- Deleted Pokemon do not return to the island.
- Deleted Pokemon do not need to be marked retired.

Implementation approach:

- During active run battles, track party mons that faint.
- After switch-in resolution, remove fainted mons from party.
- Compact party afterward.
- Be careful not to delete eggs or unrelated party slots accidentally.

### Blackout

If the player blacks out during a run:

- Show custom blackout dialogue reflecting that they are returned to the island.
- Discard unbanked run money and food.
- Set `VAR_HELIX_LAST_RUN_DAY = VAR_HELIX_DAY_COUNT` so the player cannot set out again until the next day.
- Clear run-active state.
- Delete/clear the remaining party, leaving the player with an empty party.
- Warp the player to Helix Island next to the bed.

Suggested blackout text:

```text
Your expedition was lost in the dark...
You wake beside the island bed.
```

Important:

- Blackout should not bank run money or run food.
- Blackout should not advance the day.
- The player can rebuild from the PC/island population, strays, and future breeding.

## 20. Level-less / EXP-less progression

This is core to the design.

Rules:

- All Pokemon remain level 5 for now.
- No Pokemon should ever gain EXP.
- EXP messages during/after battle should be disabled.
- Level-up messages should never occur.
- Enemy Pokemon are level 5.
- Run progression comes from IV increases, not levels.

Implementation options:

### Required MVP

- Prevent EXP gain entirely in battle.
- Suppress EXP-related messages.
- Ensure player Pokemon do not level above 5.
- Ensure generated run enemies are level 5.
- Ensure generated/caught run Pokemon are level 5.

### EXP bar handling

Desired display:

- Set EXP of all Pokemon to one point before level 6 and keep it there so the EXP bar appears full.
- Never allow the value to change through battle.

### Optional HUD cleanup

If time permits after core features:

- Remove level text from battle display.
- Remove EXP bar / EXP text from battle display.
- Remove level/EXP emphasis from summary screen if not already hidden.

Do not spend major implementation budget on battle HUD surgery before the run loop works.

## 21. Special functions to add

Register script-callable functions in `data/specials.inc`.

Recommended specials:

```c
HelixSpecial_SetNexusGuideGraphics
HelixSpecial_CountUsableRunParty
HelixSpecial_CheckRunEntry
HelixSpecial_StartCavesRun
HelixSpecial_AssignRunNicknames
HelixSpecial_InitCaveRoom
HelixSpecial_RollNextCaveRoom
HelixSpecial_SetupRunTrainer
HelixSpecial_SetupRunWildEncounter
HelixSpecial_OnRunBattleWon
HelixSpecial_AddRunMoney
HelixSpecial_AddRunFood
HelixSpecial_PrepareIVRewardChoices
HelixSpecial_ApplyIVRewardChoice
HelixSpecial_CheckWildRoomComplete
HelixSpecial_CompleteRunRoom
HelixSpecial_CompleteCavesRun
HelixSpecial_HandleRunBlackout
HelixSpecial_RetireRunParty
HelixSpecial_HasRetiredPartyMon
```

Some of these can be merged if desired, but keep responsibilities clear.

Non-special C helpers should handle reusable logic:

```c
static u8 CountUsableRunPartyMons(void);
static bool8 IsRunEligibleMon(struct Pokemon *mon);
static bool8 IsMonRetiredFromRuns(struct Pokemon *mon);
static void MarkMonRetiredFromRuns(struct Pokemon *mon);
static void SetMonFixedRunLevel(struct Pokemon *mon);
static void SetEnemyFixedIVs(struct Pokemon *mon);
static void AddRunFood(u16 amount);
static void AddRunMoney(u16 amount);
static u8 RollWeightedRoomType(const struct HelixWeightedRoomType *table);
static u8 RollRandomStatChoice(void);
static void ApplyIVIncreaseToPartyMon(u8 partyIndex, u8 statId, u8 amount);
static bool8 SpeciesMatchesPlayerType(u16 species);
```

## 22. Battle integration notes

The hardest pieces are likely battle-system hooks. Keep them isolated and well-commented.

Required battle integrations:

1. Disable EXP gain.
2. Suppress EXP messages.
3. Keep all Pokemon at level 5.
4. Track final-blow party mon for IV rewards.
5. Detect player Pokemon fainting during active runs.
6. Delete fainted Pokemon after switch-in resolution.
7. Customize blackout handling during active runs.
8. Prevent invalid captures during runs.
9. Send valid captures directly to PC.
10. Track wild battle outcome: KO, capture, or flee.

Recommended design:

- Add a small set of Helix battle callbacks/helpers rather than scattering run logic across battle code.
- Gate everything behind `VAR_HELIX_RUN_ACTIVE == 1`.
- Avoid affecting normal island/non-run behavior unless intentionally part of the global no-EXP design.

Because level-less progression is global, disabling EXP may apply everywhere, but run-specific deletion/capture/blackout logic should only apply during active runs.

## 23. Run completion and once-per-day lockout

After either success or blackout:

```c
VAR_HELIX_RUN_ACTIVE = 0;
VAR_HELIX_LAST_RUN_DAY = VAR_HELIX_DAY_COUNT;
```

Door entry check:

```c
if (VAR_HELIX_LAST_RUN_DAY == VAR_HELIX_DAY_COUNT)
{
    // reject; player must sleep before another run
}
```

Sleeping on the island increments `VAR_HELIX_DAY_COUNT`, which naturally clears the restriction without needing an additional flag.

## 24. Script flow sketch

### Nexus top door

```text
Player steps on / interacts with top door
  call HelixSpecial_CheckRunEntry
  if result == OK:
      call HelixSpecial_StartCavesRun
      warp MAP_HELIX_CAVE_TRAINER_INTRO
  else:
      applymovement player, step_down
      msgbox appropriate rejection text
```

### Fixed intro trainer room

```text
On transition:
  call HelixSpecial_AssignRunNicknames
  setup Hiker/Diglett if not defeated

On trainer sight:
  trainerbattle
  call HelixSpecial_AddRunMoney(100)
  call HelixSpecial_PrepareIVRewardChoices
  show reward menu
  call HelixSpecial_ApplyIVRewardChoice
  set room complete
  enable exit
```

### Random room transition

```text
Exit door:
  if room not complete:
      msgbox blocked text
      stop
  increment room index
  if room index > random room count:
      warp boss room
  else:
      call HelixSpecial_RollNextCaveRoom
      warp matching reusable room map
```

### Wild room

```text
On transition:
  call HelixSpecial_InitCaveRoom
  roll target 2..4

After each wild battle:
  if KO:
      add random 2..5 run food
  if capture:
      send capture to PC
  if flee:
      set WILD_FLED
  increment WILD_DONE
  if WILD_DONE >= WILD_TARGET:
      if not fled and has final-KO mon:
          offer IV reward
      mark room complete
      enable exit
```

### Boss room

```text
Interact with Regirock:
  start static encounter

After KO/capture:
  roll endpoint item
  show summary
  bank run money and food
  give item
  retire surviving run party
  set last run day
  clear run state
  warp Nexus in front of Caves door
```

## 25. Implementation priority

Use this order so limited Claude usage goes to core functionality first.

### Priority 1 - Required run skeleton

- Add Nexus map.
- Add Caves intro trainer map.
- Add basic Caves random room maps.
- Add boss room.
- Add vars/constants/enums.
- Add Nexus entry from island.
- Add Nexus guide and PC.
- Add top door entry validation.
- Add run start / run active state.
- Add fixed intro Hiker/Diglett fight.
- Add room completion and door progression.
- Add random room selection.
- Add Regirock endpoint and return-to-Nexus.

### Priority 2 - Core mechanics

- Disable EXP gain and EXP messages.
- Keep all Pokemon level 5.
- Add unbanked run money/food.
- Add run summary and banking on success.
- Add blackout reward loss.
- Add once-per-day run lockout.
- Add catch-to-PC behavior.
- Add catch type restriction.
- Add faint deletion.
- Add retired ribbon marking and entry blocking.

### Priority 3 - Reward progression

- Track final-blow mon.
- Add IV reward choice generation.
- Add intro trainer IV reward.
- Add random trainer IV reward.
- Add wild room IV reward with flee disqualification.
- Add stat recalculation after IV increase.

### Priority 4 - Configurability / polish

- Move room weights, trainer tables, encounter pools, shop stock, reward values into clear data tables.
- Add random trainer sayings by trainer class.
- Add nickname assignment.
- Improve door direction support.
- Add cleaner messages.
- Optional battle HUD cleanup.

## 26. Acceptance criteria

The implementation is acceptable when:

1. Player can enter Nexus from island after the tutorial/day-2 state.
2. Nexus has guide NPC, PC, top/left/right doors.
3. Talking to the guide explains route rules.
4. Left/right doors are closed placeholders.
5. Top door starts Caves only after guide intro and valid party check.
6. Eggs are ignored for run party count.
7. More than 3 usable Pokemon blocks entry.
8. 0 usable Pokemon blocks entry.
9. Retired Pokemon block entry.
10. Completing or losing a run blocks additional runs until the next island day.
11. First Caves room always runs Hiker/Diglett.
12. Non-nicknamed run party Pokemon get random names on entry.
13. Trainer victory adds 100 unbanked run money.
14. Trainer victory offers IV reward to final-blow Pokemon.
15. Random rooms roll wild/trainer/shop with configurable equal-ish odds.
16. Wild room runs 2 to 4 encounters.
17. Wild KO adds random 2 to 5 unbanked food.
18. Wild capture sends Pokemon directly to PC.
19. Wild fleeing disqualifies the room's IV reward.
20. Wild room grants IV reward only if completed without fleeing and a valid KO recipient exists.
21. Shop room sells Moon Stone, Poke Ball, Potion for 100 each.
22. Route ends in Regirock room.
23. Regirock is level 5 with 5 IVs.
24. Regirock grants no IV reward.
25. Regirock victory/capture gives 50/50 Moon Stone or Rocky Helmet.
26. Run summary displays money, food, and endpoint item.
27. Successful run banks food/money/item.
28. Successful run retires surviving run party Pokemon.
29. Blackout loses unbanked food/money and returns player beside island bed with empty party.
30. Fainted Pokemon disappear from party during runs after switch-in resolution.
31. No Pokemon gain EXP.
32. No Pokemon level above 5.
33. EXP messages do not appear.
34. Core values are named constants or table entries, not buried magic numbers.

## 27. Known implementation risks

### Battle hooks

The hardest work is probably battle integration:

- Final-blow tracking.
- Faint deletion after switch-in.
- Catch-to-PC during active runs.
- EXP suppression.
- Blackout override.

Keep these isolated and gated behind helper functions.

### Run reward money

Pokemon's normal trainer-prize flow likely adds money directly. Because run money should be unbanked until success, this may require custom trainer payout handling or subtract-and-track logic.

### Retired ribbon choice

Need to identify a safe ribbon constant that will not conflict with other important systems. If no good ribbon exists, use a safe per-Pokemon marker alternative only after confirming save implications.

### Door direction versus map simplicity

The route wants directional structure, but the MVP can tolerate visually simple northbound rooms if needed. Do not let directional polish block the run loop.

### EXP bar / level HUD

Disabling EXP is required. Removing the level/EXP HUD is optional polish for this pass.

## 28. Open design questions after this final draft

These are not blockers for the MVP, but should be revisited later:

1. Should shops use banked island money, unbanked run money, or both?
2. Which ribbon should officially represent retired expedition Pokemon?
3. Should caught endpoint legendaries also be retired immediately, or treated like new island population not yet used on a run?
4. Should all Pokemon remain level 5 forever, or should the project later move to fixed level 50 for more precise battle math?
5. Should Regirock completion unlock anything persistent, or is it just a repeatable first-route endpoint for now?
6. Should retiring apply only after successful completion, or should Pokemon that voluntarily return from partial future routes also retire?
7. Should the no-EXP rule apply globally from new game, including island/trainer events outside runs?

For the current implementation, assume:

- Shops use banked island money.
- Retired ribbon blocks future run entry.
- Caught run Pokemon go directly to PC and are not retired unless later used in a run.
- Level remains fixed at 5.
- Regirock is a repeatable MVP endpoint.
- No EXP applies globally.
