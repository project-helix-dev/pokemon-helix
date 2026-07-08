# Intro Cutscene & Quiz System — Technical Reference

This document describes the complete intro flow from the main menu through type assignment and starter selection.

## Overview

The intro runs entirely within `src/main_menu.c` as a series of task state functions, replacing the vanilla Birch speech. It does **not** use map scripts — the quiz, type computation, and starter selection all happen in C before the player enters the overworld.

## Flow Sequence

```
Task_NewGameBirchSpeech_Init
  → Birch sprite + background setup
  → gText_Birch_Welcome (Deoxys introduction)
  → gText_ThisIsAPokemon (Omanyte demonstration)
  → gText_Birch_MainSpeech (DNA/island theme)
  → Gender selection
  → Name entry
  → Name confirmation (Yes → quiz, No → re-enter name)

Task_HelixSpeech_QuizIntro
  → gText_Helix_QuizIntro
  → Quiz Q1: "What calls to you most strongly?" (Land / Sea / Sky)
  → Quiz Q2: "Which is the most powerful trait?" (Knowledge / Emotion / Willpower)
  → Quiz Q3: "What guides your path?" (Truth / Ideals / Balance)
  → HelixComputeTypeFromQuiz(q1, q2, q3)
  → HelixSpecial_BufferTypeName → gText_Helix_TypeResult

Task_HelixSpeech_ShowStarterMenu
  → Starter 1 selection (3-choice menu with sprite preview)
  → gText_Helix_StarterSecond (confirms first choice)
  → Starter 2 selection (same menu, sprite preview)
  → gText_Helix_GotStarters (confirms second choice)
  → HelixStashNewGameState(typeId, starter1, starter2)

Task_HelixSpeech_GiveStarters
  → Fanfare + final dialogue
  → gText_Birch_AreYouReady (farewell)
  → SetMainCallback2(CB2_NewGame)

CB2_NewGame (overworld.c)
  → NewGameInitData()  [wipes save block]
  → HelixApplyNewGameState()  [restores type, starters, vars, flags]
  → Player appears on HelixIsland
```

## Quiz System

### Questions and Answers

Each question has 3 choices, stored as indices 0–2:

| Question | Choice 0 | Choice 1 | Choice 2 |
|----------|----------|----------|----------|
| Q1 | The Land | The Sea | The Sky |
| Q2 | Knowledge | Emotion | Willpower |
| Q3 | Truth | Ideals | Balance |

### Combo Index Calculation

The three answers produce a combo index into a 27-entry probability table:

```
comboIndex = q1 * 9 + q2 * 3 + q3
```

### Weighted Probability Table

Each combo maps to a `HelixQuizOutcome` containing up to 4 weighted type options:

```c
struct HelixTypeWeight {
    u8 typeId;
    u8 weight;
};

struct HelixQuizOutcome {
    struct HelixTypeWeight options[HELIX_MAX_TYPE_WEIGHTS]; // max 4
};
```

A `{0, 0}` entry acts as a sentinel marking the end of the weight list for that combo. The RNG rolls against the cumulative weight sum to select the final type:

```c
roll = Random() % totalWeight;
// Walk options until cumulative weight exceeds roll
```

This means the same quiz answers can produce different types across playthroughs, with some types being rarer than others depending on the combo.

### Probability Table Layout

The 27 combos are organized as:

- **Indices 0–8**: Q1 = Land (0)
- **Indices 9–17**: Q1 = Sea (1)
- **Indices 18–26**: Q1 = Sky (2)

Within each Q1 group:
- Indices +0..+2: Q2 = Knowledge (0)
- Indices +3..+5: Q2 = Emotion (1)
- Indices +6..+8: Q2 = Willpower (2)

Within each Q2 group, Q3 selects the final index (0 = Truth, 1 = Ideals, 2 = Balance).

The full table is defined as `sQuizOutcomes[HELIX_QUIZ_COMBOS]` in `src/helix.c`.

## Starter Selection

After type assignment, the player selects 2 of 3 starters from their type's pool.

### Menu Presentation

Each quiz question and the starter selection use `AddMenuActionTextWindow` with 3 options. For the quiz, text labels are defined in `birch_speech.inc`. For starters, the species names are buffered dynamically via `HelixGetStarterSpecies` → `GetSpeciesName`.

### Sprite Preview

During starter selection, a mon pic sprite is created for the currently highlighted option:

- **Palette slot**: 13 (avoids conflicts with Birch/player sprites)
- **Position**: centered at (160, 68)
- The sprite is destroyed and recreated each time the cursor moves to a new option
- When the menu is dismissed, the sprite is destroyed and the player sprite is restored

Key functions:
- `HelixSpeech_UpdatePreviewSprite` — creates/swaps the preview sprite
- `HelixSpeech_DestroyPreviewSprite` — frees the sprite
- `HelixSpeech_HidePreviewShowPlayer` — restores player sprite visibility

### EWRAM State

```c
static EWRAM_DATA u8 sStarterPreviewSpriteId = 0;   // re-init to SPRITE_NONE before use
static EWRAM_DATA u8 sStarterPreviewCursorPos = 0;   // re-init to 0xFF before use
```

Both are re-initialized at the start of `HelixSpeech_ShowStarterMenu`.

## Task Data Slots

The intro task uses all 16 `data[]` slots. Helix-specific slots:

| Slot | Define | Usage |
|------|--------|-------|
| `data[3]` | `tHelixQuizQ3` | Reused from init (set to 0xFF at start, not read before quiz) |
| `data[12]` | `tHelixQuizQ1` | Quiz answer 1 (0–2) |
| `data[13]` | `tHelixQuizQ2` | Quiz answer 2 (0–2) |
| `data[14]` | `tHelixTypeId` | Computed type ID (0–17) |
| `data[15]` | `tHelixStarter1` | First starter index (0–2) |

Second starter index is passed directly to `HelixStashNewGameState` from the menu input and is not stored in a task data slot.

## Dialogue Text

All intro dialogue is in `data/text/birch_speech.inc`. Labels use the following conventions:

- `gText_Birch_*` — vanilla-inherited labels (Welcome, MainSpeech, BoyOrGirl, etc.)
- `gText_Helix_*` — Helix-specific labels (QuizIntro, QuizQ1, TypeResult, StarterIntro, etc.)

Text placeholders used:
- `{PLAYER}` / `{KUN}` — player name with gender suffix
- `{STR_VAR_1}` — type name (after BufferTypeName) or starter name
- `{STR_VAR_2}`, `{STR_VAR_3}` — additional starter names when buffered

## Stash/Apply Pattern

See `docs/architecture.md` for the detailed explanation of why quiz choices must be stashed in EWRAM and applied after the save wipe.
