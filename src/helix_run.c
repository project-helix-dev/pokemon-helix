#include "global.h"
#include "helix.h"
#include "helix_run.h"
#include "battle.h"
#include "event_data.h"
#include "main.h"
#include "money.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "string_util.h"
#include "constants/battle.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_event_ids.h"
#include "constants/maps.h"
#include "constants/opponents.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "constants/vars.h"

// ═══════════════════════════════════════════════════════════════════════
// Helix expedition (Nexus / Caves run) system.
// All run-specific behavior is gated behind VAR_HELIX_RUN_ACTIVE.
// See docs/helix_nexus_run_level_system_brief_final.md
// ═══════════════════════════════════════════════════════════════════════

// ── Data tables ─────────────────────────────────────────────────────────

// Cave room type weights (must sum to 100)
static const struct HelixWeightedRoomType sCaveRoomTypeWeights[] =
{
    { HELIX_RUN_ROOM_WILD,    33 },
    { HELIX_RUN_ROOM_TRAINER, 33 },
    { HELIX_RUN_ROOM_SHOP,    34 },
    { HELIX_RUN_ROOM_NONE,     0 }, // terminator
};

// Provisional cave route: exit direction of each random room (1..9).
// The fixed intro trainer room (index 0) always exits up.
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

// Shared wild/trainer species pool for the Caves biome
static const u16 sCaveSpeciesPool[] =
{
    SPECIES_DIGLETT,
    SPECIES_GEODUDE,
    SPECIES_SANDSHREW,
    SPECIES_NUMEL,
    SPECIES_ZUBAT,
};

// Trainer room variants → overworld sprite (battle data lives in trainers.party)
static const u16 sRunTrainerGfx[HELIX_RUN_TRAINER_VARIANT_COUNT] =
{
    [HELIX_RUN_TRAINER_HIKER]   = OBJ_EVENT_GFX_HIKER,
    [HELIX_RUN_TRAINER_CAMPER]  = OBJ_EVENT_GFX_CAMPER,
    [HELIX_RUN_TRAINER_NERD]    = OBJ_EVENT_GFX_SCHOOL_KID_M,
    [HELIX_RUN_TRAINER_OLD_MAN] = OBJ_EVENT_GFX_EXPERT_M,
};

// Random expedition nickname pools
static const u8 *const sRunNicknamesMale[] =
{
    COMPOUND_STRING("Basil"),
    COMPOUND_STRING("Cliff"),
    COMPOUND_STRING("Dusty"),
    COMPOUND_STRING("Flint"),
    COMPOUND_STRING("Gorm"),
    COMPOUND_STRING("Igor"),
    COMPOUND_STRING("Milo"),
    COMPOUND_STRING("Rex"),
};

static const u8 *const sRunNicknamesFemale[] =
{
    COMPOUND_STRING("Amber"),
    COMPOUND_STRING("Cora"),
    COMPOUND_STRING("Fern"),
    COMPOUND_STRING("Iris"),
    COMPOUND_STRING("Lily"),
    COMPOUND_STRING("Opal"),
    COMPOUND_STRING("Ruby"),
    COMPOUND_STRING("Wren"),
};

// Stat display names for the IV reward menu
static const u8 sStatName_HP[]    = _("HP");
static const u8 sStatName_Atk[]   = _("Attack");
static const u8 sStatName_Def[]   = _("Defense");
static const u8 sStatName_SpAtk[] = _("Sp. Atk");
static const u8 sStatName_SpDef[] = _("Sp. Def");
static const u8 sStatName_Speed[] = _("Speed");

static const u8 *const sStatNames[HELIX_STAT_COUNT] =
{
    [HELIX_STAT_HP]    = sStatName_HP,
    [HELIX_STAT_ATK]   = sStatName_Atk,
    [HELIX_STAT_DEF]   = sStatName_Def,
    [HELIX_STAT_SPATK] = sStatName_SpAtk,
    [HELIX_STAT_SPDEF] = sStatName_SpDef,
    [HELIX_STAT_SPEED] = sStatName_Speed,
};

// HelixStatId → MON_DATA IV request (engine IV order is HP/Atk/Def/Speed/SpAtk/SpDef)
static const u8 sStatToIVData[HELIX_STAT_COUNT] =
{
    [HELIX_STAT_HP]    = MON_DATA_HP_IV,
    [HELIX_STAT_ATK]   = MON_DATA_ATK_IV,
    [HELIX_STAT_DEF]   = MON_DATA_DEF_IV,
    [HELIX_STAT_SPATK] = MON_DATA_SPATK_IV,
    [HELIX_STAT_SPDEF] = MON_DATA_SPDEF_IV,
    [HELIX_STAT_SPEED] = MON_DATA_SPEED_IV,
};

