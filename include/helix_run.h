#ifndef GUARD_HELIX_RUN_H
#define GUARD_HELIX_RUN_H

#include "pokemon.h"
#include "constants/helix_run.h"

// ═══════════════════════════════════════════════════════════════════════
// Helix expedition (Nexus / Caves run) system — see
// docs/helix_nexus_run_level_system_brief_final.md
// ═══════════════════════════════════════════════════════════════════════

// Tunables
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

// Global design rule: no Pokémon ever gains EXP (level-less IV progression).
#define HELIX_NO_EXP                           TRUE

// Run state / room type / direction / stat / trainer variant constants are
// shared with event scripts and live in constants/helix_run.h.

enum HelixRunBiome
{
    HELIX_RUN_BIOME_CAVES,
    HELIX_RUN_BIOME_COUNT,
};

struct HelixWeightedRoomType
{
    u8 roomType;
    u8 weight;
};

// ── Script specials (registered in data/specials.inc) ──────────────────
void HelixSpecial_SetNexusGuideGraphics(void);
void HelixSpecial_CheckRunEntry(void);
void HelixSpecial_StartCavesRun(void);
void HelixSpecial_RollNextCaveRoom(void);
void HelixSpecial_SetupRunTrainerGfx(void);
void HelixSpecial_OnRunTrainerWon(void);
void HelixSpecial_HasIVRewardRecipient(void);
void HelixSpecial_BufferIVRecipientName(void);
void HelixSpecial_PrepareIVRewardChoices(void);
void HelixSpecial_ApplyIVRewardChoice(void);
void HelixSpecial_RollEndpointReward(void);
void HelixSpecial_BufferRunSummary(void);
void HelixSpecial_CompleteCavesRun(void);

// ── C helpers used by battle / overworld hooks ─────────────────────────
bool32 HelixIsRunActive(void);
bool32 HelixNoExpEnabled(void);
bool32 HelixRunShouldBlockWildEncounters(void);
bool32 HelixSpeciesMatchesPlayerType(u16 species);
bool32 HelixRunBlocksBallThrow(void);
void HelixOnOpponentFainted(u32 attackerPartyIndex, bool32 attackerIsPlayerSide);
void HelixOnWildBattleEnd(void);
void HelixOnScriptedWildBattleEnd(void);
void HelixAddRunMoney(u32 amount);
void HelixMaybeAdjustRunWildMon(struct Pokemon *mon);
void HelixMaybeRandomizeRunTrainerMon(struct Pokemon *party, u16 trainerNum);
bool32 HelixTryHandleRunBlackout(void);
void HelixSetFixedExp(struct Pokemon *mon);

#endif // GUARD_HELIX_RUN_H
