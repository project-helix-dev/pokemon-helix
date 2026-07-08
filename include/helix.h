#ifndef GUARD_HELIX_H
#define GUARD_HELIX_H

#include "constants/species.h"

// Helix type IDs (matches TYPE_ constants from constants/types.h)
#define HELIX_TYPE_NORMAL    0
#define HELIX_TYPE_FIGHTING  1
#define HELIX_TYPE_FLYING    2
#define HELIX_TYPE_POISON    3
#define HELIX_TYPE_GROUND    4
#define HELIX_TYPE_ROCK      5
#define HELIX_TYPE_BUG       6
#define HELIX_TYPE_GHOST     7
#define HELIX_TYPE_STEEL     8
#define HELIX_TYPE_FIRE      9
#define HELIX_TYPE_WATER    10
#define HELIX_TYPE_GRASS    11
#define HELIX_TYPE_ELECTRIC 12
#define HELIX_TYPE_PSYCHIC  13
#define HELIX_TYPE_ICE      14
#define HELIX_TYPE_DRAGON   15
#define HELIX_TYPE_DARK     16
#define HELIX_TYPE_FAIRY    17
#define HELIX_NUM_TYPES     18

#define HELIX_STARTERS_PER_TYPE 3
#define HELIX_STARTING_FOOD    200
#define HELIX_QUIZ_ANSWERS     3
#define HELIX_QUIZ_COMBOS      (HELIX_QUIZ_ANSWERS * HELIX_QUIZ_ANSWERS * HELIX_QUIZ_ANSWERS) // 27
#define HELIX_MAX_TYPE_WEIGHTS 4

// Stray rarities (ordered from most to least common)
#define HELIX_RARITY_COMMON     0  // ~70% — standard pokeball, single-typed, weaker species
#define HELIX_RARITY_UNCOMMON   1  // ~22% — great ball, dual-typed, bread-and-butter team members
#define HELIX_RARITY_RARE       2  // ~7%  — ultra ball, powerful species with atypical abilities
#define HELIX_RARITY_LEGENDARY  3  // ~1%  — master ball, one legendary per type
#define HELIX_NUM_RARITIES      4

// Sentinel value: use default personality-based ability slot
#define STRAY_ABILITY_DEFAULT   0xFF

// One entry in a stray pool.  species == SPECIES_NONE is the sentinel.
// Set abilityNum to 0, 1, or 2 to force a specific ability slot, or
// STRAY_ABILITY_DEFAULT to let the engine choose from personality.
struct StrayEntry
{
    u16 species;
    u8  abilityNum;
};

// IV generation
#define HELIX_IV_MAX            5   // Max IV per stat for starters/strays (min is always 1)

// Breeding system
#define HELIX_DEFAULT_COMFORT   50  // Out of 100
#define HELIX_BREED_CHANCE_BASE 90  // DEMO: was 30 — Base % chance per male/female pair (before comfort modifier)
#define HELIX_EGG_HATCH_LEVEL   5   // Level of hatched mons

struct HelixStarterPool
{
    u16 species[HELIX_STARTERS_PER_TYPE];
};

struct HelixTypeWeight
{
    u8 typeId;
    u8 weight;
};

struct HelixQuizOutcome
{
    struct HelixTypeWeight options[HELIX_MAX_TYPE_WEIGHTS];
};

extern const struct HelixStarterPool gHelixStarterPools[HELIX_NUM_TYPES];
extern const u8 *const gHelixTypeNames[HELIX_NUM_TYPES];

void HelixGiveStartersForType(u8 typeId, u8 starter1Index, u8 starter2Index);
u16 HelixGetStarterSpecies(u8 typeId, u8 index);
u8 HelixComputeTypeFromQuiz(u8 q1, u8 q2, u8 q3);
void HelixStashNewGameState(u8 typeId, u8 starter1, u8 starter2);
void HelixApplyNewGameState(void);

// Population & Food
void HelixSpecial_CountPopulation(void);

// Breeding system — called as specials from the day-end script
void HelixSpecial_HatchEggs(void);
void HelixSpecial_BreedBoxes(void);

// Stray system
void HelixSpecial_GenerateStray(void);
void HelixSpecial_GenerateDay1Stray(void);
void HelixSpecial_GiveStray(void);

// Guide NPC
void HelixSetGuideGraphicsForLocalId(u8 localId);
void HelixSpecial_SetGuideGraphics(void);

#endif // GUARD_HELIX_H