// ── Basic state helpers ─────────────────────────────────────────────────

bool32 HelixIsRunActive(void)
{
    return VarGet(VAR_HELIX_RUN_ACTIVE) == HELIX_RUN_IN_PROGRESS;
}

bool32 HelixNoExpEnabled(void)
{
    return HELIX_NO_EXP;
}

// Map the Helix type id (0-17, no Mystery/Stellar) onto the engine's Type enum
static u32 HelixTypeToEngineType(u32 helixType)
{
    if (helixType >= HELIX_NUM_TYPES)
        helixType = HELIX_TYPE_NORMAL;
    if (helixType <= HELIX_TYPE_STEEL)
        return helixType + 1;           // TYPE_NONE offset
    return helixType + 2;               // also skip TYPE_MYSTERY
}

bool32 HelixSpeciesMatchesPlayerType(u16 species)
{
    u32 type = HelixTypeToEngineType(VarGet(VAR_HELIX_PLAYER_TYPE));

    return gSpeciesInfo[species].types[0] == type
        || gSpeciesInfo[species].types[1] == type;
}

static bool32 PlayerIsOnMap(u16 mapConst)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(mapConst)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(mapConst);
}

// Keep the EXP bar looking full: pin EXP to one point below level 6.
// EXP never changes in battle (HELIX_NO_EXP), so this value is permanent.
void HelixSetFixedExp(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 exp;

    if (species == SPECIES_NONE)
        return;
    exp = gExperienceTables[gSpeciesInfo[species].growthRate][HELIX_RUN_FIXED_LEVEL + 1] - 1;
    SetMonData(mon, MON_DATA_EXP, &exp);
}

// ── Retirement (Earth Ribbon = "returned from an expedition") ───────────

static bool32 IsMonRetiredFromRuns(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_EARTH_RIBBON) != 0;
}

static void MarkMonRetiredFromRuns(struct Pokemon *mon)
{
    bool8 ribbon = TRUE;
    SetMonData(mon, MON_DATA_EARTH_RIBBON, &ribbon);
}

// Usable = a real, non-egg Pokémon (eggs are ignored for run rules)
static bool32 IsUsableRunMon(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
        && !GetMonData(mon, MON_DATA_IS_EGG);
}

static u32 CountUsableRunPartyMons(void)
{
    u32 i, count = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (IsUsableRunMon(&gPlayerParty[i]))
            count++;
    }
    return count;
}

static bool32 PartyHasRetiredMon(void)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (IsUsableRunMon(&gPlayerParty[i]) && IsMonRetiredFromRuns(&gPlayerParty[i]))
            return TRUE;
    }
    return FALSE;
}

// ── Run state reset ─────────────────────────────────────────────────────

static void ResetRunVars(void)
{
    VarSet(VAR_HELIX_RUN_BIOME, HELIX_RUN_BIOME_CAVES);
    VarSet(VAR_HELIX_RUN_ROOM_INDEX, 0);
    VarSet(VAR_HELIX_RUN_ROOM_TYPE, HELIX_RUN_ROOM_NONE);
    VarSet(VAR_HELIX_RUN_ROOM_COMPLETE, 0);
    VarSet(VAR_HELIX_RUN_WILD_TARGET, 0);
    VarSet(VAR_HELIX_RUN_WILD_DONE, 0);
    VarSet(VAR_HELIX_RUN_WILD_FLED, 0);
    VarSet(VAR_HELIX_RUN_TRAINER_VARIANT, 0);
    VarSet(VAR_HELIX_RUN_MONEY_EARNED, 0);
    VarSet(VAR_HELIX_RUN_FOOD_EARNED, 0);
    VarSet(VAR_HELIX_RUN_LAST_KO, 0);
    VarSet(VAR_HELIX_RUN_REWARD_STAT_1, 0);
    VarSet(VAR_HELIX_RUN_REWARD_STAT_2, 0);
    VarSet(VAR_HELIX_RUN_REWARD_STAT_3, 0);
    VarSet(VAR_HELIX_RUN_ENDPOINT_REWARD, ITEM_NONE);
    VarSet(VAR_HELIX_RUN_EXIT_DIR, HELIX_RUN_DIR_UP);
}

// ── Nicknames ───────────────────────────────────────────────────────────

static void AssignRunNicknames(void)
{
    u32 i;
    u8 nickname[POKEMON_NAME_LENGTH + 1];

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        const u8 *newName;

        if (!IsUsableRunMon(mon))
            continue;

        // Preserve existing nicknames (default name == species name)
        GetMonData(mon, MON_DATA_NICKNAME, nickname);
        if (StringCompare(nickname, GetSpeciesName(species)) != 0)
            continue;

        if (GetMonGender(mon) == MON_FEMALE)
            newName = sRunNicknamesFemale[Random() % ARRAY_COUNT(sRunNicknamesFemale)];
        else if (GetMonGender(mon) == MON_MALE)
            newName = sRunNicknamesMale[Random() % ARRAY_COUNT(sRunNicknamesMale)];
        else if (Random() % 2)
            newName = sRunNicknamesFemale[Random() % ARRAY_COUNT(sRunNicknamesFemale)];
        else
            newName = sRunNicknamesMale[Random() % ARRAY_COUNT(sRunNicknamesMale)];

        SetMonData(mon, MON_DATA_NICKNAME, newName);
    }
}

// ── Entry validation and run start ──────────────────────────────────────

// VAR_RESULT = HelixRunEntryResult
void HelixSpecial_CheckRunEntry(void)
{
    u32 usable = CountUsableRunPartyMons();

    if (VarGet(VAR_HELIX_LAST_RUN_DAY) != 0
     && VarGet(VAR_HELIX_LAST_RUN_DAY) == VarGet(VAR_HELIX_DAY_COUNT))
        gSpecialVar_Result = HELIX_RUN_ENTRY_ALREADY_TODAY;
    else if (usable == 0)
        gSpecialVar_Result = HELIX_RUN_ENTRY_NO_PARTY;
    else if (usable > HELIX_RUN_PARTY_MAX)
        gSpecialVar_Result = HELIX_RUN_ENTRY_TOO_MANY;
    else if (PartyHasRetiredMon())
        gSpecialVar_Result = HELIX_RUN_ENTRY_RETIRED;
    else
        gSpecialVar_Result = HELIX_RUN_ENTRY_OK;
}

void HelixSpecial_StartCavesRun(void)
{
    ResetRunVars();
    VarSet(VAR_HELIX_RUN_ACTIVE, HELIX_RUN_IN_PROGRESS);
    VarSet(VAR_HELIX_RUN_ROOM_TYPE, HELIX_RUN_ROOM_FIXED_TRAINER);
    AssignRunNicknames();
}

// ── Room progression ────────────────────────────────────────────────────

static u32 RollWeightedRoomType(const struct HelixWeightedRoomType *table)
{
    u32 i, total = 0, roll, cumulative = 0;

    for (i = 0; table[i].weight != 0; i++)
        total += table[i].weight;
    if (total == 0)
        return HELIX_RUN_ROOM_WILD;

    roll = Random() % total;
    for (i = 0; table[i].weight != 0; i++)
    {
        cumulative += table[i].weight;
        if (roll < cumulative)
            return table[i].roomType;
    }
    return table[0].roomType;
}

// Advance to the next room: rolls its type and initializes per-room state.
// VAR_RESULT = new room type (script warps to the matching map).
void HelixSpecial_RollNextCaveRoom(void)
{
    u32 index = VarGet(VAR_HELIX_RUN_ROOM_INDEX) + 1;
    u32 roomType;

    VarSet(VAR_HELIX_RUN_ROOM_INDEX, index);

    if (index > HELIX_RUN_CAVE_RANDOM_ROOM_COUNT)
        roomType = HELIX_RUN_ROOM_BOSS;
    else
        roomType = RollWeightedRoomType(sCaveRoomTypeWeights);

    VarSet(VAR_HELIX_RUN_ROOM_TYPE, roomType);
    // Shop rooms have no objective; boss rooms complete via the Regirock battle.
    VarSet(VAR_HELIX_RUN_ROOM_COMPLETE, roomType == HELIX_RUN_ROOM_SHOP ? 1 : 0);
    VarSet(VAR_HELIX_RUN_EXIT_DIR,
           index <= HELIX_RUN_CAVE_RANDOM_ROOM_COUNT ? sCaveRouteExitDirs[index - 1] : HELIX_RUN_DIR_UP);
    VarSet(VAR_HELIX_RUN_LAST_KO, 0);
    VarSet(VAR_HELIX_RUN_WILD_DONE, 0);
    VarSet(VAR_HELIX_RUN_WILD_FLED, 0);

    if (roomType == HELIX_RUN_ROOM_WILD)
        VarSet(VAR_HELIX_RUN_WILD_TARGET,
               HELIX_RUN_WILD_MIN_ENCOUNTERS
               + Random() % (HELIX_RUN_WILD_MAX_ENCOUNTERS - HELIX_RUN_WILD_MIN_ENCOUNTERS + 1));
    else
        VarSet(VAR_HELIX_RUN_WILD_TARGET, 0);

    if (roomType == HELIX_RUN_ROOM_TRAINER)
        VarSet(VAR_HELIX_RUN_TRAINER_VARIANT, Random() % HELIX_RUN_TRAINER_VARIANT_COUNT);

    gSpecialVar_Result = roomType;
}

// Sets VAR_OBJ_GFX_ID_0 so the trainer room's object uses the right sprite.
void HelixSpecial_SetupRunTrainerGfx(void)
{
    u32 variant = VarGet(VAR_HELIX_RUN_TRAINER_VARIANT);

    if (variant >= HELIX_RUN_TRAINER_VARIANT_COUNT)
        variant = HELIX_RUN_TRAINER_HIKER;
    VarSet(VAR_OBJ_GFX_ID_0, sRunTrainerGfx[variant]);
}

// ── Faint deletion (Nuzlocke-style attrition) ───────────────────────────

// Removes fainted, non-egg party mons after a run battle. Returns count removed.
static u32 RemoveFaintedRunMons(void)
{
    u32 i, removed = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];

        if (IsUsableRunMon(mon) && GetMonData(mon, MON_DATA_HP) == 0)
        {
            ZeroMonData(mon);
            removed++;
        }
    }

    if (removed != 0)
    {
        CompactPartySlots();
        CalculatePlayerPartyCount();
    }
    return removed;
}

// ── Battle hooks ────────────────────────────────────────────────────────

// Called from Cmd_tryfaintmon when an opponent mon faints.
void HelixOnOpponentFainted(u32 attackerPartyIndex, bool32 attackerIsPlayerSide)
{
    if (!HelixIsRunActive() || !attackerIsPlayerSide || attackerPartyIndex >= PARTY_SIZE)
        return;
    VarSet(VAR_HELIX_RUN_LAST_KO, attackerPartyIndex + 1);
}

void HelixAddRunMoney(u32 amount)
{
    VarSet(VAR_HELIX_RUN_MONEY_EARNED, VarGet(VAR_HELIX_RUN_MONEY_EARNED) + amount);
}

// Called from CB2_EndWildBattle when the player was not defeated.
// Updates wild-room counters and signals the room's ON_FRAME script through
// VAR_TEMP_1 (battle resolved), VAR_TEMP_2 (food gained), VAR_TEMP_3 (mons lost).
void HelixOnWildBattleEnd(void)
{
    u32 food = 0;

    if (!HelixIsRunActive() || !PlayerIsOnMap(MAP_HELIX_CAVE_ROOM_WILD))
        return;

    switch (gBattleOutcome)
    {
    case B_OUTCOME_WON:
        food = HELIX_RUN_WILD_FOOD_MIN
             + Random() % (HELIX_RUN_WILD_FOOD_MAX - HELIX_RUN_WILD_FOOD_MIN + 1);
        VarSet(VAR_HELIX_RUN_FOOD_EARNED, VarGet(VAR_HELIX_RUN_FOOD_EARNED) + food);
        break;
    case B_OUTCOME_CAUGHT:
        // Capture counts toward completion; catch itself was handled in battle.
        break;
    case B_OUTCOME_RAN:
    case B_OUTCOME_PLAYER_TELEPORTED:
    case B_OUTCOME_MON_FLED:
        VarSet(VAR_HELIX_RUN_WILD_FLED, 1);
        break;
    default:
        return;
    }

    VarSet(VAR_HELIX_RUN_WILD_DONE, VarGet(VAR_HELIX_RUN_WILD_DONE) + 1);
    VarSet(VAR_TEMP_1, 1);
    VarSet(VAR_TEMP_2, food);
    VarSet(VAR_TEMP_3, RemoveFaintedRunMons());
}

// Called from CB2_EndScriptedWildBattle (Regirock) when not defeated.
void HelixOnScriptedWildBattleEnd(void)
{
    if (!HelixIsRunActive())
        return;
    RemoveFaintedRunMons();
}

// Script special: post-trainer-battle cleanup. VAR_RESULT = mons lost.
void HelixSpecial_OnRunTrainerWon(void)
{
    gSpecialVar_Result = 0;
    if (!HelixIsRunActive())
        return;
    gSpecialVar_Result = RemoveFaintedRunMons();
}

// Block further wild encounters once the wild room's objective is met.
bool32 HelixRunShouldBlockWildEncounters(void)
{
    if (!HelixIsRunActive() || !PlayerIsOnMap(MAP_HELIX_CAVE_ROOM_WILD))
        return FALSE;
    return VarGet(VAR_HELIX_RUN_ROOM_COMPLETE) != 0
        || VarGet(VAR_HELIX_RUN_WILD_DONE) >= VarGet(VAR_HELIX_RUN_WILD_TARGET);
}

// Normalize any wild mon generated during a run: straight 5 IVs, full EXP bar.
void HelixMaybeAdjustRunWildMon(struct Pokemon *mon)
{
    u32 i, iv;

    if (!HelixIsRunActive())
        return;

    iv = HELIX_RUN_ENEMY_IV_VALUE;
    for (i = 0; i < NUM_STATS; i++)
        SetMonData(mon, MON_DATA_HP_IV + i, &iv);
    HelixSetFixedExp(mon);
    CalculateMonStats(mon);
}

// Random trainer rooms reuse one trainer per class; the mon species is
// re-rolled from the cave pool here after the party is built.
void HelixMaybeRandomizeRunTrainerMon(struct Pokemon *party, u16 trainerNum)
{
    u16 species;
    u32 personality;

    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return;
    if (trainerNum < TRAINER_HELIX_CAVE_HIKER || trainerNum > TRAINER_HELIX_CAVE_OLD_MAN)
        return;

    species = sCaveSpeciesPool[Random() % ARRAY_COUNT(sCaveSpeciesPool)];
    personality = Random32();
    CreateMon(&party[0], species, HELIX_RUN_FIXED_LEVEL, personality, OTID_STRUCT_RANDOM_NO_SHINY);
    HelixMaybeAdjustRunWildMon(&party[0]);
}

// ── IV reward system ────────────────────────────────────────────────────

static struct Pokemon *GetIVRewardRecipient(void)
{
    u32 slot = VarGet(VAR_HELIX_RUN_LAST_KO);

    if (slot == 0 || slot > PARTY_SIZE)
        return NULL;

    // The recorded slot may have shifted or been deleted by faint attrition;
    // require a live, usable mon in that slot.
    if (!IsUsableRunMon(&gPlayerParty[slot - 1])
     || GetMonData(&gPlayerParty[slot - 1], MON_DATA_HP) == 0)
        return NULL;
    return &gPlayerParty[slot - 1];
}

// VAR_RESULT = TRUE if a valid last-KO recipient exists
void HelixSpecial_HasIVRewardRecipient(void)
{
    gSpecialVar_Result = GetIVRewardRecipient() != NULL;
}

// Buffers the recipient's nickname into STR_VAR_1
void HelixSpecial_BufferIVRecipientName(void)
{
    struct Pokemon *mon = GetIVRewardRecipient();

    if (mon != NULL)
        GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
}

// Rolls 3 distinct stats, stores them, and buffers their names into
// STR_VAR_1/2/3 for the dynmultichoice menu.
void HelixSpecial_PrepareIVRewardChoices(void)
{
    u8 stats[HELIX_STAT_COUNT];
    u32 i, j, temp;

    for (i = 0; i < HELIX_STAT_COUNT; i++)
        stats[i] = i;
    // Fisher-Yates shuffle, then take the first three
    for (i = HELIX_STAT_COUNT - 1; i > 0; i--)
    {
        j = Random() % (i + 1);
        temp = stats[i];
        stats[i] = stats[j];
        stats[j] = temp;
    }

    VarSet(VAR_HELIX_RUN_REWARD_STAT_1, stats[0]);
    VarSet(VAR_HELIX_RUN_REWARD_STAT_2, stats[1]);
    VarSet(VAR_HELIX_RUN_REWARD_STAT_3, stats[2]);
    StringCopy(gStringVar1, sStatNames[stats[0]]);
    StringCopy(gStringVar2, sStatNames[stats[1]]);
    StringCopy(gStringVar3, sStatNames[stats[2]]);
}

// Applies the chosen IV increase (VAR_RESULT = menu selection 0-2).
// Buffers STR_VAR_1 = nickname, STR_VAR_2 = stat name for the result message.
void HelixSpecial_ApplyIVRewardChoice(void)
{
    static const u16 sRewardStatVars[] =
    {
        VAR_HELIX_RUN_REWARD_STAT_1,
        VAR_HELIX_RUN_REWARD_STAT_2,
        VAR_HELIX_RUN_REWARD_STAT_3,
    };
    struct Pokemon *mon = GetIVRewardRecipient();
    u32 selection = gSpecialVar_Result;
    u32 statId, iv;

    if (mon == NULL)
        return;
    if (selection >= ARRAY_COUNT(sRewardStatVars))
        selection = 0;

    statId = VarGet(sRewardStatVars[selection]);
    if (statId >= HELIX_STAT_COUNT)
        statId = HELIX_STAT_HP;

    iv = GetMonData(mon, sStatToIVData[statId]);
    iv += HELIX_RUN_IV_REWARD_AMOUNT;
    if (iv > HELIX_RUN_IV_REWARD_CAP)
        iv = HELIX_RUN_IV_REWARD_CAP;
    SetMonData(mon, sStatToIVData[statId], &iv);
    CalculateMonStats(mon);

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringCopy(gStringVar2, sStatNames[statId]);

    // One reward per room
    VarSet(VAR_HELIX_RUN_LAST_KO, 0);
}

// ── Run completion / blackout ───────────────────────────────────────────

// 50/50 Moon Stone or Rocky Helmet, stored in VAR_HELIX_RUN_ENDPOINT_REWARD
void HelixSpecial_RollEndpointReward(void)
{
    if (Random() % 100 < HELIX_RUN_REGIROCK_ITEM_CHANCE_PERCENT)
        VarSet(VAR_HELIX_RUN_ENDPOINT_REWARD, ITEM_MOON_STONE);
    else
        VarSet(VAR_HELIX_RUN_ENDPOINT_REWARD, ITEM_ROCKY_HELMET);
}

// Buffers run summary numbers: STR_VAR_1 = money, STR_VAR_2 = food
void HelixSpecial_BufferRunSummary(void)
{
    ConvertIntToDecimalStringN(gStringVar1, VarGet(VAR_HELIX_RUN_MONEY_EARNED), STR_CONV_MODE_LEFT_ALIGN, 5);
    ConvertIntToDecimalStringN(gStringVar2, VarGet(VAR_HELIX_RUN_FOOD_EARNED), STR_CONV_MODE_LEFT_ALIGN, 5);
}

// Successful run: bank rewards, retire survivors, apply day lockout.
// The endpoint item is given by the script (giveitem) before this runs.
void HelixSpecial_CompleteCavesRun(void)
{
    u32 i;

    AddMoney(&gSaveBlock1Ptr->money, VarGet(VAR_HELIX_RUN_MONEY_EARNED));
    VarSet(VAR_HELIX_FOOD, VarGet(VAR_HELIX_FOOD) + VarGet(VAR_HELIX_RUN_FOOD_EARNED));

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (IsUsableRunMon(&gPlayerParty[i]))
            MarkMonRetiredFromRuns(&gPlayerParty[i]);
    }

    VarSet(VAR_HELIX_LAST_RUN_DAY, VarGet(VAR_HELIX_DAY_COUNT));
    ResetRunVars();
    VarSet(VAR_HELIX_RUN_ACTIVE, HELIX_RUN_INACTIVE);
}

// Blackout during a run: lose everything unbanked and the whole party.
// Called from DoWhiteOut; returns TRUE if the Helix path took over.
// The island's ON_FRAME script shows the message while
// VAR_HELIX_RUN_ACTIVE == HELIX_RUN_BLACKOUT_RETURN.
bool32 HelixTryHandleRunBlackout(void)
{
    if (!HelixIsRunActive())
        return FALSE;

    VarSet(VAR_HELIX_LAST_RUN_DAY, VarGet(VAR_HELIX_DAY_COUNT));
    ResetRunVars();
    VarSet(VAR_HELIX_RUN_ACTIVE, HELIX_RUN_BLACKOUT_RETURN);

    ZeroPlayerPartyMons();
    gPlayerPartyCount = 0;
    return TRUE;
}

// ── Nexus guide ─────────────────────────────────────────────────────────

void HelixSpecial_SetNexusGuideGraphics(void)
{
    HelixSetGuideGraphicsForLocalId(LOCALID_HELIX_NEXUS_GUIDE);
}
