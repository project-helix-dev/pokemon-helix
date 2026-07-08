#include "global.h"
#include "helix.h"
#include "helix_run.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "event_data.h"
#include "string_util.h"
#include "script_pokemon_util.h"
#include "random.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/species.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/region_map_sections.h"
#include "constants/event_objects.h"
#include "constants/map_event_ids.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "daycare.h"
#include "move.h"

// Stashed new-game choices — survive NewGameInitData save wipe, applied in CB2_NewGame.
static EWRAM_DATA u8 sStashedTypeId = 0;
static EWRAM_DATA u8 sStashedStarter1 = 0;
static EWRAM_DATA u8 sStashedStarter2 = 0;

const u8 gHelixTypeName_Normal[] = _("Normal");
const u8 gHelixTypeName_Fighting[] = _("Fighting");
const u8 gHelixTypeName_Flying[] = _("Flying");
const u8 gHelixTypeName_Poison[] = _("Poison");
const u8 gHelixTypeName_Ground[] = _("Ground");
const u8 gHelixTypeName_Rock[] = _("Rock");
const u8 gHelixTypeName_Bug[] = _("Bug");
const u8 gHelixTypeName_Ghost[] = _("Ghost");
const u8 gHelixTypeName_Steel[] = _("Steel");
const u8 gHelixTypeName_Fire[] = _("Fire");
const u8 gHelixTypeName_Water[] = _("Water");
const u8 gHelixTypeName_Grass[] = _("Grass");
const u8 gHelixTypeName_Electric[] = _("Electric");
const u8 gHelixTypeName_Psychic[] = _("Psychic");
const u8 gHelixTypeName_Ice[] = _("Ice");
const u8 gHelixTypeName_Dragon[] = _("Dragon");
const u8 gHelixTypeName_Dark[] = _("Dark");
const u8 gHelixTypeName_Fairy[] = _("Fairy");

const u8 *const gHelixTypeNames[HELIX_NUM_TYPES] =
{
    [HELIX_TYPE_NORMAL]   = gHelixTypeName_Normal,
    [HELIX_TYPE_FIGHTING] = gHelixTypeName_Fighting,
    [HELIX_TYPE_FLYING]   = gHelixTypeName_Flying,
    [HELIX_TYPE_POISON]   = gHelixTypeName_Poison,
    [HELIX_TYPE_GROUND]   = gHelixTypeName_Ground,
    [HELIX_TYPE_ROCK]     = gHelixTypeName_Rock,
    [HELIX_TYPE_BUG]      = gHelixTypeName_Bug,
    [HELIX_TYPE_GHOST]    = gHelixTypeName_Ghost,
    [HELIX_TYPE_STEEL]    = gHelixTypeName_Steel,
    [HELIX_TYPE_FIRE]     = gHelixTypeName_Fire,
    [HELIX_TYPE_WATER]    = gHelixTypeName_Water,
    [HELIX_TYPE_GRASS]    = gHelixTypeName_Grass,
    [HELIX_TYPE_ELECTRIC] = gHelixTypeName_Electric,
    [HELIX_TYPE_PSYCHIC]  = gHelixTypeName_Psychic,
    [HELIX_TYPE_ICE]      = gHelixTypeName_Ice,
    [HELIX_TYPE_DRAGON]   = gHelixTypeName_Dragon,
    [HELIX_TYPE_DARK]     = gHelixTypeName_Dark,
    [HELIX_TYPE_FAIRY]    = gHelixTypeName_Fairy,
};

// Starter pools from the design document
const struct HelixStarterPool gHelixStarterPools[HELIX_NUM_TYPES] =
{
    [HELIX_TYPE_NORMAL]   = {{ SPECIES_RATTATA,    SPECIES_AIPOM,      SPECIES_SKITTY     }},
    [HELIX_TYPE_FIGHTING] = {{ SPECIES_MANKEY,     SPECIES_TYROGUE,    SPECIES_MAKUHITA    }},
    [HELIX_TYPE_FLYING]   = {{ SPECIES_DODUO,      SPECIES_HOOTHOOT,   SPECIES_TAILLOW     }},
    [HELIX_TYPE_POISON]   = {{ SPECIES_EKANS,      SPECIES_SPINARAK,   SPECIES_GULPIN      }},
    [HELIX_TYPE_GROUND]   = {{ SPECIES_DIGLETT,    SPECIES_GLIGAR,     SPECIES_BALTOY      }},
    [HELIX_TYPE_ROCK]     = {{ SPECIES_DWEBBLE,    SPECIES_BONSLY,     SPECIES_NOSEPASS    }},
    [HELIX_TYPE_BUG]      = {{ SPECIES_VENONAT,    SPECIES_LEDYBA,     SPECIES_SURSKIT     }},
    [HELIX_TYPE_GHOST]    = {{ SPECIES_GOLETT,     SPECIES_MISDREAVUS, SPECIES_SHUPPET     }},
    [HELIX_TYPE_STEEL]    = {{ SPECIES_BRONZOR,    SPECIES_CUFANT,     SPECIES_VAROOM      }},
    [HELIX_TYPE_FIRE]     = {{ SPECIES_VULPIX,     SPECIES_HOUNDOUR,   SPECIES_NUMEL       }},
    [HELIX_TYPE_WATER]    = {{ SPECIES_KRABBY,     SPECIES_WOOPER,     SPECIES_WAILMER     }},
    [HELIX_TYPE_GRASS]    = {{ SPECIES_PARAS,      SPECIES_SUNKERN,    SPECIES_CACNEA      }},
    [HELIX_TYPE_ELECTRIC] = {{ SPECIES_VOLTORB,    SPECIES_CHINCHOU,   SPECIES_ELECTRIKE   }},
    [HELIX_TYPE_PSYCHIC]  = {{ SPECIES_DROWZEE,    SPECIES_NATU,       SPECIES_SPOINK      }},
    [HELIX_TYPE_ICE]      = {{ SPECIES_CUBCHOO,    SPECIES_SMOOCHUM,   SPECIES_SNORUNT     }},
    [HELIX_TYPE_DRAGON]   = {{ SPECIES_NOIBAT,     SPECIES_APPLIN,     SPECIES_DURALUDON   }},
    [HELIX_TYPE_DARK]     = {{ SPECIES_MURKROW,    SPECIES_SNEASEL,    SPECIES_POOCHYENA   }},
    [HELIX_TYPE_FAIRY]    = {{ SPECIES_SNUBBULL,   SPECIES_COTTONEE,   SPECIES_SWIRLIX     }},
};

u16 HelixGetStarterSpecies(u8 typeId, u8 index)
{
    if (typeId >= HELIX_NUM_TYPES)
        typeId = HELIX_TYPE_NORMAL;
    if (index >= HELIX_STARTERS_PER_TYPE)
        index = 0;
    return gHelixStarterPools[typeId].species[index];
}

// Neutral natures (no stat boost/penalty)
static const u8 sNeutralNatures[] = {
    NATURE_HARDY, NATURE_DOCILE, NATURE_SERIOUS, NATURE_BASHFUL, NATURE_QUIRKY
};

// Set each IV to a random value in [1, HELIX_IV_MAX]
static void HelixSetRandomIVs(struct Pokemon *mon)
{
    u32 i;
    for (i = 0; i < NUM_STATS; i++)
    {
        u32 iv = (Random() % HELIX_IV_MAX) + 1;
        SetMonData(mon, MON_DATA_HP_IV + i, &iv);
    }
    HelixSetFixedExp(mon); // level-less design: EXP bar pinned full
    CalculateMonStats(mon);
}

void HelixGiveStartersForType(u8 typeId, u8 starter1Index, u8 starter2Index)
{
    u16 species1 = HelixGetStarterSpecies(typeId, starter1Index);
    u16 species2 = HelixGetStarterSpecies(typeId, starter2Index);
    struct Pokemon mon;
    u8 nature;
    u32 personality;

    // First ancestor: neutral nature, random gender (avoids infinite loop on single-gender species)
    nature = sNeutralNatures[Random() % ARRAY_COUNT(sNeutralNatures)];
    personality = GetMonPersonality(species1, MON_GENDER_RANDOM, nature, RANDOM_UNOWN_LETTER);
    CreateMon(&mon, species1, 5, personality, OTID_STRUCT_PLAYER_ID);
    HelixSetRandomIVs(&mon);
    GiveMonInitialMoveset(&mon);
    GiveScriptedMonToPlayer(&mon, PARTY_SIZE);

    // Second ancestor: neutral nature, random gender
    nature = sNeutralNatures[Random() % ARRAY_COUNT(sNeutralNatures)];
    personality = GetMonPersonality(species2, MON_GENDER_RANDOM, nature, RANDOM_UNOWN_LETTER);
    CreateMon(&mon, species2, 5, personality, OTID_STRUCT_PLAYER_ID);
    HelixSetRandomIVs(&mon);
    GiveMonInitialMoveset(&mon);
    GiveScriptedMonToPlayer(&mon, PARTY_SIZE);
}

// Script special: Buffer the player's assigned type name into STR_VAR_1
void HelixSpecial_BufferTypeName(void)
{
    u8 typeId = VarGet(VAR_HELIX_PLAYER_TYPE);
    if (typeId >= HELIX_NUM_TYPES)
        typeId = HELIX_TYPE_NORMAL;
    StringCopy(gStringVar1, gHelixTypeNames[typeId]);
}

static const struct HelixQuizOutcome sQuizOutcomes[HELIX_QUIZ_COMBOS] =
{
    // Q1=Land(0), Q2=Knowledge(0), Q3=Truth(0)/Ideals(1)/Balance(2)
    [0]  = {{ { HELIX_TYPE_GROUND,   80 }, { HELIX_TYPE_ROCK,    10 }, { HELIX_TYPE_STEEL,    5 }, { HELIX_TYPE_GRASS,    5 } }},
    [1]  = {{ { HELIX_TYPE_ROCK,     80 }, { HELIX_TYPE_GROUND,  10 }, { HELIX_TYPE_FIRE,     5 }, { HELIX_TYPE_GRASS,    5 } }},
    [2]  = {{ { HELIX_TYPE_STEEL,    80 }, { HELIX_TYPE_NORMAL,  10 }, { HELIX_TYPE_ROCK,     5 }, { HELIX_TYPE_GRASS,    5 } }},

    // Q1=Land(0), Q2=Emotion(1), Q3=Truth(0)/Ideals(1)/Balance(2)
    [3]  = {{ { HELIX_TYPE_GROUND,   45 }, { HELIX_TYPE_FIRE,    25 }, { HELIX_TYPE_FIGHTING,15 }, { HELIX_TYPE_GRASS,   15 } }},
    [4]  = {{ { HELIX_TYPE_FIRE,     80 }, { HELIX_TYPE_GROUND,   5 }, { HELIX_TYPE_POISON,  10 }, { HELIX_TYPE_DRAGON,   5 } }},
    [5]  = {{ { HELIX_TYPE_NORMAL,   40 }, { HELIX_TYPE_GRASS,   30 }, { HELIX_TYPE_FIRE,    20 }, { HELIX_TYPE_FAIRY,   10 } }},

    // Q1=Land(0), Q2=Willpower(2), Q3=Truth(0)/Ideals(1)/Balance(2)
    [6]  = {{ { HELIX_TYPE_FIGHTING, 80 }, { HELIX_TYPE_ROCK,    10 }, { HELIX_TYPE_STEEL,   10 }, { 0, 0 } }},
    [7]  = {{ { HELIX_TYPE_POISON,   80 }, { HELIX_TYPE_GROUND,  10 }, { HELIX_TYPE_STEEL,   10 }, { 0, 0 } }},
    [8]  = {{ { HELIX_TYPE_NORMAL,   80 }, { HELIX_TYPE_GROUND,  10 }, { HELIX_TYPE_ROCK,    10 }, { 0, 0 } }},

    // Q1=Sea(1), Q2=Knowledge(0), Q3=Truth(0)/Ideals(1)/Balance(2)
    [9]  = {{ { HELIX_TYPE_WATER,    80 }, { HELIX_TYPE_PSYCHIC, 10 }, { HELIX_TYPE_ICE,     10 }, { 0, 0 } }},
    [10] = {{ { HELIX_TYPE_PSYCHIC,  80 }, { HELIX_TYPE_WATER,    5 }, { HELIX_TYPE_GHOST,   15 }, { 0, 0 } }},
    [11] = {{ { HELIX_TYPE_ICE,      80 }, { HELIX_TYPE_WATER,   10 }, { HELIX_TYPE_PSYCHIC,  5 }, { HELIX_TYPE_FAIRY,    5 } }},

    // Q1=Sea(1), Q2=Emotion(1), Q3=Truth(0)/Ideals(1)/Balance(2)
    [12] = {{ { HELIX_TYPE_GHOST,    80 }, { HELIX_TYPE_WATER,   10 }, { HELIX_TYPE_DARK,    10 }, { 0, 0 } }},
    [13] = {{ { HELIX_TYPE_DARK,     80 }, { HELIX_TYPE_WATER,    5 }, { HELIX_TYPE_POISON,  15 }, { 0, 0 } }},
    [14] = {{ { HELIX_TYPE_WATER,    40 }, { HELIX_TYPE_NORMAL,  20 }, { HELIX_TYPE_GHOST,   20 }, { HELIX_TYPE_FAIRY,   20 } }},

    // Q1=Sea(1), Q2=Willpower(2), Q3=Truth(0)/Ideals(1)/Balance(2)
    [15] = {{ { HELIX_TYPE_WATER,    40 }, { HELIX_TYPE_FIGHTING,35 }, { HELIX_TYPE_ICE,     25 }, { 0, 0 } }},
    [16] = {{ { HELIX_TYPE_ICE,      45 }, { HELIX_TYPE_WATER,   20 }, { HELIX_TYPE_STEEL,   35 }, { 0, 0 } }},
    [17] = {{ { HELIX_TYPE_NORMAL,   70 }, { HELIX_TYPE_WATER,   15 }, { HELIX_TYPE_ICE,     10 }, { HELIX_TYPE_FAIRY,    5 } }},

    // Q1=Sky(2), Q2=Knowledge(0), Q3=Truth(0)/Ideals(1)/Balance(2)
    [18] = {{ { HELIX_TYPE_FLYING,   80 }, { HELIX_TYPE_ELECTRIC,10 }, { HELIX_TYPE_PSYCHIC, 10 }, { 0, 0 } }},
    [19] = {{ { HELIX_TYPE_ELECTRIC, 80 }, { HELIX_TYPE_BUG,     15 }, { HELIX_TYPE_FLYING,   5 }, { 0, 0 } }},
    [20] = {{ { HELIX_TYPE_PSYCHIC,  45 }, { HELIX_TYPE_FLYING,  20 }, { HELIX_TYPE_ELECTRIC,20 }, { HELIX_TYPE_DRAGON,  15 } }},

    // Q1=Sky(2), Q2=Emotion(1), Q3=Truth(0)/Ideals(1)/Balance(2)
    [21] = {{ { HELIX_TYPE_FIRE,     35 }, { HELIX_TYPE_FLYING,  30 }, { HELIX_TYPE_DARK,    20 }, { HELIX_TYPE_GRASS,   15 } }},
    [22] = {{ { HELIX_TYPE_FIRE,     60 }, { HELIX_TYPE_FLYING,  15 }, { HELIX_TYPE_DARK,    15 }, { HELIX_TYPE_DRAGON,  10 } }},
    [23] = {{ { HELIX_TYPE_GRASS,    80 }, { HELIX_TYPE_BUG,     15 }, { HELIX_TYPE_FAIRY,    5 }, { 0, 0 } }},

    // Q1=Sky(2), Q2=Willpower(2), Q3=Truth(0)/Ideals(1)/Balance(2)
    [24] = {{ { HELIX_TYPE_FIGHTING, 40 }, { HELIX_TYPE_FLYING,  20 }, { HELIX_TYPE_ELECTRIC,25 }, { HELIX_TYPE_GRASS,   15 } }},
    [25] = {{ { HELIX_TYPE_ELECTRIC, 60 }, { HELIX_TYPE_STEEL,   20 }, { HELIX_TYPE_FLYING,   5 }, { HELIX_TYPE_DRAGON,  15 } }},
    [26] = {{ { HELIX_TYPE_BUG,      80 }, { HELIX_TYPE_GRASS,   15 }, { HELIX_TYPE_NORMAL,   5 }, { 0, 0 } }},
};

// C-callable: Compute type from quiz answers using weighted probability table.
// q1, q2, q3 are each 0-2. Looks up the combo and rolls RNG to pick a type.
u8 HelixComputeTypeFromQuiz(u8 q1, u8 q2, u8 q3)
{
    u8 comboIndex;
    u16 roll, cumulative;
    u16 totalWeight = 0;
    u8 i;
    const struct HelixQuizOutcome *outcome;

    if (q1 >= HELIX_QUIZ_ANSWERS) q1 = 0;
    if (q2 >= HELIX_QUIZ_ANSWERS) q2 = 0;
    if (q3 >= HELIX_QUIZ_ANSWERS) q3 = 0;

    comboIndex = q1 * 9 + q2 * 3 + q3;
    outcome = &sQuizOutcomes[comboIndex];

    for (i = 0; i < HELIX_MAX_TYPE_WEIGHTS; i++)
    {
        if (outcome->options[i].weight == 0)
            break;
        totalWeight += outcome->options[i].weight;
    }

    if (totalWeight == 0)
        return HELIX_TYPE_NORMAL;

    roll = Random() % totalWeight;
    cumulative = 0;
    for (i = 0; i < HELIX_MAX_TYPE_WEIGHTS; i++)
    {
        if (outcome->options[i].weight == 0)
            break;
        cumulative += outcome->options[i].weight;
        if (roll < cumulative)
            return outcome->options[i].typeId;
    }

    return outcome->options[0].typeId;
}

// Stash quiz/starter choices in EWRAM so they survive the NewGameInitData save wipe.
void HelixStashNewGameState(u8 typeId, u8 starter1, u8 starter2)
{
    sStashedTypeId = typeId;
    sStashedStarter1 = starter1;
    sStashedStarter2 = starter2;
}

// Apply stashed choices after NewGameInitData has cleared the save block.
void HelixApplyNewGameState(void)
{
    VarSet(VAR_HELIX_PLAYER_TYPE, sStashedTypeId);
    HelixGiveStartersForType(sStashedTypeId, sStashedStarter1, sStashedStarter2);
    VarSet(VAR_HELIX_FOOD, HELIX_STARTING_FOOD);
    VarSet(VAR_HELIX_DAY_COUNT, 1);
    VarSet(VAR_HELIX_ISLAND_POP, 2);
    VarSet(VAR_HELIX_INTRO_STATE, 2);
    VarSet(VAR_HELIX_COMFORT, HELIX_DEFAULT_COMFORT);
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_B_DASH);
}

// Script special: Buffer starter species names for the assigned type.
// Puts starter 0 name in STR_VAR_1, starter 1 in STR_VAR_2, starter 2 in STR_VAR_3
void HelixSpecial_BufferStarterNames(void)
{
    u8 typeId = VarGet(VAR_HELIX_PLAYER_TYPE);
    u16 s0 = HelixGetStarterSpecies(typeId, 0);
    u16 s1 = HelixGetStarterSpecies(typeId, 1);
    u16 s2 = HelixGetStarterSpecies(typeId, 2);
    StringCopy(gStringVar1, GetSpeciesName(s0));
    StringCopy(gStringVar2, GetSpeciesName(s1));
    StringCopy(gStringVar3, GetSpeciesName(s2));
}

// ========================================================================
// Helix Population & Food
// ========================================================================

// Count all Pokémon (including eggs) across all PC boxes and the player's party.
// Stores the result in VAR_HELIX_ISLAND_POP.
void HelixSpecial_CountPopulation(void)
{
    u16 count = 0;
    u8 box, slot;

    // Count party
    for (slot = 0; slot < PARTY_SIZE; slot++)
    {
        if (GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES) != SPECIES_NONE)
            count++;
    }

    // Count PC boxes
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (slot = 0; slot < IN_BOX_COUNT; slot++)
        {
            if (GetBoxMonDataAt(box, slot, MON_DATA_SPECIES) != SPECIES_NONE)
                count++;
        }
    }

    VarSet(VAR_HELIX_ISLAND_POP, count);
}

// ========================================================================
// Helix Breeding System
// ========================================================================

// Hatch all eggs across all boxes. Called BEFORE breeding on day-end.
// Each egg is replaced with a level 5 mon using the species/nature
// already baked into the egg's BoxPokemon data.
void HelixSpecial_HatchEggs(void)
{
    u8 box, slot;

    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (slot = 0; slot < IN_BOX_COUNT; slot++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(box, slot);
            u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);
            bool8 isEgg = GetBoxMonData(boxMon, MON_DATA_IS_EGG);

            if (species == SPECIES_NONE || !isEgg)
                continue;

            // Clear the egg flag — the mon "hatches"
            isEgg = FALSE;
            SetBoxMonData(boxMon, MON_DATA_IS_EGG, &isEgg);

            // Set the nickname to the species name (eggs are named "Egg")
            u8 speciesName[POKEMON_NAME_LENGTH + 1];
            StringCopy(speciesName, GetSpeciesName(species));
            SetBoxMonData(boxMon, MON_DATA_NICKNAME, speciesName);

            // Set the language so the name displays correctly
            SetBoxMonData(boxMon, MON_DATA_LANGUAGE, &gGameLanguage);
        }
    }
}

// Helper: check if eggMon already knows a move
static bool8 EggKnowsMove(struct Pokemon *mon, u16 move)
{
    u8 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (GetMonData(mon, MON_DATA_MOVE1 + i) == move)
            return TRUE;
    }
    return FALSE;
}

// Helper: add a move to the first empty slot. Returns TRUE if added.
static bool8 EggAddMove(struct Pokemon *mon, u16 move)
{
    u8 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (GetMonData(mon, MON_DATA_MOVE1 + i) == MOVE_NONE)
        {
            SetMonData(mon, MON_DATA_MOVE1 + i, &move);
            u8 pp = GetMovePP(move);
            SetMonData(mon, MON_DATA_PP1 + i, &pp);
            return TRUE;
        }
    }
    return FALSE;
}

// Helper: count non-NONE moves the egg currently has
static u8 EggMoveCount(struct Pokemon *mon)
{
    u8 count = 0;
    u8 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (GetMonData(mon, MON_DATA_MOVE1 + i) != MOVE_NONE)
            count++;
    }
    return count;
}

// Egg move inheritance: DEMO boosted (was 75/10/10/5, now 10/30/30/30)
static void InheritEggMoves(struct Pokemon *eggMon, u16 eggSpecies,
                            struct BoxPokemon *parent1, u16 species1,
                            struct BoxPokemon *parent2, u16 species2)
{
    u8 eggMoveRoll = Random() % 100;
    u8 i;

    if (EggMoveCount(eggMon) >= MAX_MON_MOVES)
        return;

    if (eggMoveRoll < 10)
    {
        // DEMO: 10% (was 75%): No moves inherited
        return;
    }
    else if (eggMoveRoll < 40)
    {
        // DEMO: 30% (was 10%): Random egg move from the child's species pool
        u16 poolMoves[EGG_MOVES_ARRAY_COUNT];
        u8 numPool = GetEggMovesBySpecies(eggSpecies, poolMoves);
        if (numPool == 0)
            return;

        u16 move = poolMoves[Random() % numPool];
        if (!EggKnowsMove(eggMon, move))
            EggAddMove(eggMon, move);
    }
    else if (eggMoveRoll < 70)
    {
        // DEMO: 30% (was 10%): Same-species parent egg move inheritance
        // First eligible move is auto-inherited (the category roll IS the roll).
        // Each subsequent eligible move gets an independent 10% chance.
        struct BoxPokemon *sameParents[2];
        u8 sameCount = 0;
        u8 p;
        bool8 firstInherited = FALSE;

        if (species1 == eggSpecies)
            sameParents[sameCount++] = parent1;
        if (species2 == eggSpecies)
            sameParents[sameCount++] = parent2;

        for (p = 0; p < sameCount; p++)
        {
            for (i = 0; i < MAX_MON_MOVES; i++)
            {
                u16 parentMove = GetBoxMonData(sameParents[p], MON_DATA_MOVE1 + i);
                if (parentMove == MOVE_NONE)
                    continue;
                if (!SpeciesCanLearnEggMove(eggSpecies, parentMove))
                    continue;
                if (EggKnowsMove(eggMon, parentMove))
                    continue;

                if (!firstInherited)
                {
                    firstInherited = TRUE;
                }
                else if (Random() % 100 >= 60) // DEMO: was 10
                {
                    continue;
                }

                if (!EggAddMove(eggMon, parentMove))
                    return; // No room
            }
        }
    }
    else
    {
        // DEMO: 30% (was 5%): Cross-species egg move inheritance
        // Pick a random move from the OTHER parent that is an egg move
        // of that parent's species.
        struct BoxPokemon *otherParent = NULL;
        u16 otherSpecies = SPECIES_NONE;

        if (species1 != eggSpecies)
        {
            otherParent = parent1;
            otherSpecies = species1;
        }
        else if (species2 != eggSpecies)
        {
            otherParent = parent2;
            otherSpecies = species2;
        }

        if (otherParent != NULL)
        {
            u16 eligible[MAX_MON_MOVES];
            u8 eligibleCount = 0;

            for (i = 0; i < MAX_MON_MOVES; i++)
            {
                u16 move = GetBoxMonData(otherParent, MON_DATA_MOVE1 + i);
                if (move == MOVE_NONE)
                    continue;
                if (!SpeciesCanLearnEggMove(otherSpecies, move))
                    continue;
                if (EggKnowsMove(eggMon, move))
                    continue;
                eligible[eligibleCount++] = move;
            }

            if (eligibleCount > 0)
                EggAddMove(eggMon, eligible[Random() % eligibleCount]);
        }
    }
}

// Create an egg from two parents and place it in the given box slot.
// Inheritance: species (50/50), nature (45/45/10), IVs (per-stat 50/50),
// shiny (rate based on parents), ability (DEMO: 25/15/30/30), egg moves (DEMO: 10/30/30/30).
static void CreateBreedingEgg(u8 box, u8 emptySlot, struct BoxPokemon *parent1, struct BoxPokemon *parent2)
{
    u16 species1 = GetBoxMonData(parent1, MON_DATA_SPECIES);
    u16 species2 = GetBoxMonData(parent2, MON_DATA_SPECIES);
    u8 i;

    // ── Species: 50/50 from each parent ──
    u16 eggSpecies = (Random() % 2 == 0) ? species1 : species2;

    // ── Nature: 45% parent1, 45% parent2, 10% fully random ──
    u32 personality1 = GetBoxMonData(parent1, MON_DATA_PERSONALITY);
    u32 personality2 = GetBoxMonData(parent2, MON_DATA_PERSONALITY);
    u8 natureRoll = Random() % 100;
    u8 eggNature;
    if (natureRoll < 45)
        eggNature = GetNatureFromPersonality(personality1);
    else if (natureRoll < 90)
        eggNature = GetNatureFromPersonality(personality2);
    else
        eggNature = Random() % NUM_NATURES;

    // ── Create base mon ──
    u32 eggPersonality = GetMonPersonality(eggSpecies, MON_GENDER_RANDOM, eggNature, RANDOM_UNOWN_LETTER);
    struct Pokemon eggMon;
    CreateMon(&eggMon, eggSpecies, HELIX_EGG_HATCH_LEVEL, eggPersonality, OTID_STRUCT_PLAYER_ID);

    // ── IVs: per-stat 50/50 from each parent ──
    for (i = 0; i < NUM_STATS; i++)
    {
        u32 iv = (Random() % 2 == 0)
            ? GetBoxMonData(parent1, MON_DATA_HP_IV + i)
            : GetBoxMonData(parent2, MON_DATA_HP_IV + i);
        SetMonData(&eggMon, MON_DATA_HP_IV + i, &iv);
    }

    // ── Shiny status ──
    {
        bool32 p1Shiny = GetBoxMonData(parent1, MON_DATA_IS_SHINY);
        bool32 p2Shiny = GetBoxMonData(parent2, MON_DATA_IS_SHINY);
        u16 shinyThreshold;
        bool8 isShiny;

        if (p1Shiny && p2Shiny)
            shinyThreshold = 100; // 1/10
        else if (p1Shiny || p2Shiny)
            shinyThreshold = 10;  // 1/100
        else
            shinyThreshold = 1;   // 1/1000

        isShiny = (Random() % 1000) < shinyThreshold;
        SetMonData(&eggMon, MON_DATA_IS_SHINY, &isShiny);
    }

    // ── Ability: DEMO boosted (was 50/30/10/10, now 25/15/30/30) ──
    {
        // Identify the same-species parent and the other parent
        struct BoxPokemon *sameParent;
        struct BoxPokemon *otherParent;
        if (species1 == eggSpecies && species2 == eggSpecies)
        {
            // Both parents are the same species — pick one as "same", one as "other"
            if (Random() % 2 == 0)
            { sameParent = parent1; otherParent = parent2; }
            else
            { sameParent = parent2; otherParent = parent1; }
        }
        else if (species1 == eggSpecies)
        { sameParent = parent1; otherParent = parent2; }
        else
        { sameParent = parent2; otherParent = parent1; }

        u8 abilityRoll = Random() % 100;

        if (abilityRoll < 25)
        {
            // DEMO: 25% (was 50%): Same ability as the same-species parent
            u16 parentOverride = GetBoxMonData(sameParent, MON_DATA_ABILITY_OVERRIDE);
            if (parentOverride != ABILITY_NONE)
            {
                // Parent has an override — pass it directly
                SetMonData(&eggMon, MON_DATA_ABILITY_OVERRIDE, &parentOverride);
            }
            else
            {
                // Copy the parent's ability slot (same species = same ability)
                u8 parentSlot = GetBoxMonData(sameParent, MON_DATA_ABILITY_NUM);
                SetMonData(&eggMon, MON_DATA_ABILITY_NUM, &parentSlot);
            }
        }
        else if (abilityRoll < 40)
        {
            // DEMO: 15% (was 30%): Natural generation — ability slot determined by personality (already set by CreateMon)
            // Nothing to do.
        }
        else if (abilityRoll < 70)
        {
            // DEMO: 30% (was 10%): Hidden ability (slot 2)
            u8 hiddenSlot = 2;
            // Only set if the species actually has a hidden ability
            if (GetSpeciesAbility(eggSpecies, hiddenSlot) != ABILITY_NONE)
                SetMonData(&eggMon, MON_DATA_ABILITY_NUM, &hiddenSlot);
        }
        else
        {
            // DEMO: 30% (was 10%): Cross-species transfer — copy the other parent's actual ability
            u16 otherOverride = GetBoxMonData(otherParent, MON_DATA_ABILITY_OVERRIDE);
            u16 otherAbility;
            if (otherOverride != ABILITY_NONE)
                otherAbility = otherOverride;
            else
            {
                u16 otherSpecies = GetBoxMonData(otherParent, MON_DATA_SPECIES);
                u8 otherSlot = GetBoxMonData(otherParent, MON_DATA_ABILITY_NUM);
                otherAbility = GetAbilityBySpecies(otherSpecies, otherSlot);
            }

            // Check if this ability exists in any slot of the egg's species
            u8 matchSlot;
            bool8 foundSlot = FALSE;
            for (matchSlot = 0; matchSlot < NUM_ABILITY_SLOTS; matchSlot++)
            {
                if (GetSpeciesAbility(eggSpecies, matchSlot) == otherAbility)
                {
                    SetMonData(&eggMon, MON_DATA_ABILITY_NUM, &matchSlot);
                    foundSlot = TRUE;
                    break;
                }
            }

            if (!foundSlot)
            {
                // Ability is foreign to this species — store as override
                SetMonData(&eggMon, MON_DATA_ABILITY_OVERRIDE, &otherAbility);
            }
        }
    }

    // Recalculate stats with inherited IVs
    HelixSetFixedExp(&eggMon); // level-less design: EXP bar pinned full
    CalculateMonStats(&eggMon);

    // ── Moveset: level-up moves first, then egg move inheritance ──
    GiveMonInitialMoveset(&eggMon);
    InheritEggMoves(&eggMon, eggSpecies, parent1, species1, parent2, species2);

    // ── Flag as egg ──
    {
        bool8 isEgg = TRUE;
        u8 language = LANGUAGE_JAPANESE;
        u8 metLocation = METLOC_HELIX_BRED;
        static const u8 sEggNickname[] = _("タマゴ");
        SetMonData(&eggMon, MON_DATA_IS_EGG, &isEgg);
        SetMonData(&eggMon, MON_DATA_NICKNAME, sEggNickname);
        SetMonData(&eggMon, MON_DATA_LANGUAGE, &language);
        SetMonData(&eggMon, MON_DATA_MET_LOCATION, &metLocation);
    }

    // Place the egg in the empty box slot
    SetBoxMonAt(box, emptySlot, &eggMon.box);
}

// For each box independently, each female and genderless mon rolls to produce an egg.
// Females pair with a random male or genderless mon in the same box.
// Genderless mons pair with a random male or genderless mon (excluding themselves).
// Each mother produces at most one egg per day.
void HelixSpecial_BreedBoxes(void)
{
    u8 box, slot, i;
    u16 comfort = VarGet(VAR_HELIX_COMFORT);

    // Effective breed chance: base + comfort/4, capped at 90%
    u16 breedChance = HELIX_BREED_CHANCE_BASE + (comfort / 4);
    if (breedChance > 90)
        breedChance = 90;

    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        // Collect mons by gender category
        u8 femaleSlots[IN_BOX_COUNT];
        u8 maleSlots[IN_BOX_COUNT];
        u8 genderlessSlots[IN_BOX_COUNT];
        u8 femaleCount = 0;
        u8 maleCount = 0;
        u8 genderlessCount = 0;

        for (slot = 0; slot < IN_BOX_COUNT; slot++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(box, slot);
            u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);

            if (species == SPECIES_NONE)
                continue;
            if (GetBoxMonData(boxMon, MON_DATA_IS_EGG))
                continue;

            u8 gender = GetBoxMonGender(boxMon);
            if (gender == MON_FEMALE)
                femaleSlots[femaleCount++] = slot;
            else if (gender == MON_MALE)
                maleSlots[maleCount++] = slot;
            else
                genderlessSlots[genderlessCount++] = slot;
        }

        // Partner pool for females: all males + all genderless
        u8 partnerCount = maleCount + genderlessCount;
        if (partnerCount == 0 && femaleCount == 0)
            continue;

        // Phase 1: Each female rolls once, picks a random male or genderless partner
        if (partnerCount > 0)
        {
            for (i = 0; i < femaleCount; i++)
            {
                s16 emptySlot = GetFirstFreeBoxSpot(box);
                if (emptySlot < 0)
                    break; // Box is full

                if (Random() % 100 >= breedChance)
                    continue;

                // Pick a random partner from males + genderless
                u8 partnerIdx = Random() % partnerCount;
                u8 partnerSlot;
                if (partnerIdx < maleCount)
                    partnerSlot = maleSlots[partnerIdx];
                else
                    partnerSlot = genderlessSlots[partnerIdx - maleCount];

                struct BoxPokemon *mother = GetBoxedMonPtr(box, femaleSlots[i]);
                struct BoxPokemon *father = GetBoxedMonPtr(box, partnerSlot);
                CreateBreedingEgg(box, (u8)emptySlot, mother, father);
            }
        }

        // Phase 2: Each genderless mon rolls once, picks a random male or genderless partner (not itself)
        if (maleCount + genderlessCount >= 2 || (genderlessCount >= 1 && maleCount >= 1))
        {
            for (i = 0; i < genderlessCount; i++)
            {
                s16 emptySlot = GetFirstFreeBoxSpot(box);
                if (emptySlot < 0)
                    break; // Box is full

                if (Random() % 100 >= breedChance)
                    continue;

                // Partner pool: males + genderless excluding self
                // Count eligible partners (partnerCount minus self = partnerCount - 1,
                // since self is in the genderless list)
                u8 eligibleCount = partnerCount - 1;
                if (eligibleCount == 0)
                    continue;

                u8 pick = Random() % eligibleCount;

                // Walk the combined male+genderless list, skipping self
                u8 partnerSlot;
                u8 seen = 0;
                u8 j;
                for (j = 0; j < maleCount; j++)
                {
                    if (seen == pick)
                    {
                        partnerSlot = maleSlots[j];
                        goto foundPartner;
                    }
                    seen++;
                }
                for (j = 0; j < genderlessCount; j++)
                {
                    if (j == i)
                        continue; // Skip self
                    if (seen == pick)
                    {
                        partnerSlot = genderlessSlots[j];
                        goto foundPartner;
                    }
                    seen++;
                }
                continue; // Shouldn't reach here

            foundPartner:;
                struct BoxPokemon *mother = GetBoxedMonPtr(box, genderlessSlots[i]);
                struct BoxPokemon *father = GetBoxedMonPtr(box, partnerSlot);
                CreateBreedingEgg(box, (u8)emptySlot, mother, father);
            }
        }
    }
}

// ── Stray System ─────────────────────────────────────────────────────────
// Per-type stray pools, split by rarity.
// Each array is terminated by a sentinel entry with species == SPECIES_NONE.
// To customise a stray: set abilityNum to 0/1/2 to force a specific ability
// slot, or STRAY_ABILITY_DEFAULT to let personality decide.
// If forcing a non-native ability, also edit the species' abilities[] in
// src/data/pokemon/species_info/ to put that ability in the chosen slot.
//
// Rarity distribution (roll 0..99):
//   0        (1%)  = legendary
//   1..7     (7%)  = rare
//   8..29    (22%) = uncommon
//   30..99   (70%) = common

#define S(sp)                   { sp, STRAY_ABILITY_DEFAULT }
#define S_AB(sp, slot)          { sp, slot }
#define STRAY_END               { SPECIES_NONE, 0 }

// ── Normal ───────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Normal_Common[]    = { S(SPECIES_RATTATA), S(SPECIES_AIPOM), S(SPECIES_TEDDIURSA), S(SPECIES_WHISMUR), S(SPECIES_SKITTY), S(SPECIES_LILLIPUP), S(SPECIES_MINCCINO), STRAY_END };
static const struct StrayEntry sStrayPool_Normal_Uncommon[]  = { S(SPECIES_PORYGON), S(SPECIES_TAUROS), S(SPECIES_KANGASKHAN), S(SPECIES_DUNSPARCE), S(SPECIES_MUNCHLAX), STRAY_END };
static const struct StrayEntry sStrayPool_Normal_Rare[]      = { S(SPECIES_SMEARGLE), S(SPECIES_SLAKOTH), S(SPECIES_TANDEMAUS), STRAY_END };
static const struct StrayEntry sStrayPool_Normal_Legendary[] = { S(SPECIES_REGIGIGAS), STRAY_END };

// ── Fighting ─────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Fighting_Common[]    = { S(SPECIES_MANKEY), S(SPECIES_MACHOP), S(SPECIES_MAKUHITA), S(SPECIES_TIMBURR), S(SPECIES_TYROGUE), S(SPECIES_PANCHAM), S(SPECIES_CRABRAWLER), STRAY_END };
static const struct StrayEntry sStrayPool_Fighting_Uncommon[]  = { S(SPECIES_MIENFOO), S(SPECIES_CLOBBOPUS), S(SPECIES_THROH), S(SPECIES_SAWK), S(SPECIES_PASSIMIAN), STRAY_END };
static const struct StrayEntry sStrayPool_Fighting_Rare[]      = { S(SPECIES_RIOLU), S(SPECIES_STUFFUL), S(SPECIES_FALINKS), STRAY_END };
static const struct StrayEntry sStrayPool_Fighting_Legendary[] = { S(SPECIES_TERRAKION), STRAY_END };

// ── Flying ───────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Flying_Common[]    = { S(SPECIES_PIDGEY), S(SPECIES_SPEAROW), S(SPECIES_PIDOVE), S(SPECIES_DODUO), S(SPECIES_HOOTHOOT), S(SPECIES_TAILLOW), S(SPECIES_STARLY), STRAY_END };
static const struct StrayEntry sStrayPool_Flying_Uncommon[]  = { S(SPECIES_WATTREL), S(SPECIES_ROOKIDEE), S(SPECIES_FLETCHLING), S(SPECIES_RUFFLET), S(SPECIES_DUCKLETT), STRAY_END };
static const struct StrayEntry sStrayPool_Flying_Rare[]      = { S(SPECIES_SIGILYPH), S(SPECIES_CHATOT), S(SPECIES_HAWLUCHA), STRAY_END };
static const struct StrayEntry sStrayPool_Flying_Legendary[] = { S(SPECIES_LUGIA), STRAY_END };

// ── Poison ───────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Poison_Common[]    = { S(SPECIES_EKANS), S(SPECIES_NIDORAN_M), S(SPECIES_NIDORAN_F), S(SPECIES_GRIMER), S(SPECIES_TRUBBISH), S(SPECIES_ZUBAT), S(SPECIES_BUDEW), STRAY_END };
static const struct StrayEntry sStrayPool_Poison_Uncommon[]  = { S(SPECIES_GULPIN), S(SPECIES_SEVIPER), S(SPECIES_STUNKY), S(SPECIES_SALANDIT), S(SPECIES_QWILFISH), STRAY_END };
static const struct StrayEntry sStrayPool_Poison_Rare[]      = { S(SPECIES_POIPOLE), S(SPECIES_CROAGUNK), S(SPECIES_SHROODLE), STRAY_END };
static const struct StrayEntry sStrayPool_Poison_Legendary[] = { S(SPECIES_ETERNATUS), STRAY_END };

// ── Ground ───────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Ground_Common[]    = { S(SPECIES_SANDSHREW), S(SPECIES_DIGLETT), S(SPECIES_CUBONE), S(SPECIES_PHANPY), S(SPECIES_BALTOY), S(SPECIES_GLIGAR), S(SPECIES_DRILBUR), STRAY_END };
static const struct StrayEntry sStrayPool_Ground_Uncommon[]  = { S(SPECIES_TRAPINCH), S(SPECIES_SILICOBRA), S(SPECIES_MUDBRAY), S(SPECIES_SANDILE), S(SPECIES_STUNFISK), STRAY_END };
static const struct StrayEntry sStrayPool_Ground_Rare[]      = { S(SPECIES_SANDYGAST), S(SPECIES_HIPPOPOTAS), S(SPECIES_TOEDSCOOL), STRAY_END };
static const struct StrayEntry sStrayPool_Ground_Legendary[] = { S(SPECIES_GROUDON), STRAY_END };

// ── Rock ─────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Rock_Common[]    = { S(SPECIES_NOSEPASS), S(SPECIES_BONSLY), S(SPECIES_ROGGENROLA), S(SPECIES_NACLI), S(SPECIES_DWEBBLE), S(SPECIES_GEODUDE), S(SPECIES_KLAWF), STRAY_END };
static const struct StrayEntry sStrayPool_Rock_Uncommon[]  = { S(SPECIES_ROCKRUFF), S(SPECIES_ROLYCOLY), S(SPECIES_ARON), S(SPECIES_ONIX), S(SPECIES_RHYHORN), STRAY_END };
static const struct StrayEntry sStrayPool_Rock_Rare[]      = { S(SPECIES_LARVITAR), S(SPECIES_SOLROCK), S(SPECIES_LUNATONE), STRAY_END };
static const struct StrayEntry sStrayPool_Rock_Legendary[] = { S(SPECIES_REGIROCK), STRAY_END };

// ── Bug ──────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Bug_Common[]    = { S(SPECIES_CATERPIE), S(SPECIES_WEEDLE), S(SPECIES_NINCADA), S(SPECIES_VENONAT), S(SPECIES_LEDYBA), S(SPECIES_SURSKIT), S(SPECIES_WURMPLE), STRAY_END };
static const struct StrayEntry sStrayPool_Bug_Uncommon[]  = { S(SPECIES_SNOM), S(SPECIES_CUTIEFLY), S(SPECIES_BLIPBUG), S(SPECIES_SHELMET), S(SPECIES_KARRABLAST), STRAY_END };
static const struct StrayEntry sStrayPool_Bug_Rare[]      = { S(SPECIES_PINSIR), S(SPECIES_SCYTHER), S(SPECIES_HERACROSS), STRAY_END };
static const struct StrayEntry sStrayPool_Bug_Legendary[] = { S(SPECIES_LARVESTA), STRAY_END };

// ── Ghost ────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Ghost_Common[]    = { S(SPECIES_GASTLY), S(SPECIES_MISDREAVUS), S(SPECIES_SHUPPET), S(SPECIES_DUSKULL), S(SPECIES_DRIFLOON), S(SPECIES_GOLETT), S(SPECIES_YAMASK), STRAY_END };
static const struct StrayEntry sStrayPool_Ghost_Uncommon[]  = { S(SPECIES_LITWICK), S(SPECIES_FRILLISH), S(SPECIES_HONEDGE), S(SPECIES_GREAVARD), S(SPECIES_PHANTUMP), STRAY_END };
static const struct StrayEntry sStrayPool_Ghost_Rare[]      = { S(SPECIES_SINISTEA), S(SPECIES_DREEPY), S(SPECIES_POLTCHAGEIST), STRAY_END };
static const struct StrayEntry sStrayPool_Ghost_Legendary[] = { S(SPECIES_MARSHADOW), STRAY_END };

// ── Steel ────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Steel_Common[]    = { S(SPECIES_MAGNEMITE), S(SPECIES_ARON), S(SPECIES_VAROOM), S(SPECIES_CUFANT), S(SPECIES_BRONZOR), S(SPECIES_KLINK), S(SPECIES_BELDUM), STRAY_END };
static const struct StrayEntry sStrayPool_Steel_Uncommon[]  = { S(SPECIES_FERROSEED), S(SPECIES_MAWILE), S(SPECIES_SKARMORY), S(SPECIES_KLEFKI), S(SPECIES_DURANT), STRAY_END };
static const struct StrayEntry sStrayPool_Steel_Rare[]      = { S(SPECIES_MELTAN), S(SPECIES_ORTHWORM), S(SPECIES_TINKATINK), STRAY_END };
static const struct StrayEntry sStrayPool_Steel_Legendary[] = { S(SPECIES_REGISTEEL), STRAY_END };

// ── Fire ─────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Fire_Common[]    = { S(SPECIES_VULPIX), S(SPECIES_GROWLITHE), S(SPECIES_PONYTA), S(SPECIES_SLUGMA), S(SPECIES_PANSEAR), S(SPECIES_HOUNDOUR), S(SPECIES_NUMEL), STRAY_END };
static const struct StrayEntry sStrayPool_Fire_Uncommon[]  = { S(SPECIES_MAGBY), S(SPECIES_CHIMCHAR), S(SPECIES_TEPIG), S(SPECIES_DARUMAKA), S(SPECIES_LITWICK), STRAY_END };
static const struct StrayEntry sStrayPool_Fire_Rare[]      = { S_AB(SPECIES_CHARMANDER, 2), S(SPECIES_CYNDAQUIL), S(SPECIES_TORCHIC), STRAY_END };
static const struct StrayEntry sStrayPool_Fire_Legendary[] = { S(SPECIES_ENTEI), STRAY_END };

// ── Water ────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Water_Common[]    = { S(SPECIES_KRABBY), S(SPECIES_WAILMER), S(SPECIES_WOOPER), S(SPECIES_SEEL), S(SPECIES_MAGIKARP), S(SPECIES_BUIZEL), S(SPECIES_PANPOUR), STRAY_END };
static const struct StrayEntry sStrayPool_Water_Uncommon[]  = { S(SPECIES_SQUIRTLE), S(SPECIES_TOTODILE), S(SPECIES_CORPHISH), S(SPECIES_FEEBAS), S(SPECIES_FROAKIE), STRAY_END };
static const struct StrayEntry sStrayPool_Water_Rare[]      = { S(SPECIES_MUDKIP), S(SPECIES_PIPLUP), S(SPECIES_OSHAWOTT), STRAY_END };
static const struct StrayEntry sStrayPool_Water_Legendary[] = { S(SPECIES_SUICUNE), STRAY_END };

// ── Grass ────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Grass_Common[]    = { S(SPECIES_TANGELA), S(SPECIES_SUNKERN), S(SPECIES_CACNEA), S(SPECIES_PANSAGE), S(SPECIES_FOMANTIS), S(SPECIES_BOUNSWEET), S(SPECIES_PARAS), STRAY_END };
static const struct StrayEntry sStrayPool_Grass_Uncommon[]  = { S(SPECIES_BULBASAUR), S(SPECIES_CHIKORITA), S(SPECIES_SEEDOT), S(SPECIES_CAPSAKID), S(SPECIES_SHROOMISH), STRAY_END };
static const struct StrayEntry sStrayPool_Grass_Rare[]      = { S(SPECIES_TREECKO), S(SPECIES_TURTWIG), S(SPECIES_SNIVY), STRAY_END };
static const struct StrayEntry sStrayPool_Grass_Legendary[] = { S(SPECIES_VIRIZION), STRAY_END };

// ── Electric ─────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Electric_Common[]    = { S(SPECIES_VOLTORB), S(SPECIES_MAREEP), S(SPECIES_ELEKID), S(SPECIES_ELECTRIKE), S(SPECIES_TYNAMO), S(SPECIES_CHINCHOU), S(SPECIES_PICHU), STRAY_END };
static const struct StrayEntry sStrayPool_Electric_Uncommon[]  = { S(SPECIES_SHINX), S(SPECIES_BLITZLE), S(SPECIES_PAWMI), S(SPECIES_JOLTIK), S(SPECIES_DEDENNE), STRAY_END };
static const struct StrayEntry sStrayPool_Electric_Rare[]      = { S(SPECIES_TOXEL), S(SPECIES_TOGEDEMARU), S(SPECIES_ROTOM), STRAY_END };
static const struct StrayEntry sStrayPool_Electric_Legendary[] = { S(SPECIES_RAIKOU), STRAY_END };

// ── Psychic ──────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Psychic_Common[]    = { S(SPECIES_ABRA), S(SPECIES_DROWZEE), S(SPECIES_SPOINK), S(SPECIES_NATU), S(SPECIES_GOTHITA), S(SPECIES_SOLOSIS), S(SPECIES_HATENNA), STRAY_END };
static const struct StrayEntry sStrayPool_Psychic_Uncommon[]  = { S(SPECIES_GIRAFARIG), S(SPECIES_ELGYEM), S(SPECIES_MEDITITE), S(SPECIES_RALTS), S(SPECIES_ESPURR), STRAY_END };
static const struct StrayEntry sStrayPool_Psychic_Rare[]      = { S(SPECIES_WYNAUT), S(SPECIES_SMOOCHUM), S(SPECIES_MIME_JR), STRAY_END };
static const struct StrayEntry sStrayPool_Psychic_Legendary[] = { S(SPECIES_CRESSELIA), STRAY_END };

// ── Ice ──────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Ice_Common[]    = { S(SPECIES_SNORUNT), S(SPECIES_SPHEAL), S(SPECIES_SWINUB), S(SPECIES_SNEASEL), S(SPECIES_SMOOCHUM), S(SPECIES_CUBCHOO), S(SPECIES_BERGMITE), STRAY_END };
static const struct StrayEntry sStrayPool_Ice_Uncommon[]  = { S(SPECIES_DELIBIRD), S(SPECIES_CRYOGONAL), S(SPECIES_CETODDLE), S(SPECIES_VANILLITE), S(SPECIES_SNOVER), STRAY_END };
static const struct StrayEntry sStrayPool_Ice_Rare[]      = { S(SPECIES_LAPRAS), S(SPECIES_EISCUE), S(SPECIES_GLASTRIER), STRAY_END };
static const struct StrayEntry sStrayPool_Ice_Legendary[] = { S(SPECIES_REGICE), STRAY_END };

// ── Dragon ───────────────────────────────────────────────────────────────
// Tatsugiri: stored as base form; a random color form is chosen at give time.
static const struct StrayEntry sStrayPool_Dragon_Common[]    = { S(SPECIES_DRATINI), S(SPECIES_BAGON), S(SPECIES_GIBLE), S(SPECIES_AXEW), S(SPECIES_DURALUDON), S(SPECIES_GOOMY), S(SPECIES_APPLIN), STRAY_END };
static const struct StrayEntry sStrayPool_Dragon_Uncommon[]  = { S(SPECIES_JANGMO_O), S(SPECIES_DEINO), S(SPECIES_FRIGIBAX), S(SPECIES_TATSUGIRI), S(SPECIES_DRAMPA), STRAY_END };
static const struct StrayEntry sStrayPool_Dragon_Rare[]      = { S(SPECIES_DRUDDIGON), S(SPECIES_TURTONATOR), S(SPECIES_CYCLIZAR), STRAY_END };
static const struct StrayEntry sStrayPool_Dragon_Legendary[] = { S(SPECIES_REGIDRAGO), STRAY_END };

// ── Dark ─────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Dark_Common[]    = { S(SPECIES_MURKROW), S(SPECIES_HOUNDOUR), S(SPECIES_POOCHYENA), S(SPECIES_SNEASEL), S(SPECIES_PURRLOIN), S(SPECIES_MASCHIFF), S(SPECIES_PAWNIARD), STRAY_END };
static const struct StrayEntry sStrayPool_Dark_Uncommon[]  = { S(SPECIES_ABSOL), S(SPECIES_VULLABY), S(SPECIES_NICKIT), S(SPECIES_SCRAGGY), S(SPECIES_CARVANHA), STRAY_END };
static const struct StrayEntry sStrayPool_Dark_Rare[]      = { S(SPECIES_INKAY), S(SPECIES_MORPEKO), S(SPECIES_ZORUA), STRAY_END };
static const struct StrayEntry sStrayPool_Dark_Legendary[] = { S(SPECIES_DARKRAI), STRAY_END };

// ── Fairy ────────────────────────────────────────────────────────────────
static const struct StrayEntry sStrayPool_Fairy_Common[]    = { S(SPECIES_SNUBBULL), S(SPECIES_COTTONEE), S(SPECIES_SWIRLIX), S(SPECIES_IMPIDIMP), S(SPECIES_TOGEPI), S(SPECIES_FLABEBE), S(SPECIES_SPRITZEE), STRAY_END };
static const struct StrayEntry sStrayPool_Fairy_Uncommon[]  = { S(SPECIES_CLEFFA), S(SPECIES_IGGLYBUFF), S(SPECIES_MORELULL), S(SPECIES_MILCERY), S(SPECIES_FIDOUGH), STRAY_END };
static const struct StrayEntry sStrayPool_Fairy_Rare[]      = { S(SPECIES_AZURILL), S(SPECIES_COMFEY), S(SPECIES_MIMIKYU), STRAY_END };
static const struct StrayEntry sStrayPool_Fairy_Legendary[] = { S(SPECIES_DIANCIE), STRAY_END };

#undef S
#undef S_AB
#undef STRAY_END

// ── Pool lookup table ────────────────────────────────────────────────────

#define POOLS_FOR(typeName) {                                          \
    [HELIX_RARITY_COMMON]    = sStrayPool_##typeName##_Common,         \
    [HELIX_RARITY_UNCOMMON]  = sStrayPool_##typeName##_Uncommon,       \
    [HELIX_RARITY_RARE]      = sStrayPool_##typeName##_Rare,           \
    [HELIX_RARITY_LEGENDARY] = sStrayPool_##typeName##_Legendary,      \
}

static const struct StrayEntry *const sStrayPools[HELIX_NUM_TYPES][HELIX_NUM_RARITIES] = {
    [HELIX_TYPE_NORMAL]   = POOLS_FOR(Normal),
    [HELIX_TYPE_FIGHTING] = POOLS_FOR(Fighting),
    [HELIX_TYPE_FLYING]   = POOLS_FOR(Flying),
    [HELIX_TYPE_POISON]   = POOLS_FOR(Poison),
    [HELIX_TYPE_GROUND]   = POOLS_FOR(Ground),
    [HELIX_TYPE_ROCK]     = POOLS_FOR(Rock),
    [HELIX_TYPE_BUG]      = POOLS_FOR(Bug),
    [HELIX_TYPE_GHOST]    = POOLS_FOR(Ghost),
    [HELIX_TYPE_STEEL]    = POOLS_FOR(Steel),
    [HELIX_TYPE_FIRE]     = POOLS_FOR(Fire),
    [HELIX_TYPE_WATER]    = POOLS_FOR(Water),
    [HELIX_TYPE_GRASS]    = POOLS_FOR(Grass),
    [HELIX_TYPE_ELECTRIC] = POOLS_FOR(Electric),
    [HELIX_TYPE_PSYCHIC]  = POOLS_FOR(Psychic),
    [HELIX_TYPE_ICE]      = POOLS_FOR(Ice),
    [HELIX_TYPE_DRAGON]   = POOLS_FOR(Dragon),
    [HELIX_TYPE_DARK]     = POOLS_FOR(Dark),
    [HELIX_TYPE_FAIRY]    = POOLS_FOR(Fairy),
};

#undef POOLS_FOR

// Graphics ID per rarity.  All default to OBJ_EVENT_GFX_ITEM_BALL until
// custom great/ultra/master ball sprites are added.
// To swap: register new gfx in data/object_events/object_event_graphics_info_pointers.h
// and update these entries.
static const u16 sStrayRarityGfx[HELIX_NUM_RARITIES] = {
    [HELIX_RARITY_COMMON]    = OBJ_EVENT_GFX_ITEM_BALL,  // Poké Ball
    [HELIX_RARITY_UNCOMMON]  = OBJ_EVENT_GFX_GREAT_BALL,  // Great Ball
    [HELIX_RARITY_RARE]      = OBJ_EVENT_GFX_ULTRA_BALL,  // Ultra Ball
    [HELIX_RARITY_LEGENDARY] = OBJ_EVENT_GFX_MASTER_BALL,  // Master Ball
};

// ── Rarity roll ──────────────────────────────────────────────────────────

static u8 RollStrayRarity(void)
{
    u8 roll = Random() % 100;
    if (roll < 1)  return HELIX_RARITY_LEGENDARY; //  1%
    if (roll < 8)  return HELIX_RARITY_RARE;      //  7%
    if (roll < 30) return HELIX_RARITY_UNCOMMON;  // 22%
    return HELIX_RARITY_COMMON;                   // 70%
}

// ── GenerateStray: roll rarity + species, set vars ───────────────────────

void HelixSpecial_GenerateStray(void)
{
    u16 typeId = VarGet(VAR_HELIX_PLAYER_TYPE);
    u8 rarity = RollStrayRarity();
    const struct StrayEntry *pool;
    u16 count;
    u16 pick;

    if (typeId >= HELIX_NUM_TYPES)
        typeId = HELIX_TYPE_NORMAL;

    pool = sStrayPools[typeId][rarity];
    while (pool[0].species == SPECIES_NONE && rarity > HELIX_RARITY_COMMON)
    {
        rarity--;
        pool = sStrayPools[typeId][rarity];
    }

    for (count = 0; pool[count].species != SPECIES_NONE; count++)
        ;

    if (count == 0)
    {
        VarSet(VAR_HELIX_STRAY_SPECIES, SPECIES_RATTATA);
        VarSet(VAR_HELIX_STRAY_ABILITY_OVERRIDE, STRAY_ABILITY_DEFAULT);
        rarity = HELIX_RARITY_COMMON;
    }
    else
    {
        pick = Random() % count;
        VarSet(VAR_HELIX_STRAY_SPECIES, pool[pick].species);
        VarSet(VAR_HELIX_STRAY_ABILITY_OVERRIDE, pool[pick].abilityNum);
    }

    VarSet(VAR_HELIX_STRAY_GENDER, MON_GENDER_RANDOM);

    // Hide all 4 ball objects, then reveal only the one for this rarity
    FlagSet(FLAG_HELIX_STRAY_HIDE_COMMON);
    FlagSet(FLAG_HELIX_STRAY_HIDE_UNCOMMON);
    FlagSet(FLAG_HELIX_STRAY_HIDE_RARE);
    FlagSet(FLAG_HELIX_STRAY_HIDE_LEGENDARY);

    // Local IDs for each rarity ball object
    static const u8 sStrayLocalIds[HELIX_NUM_RARITIES] = {
        [HELIX_RARITY_COMMON]    = LOCALID_HELIX_ISLAND_STRAY_COMMON,
        [HELIX_RARITY_UNCOMMON]  = LOCALID_HELIX_ISLAND_STRAY_UNCOMMON,
        [HELIX_RARITY_RARE]      = LOCALID_HELIX_ISLAND_STRAY_RARE,
        [HELIX_RARITY_LEGENDARY] = LOCALID_HELIX_ISLAND_STRAY_LEGENDARY,
    };

    switch (rarity)
    {
    case HELIX_RARITY_COMMON:    FlagClear(FLAG_HELIX_STRAY_HIDE_COMMON);    break;
    case HELIX_RARITY_UNCOMMON:  FlagClear(FLAG_HELIX_STRAY_HIDE_UNCOMMON);  break;
    case HELIX_RARITY_RARE:      FlagClear(FLAG_HELIX_STRAY_HIDE_RARE);      break;
    case HELIX_RARITY_LEGENDARY: FlagClear(FLAG_HELIX_STRAY_HIDE_LEGENDARY); break;
    }

    // Spawn only the chosen ball on the current map
    TrySpawnObjectEvent(sStrayLocalIds[rarity],
                        gSaveBlock1Ptr->location.mapNum,
                        gSaveBlock1Ptr->location.mapGroup);
}

// ── GenerateDay1Stray: scripted uncommon stray for day 1 ─────────────────
// Always picks uncommon slot 0 for the player's type.
// If both starters share the same gender and the stray species can be the
// opposite gender, forces that gender to prevent breeding deadlocks.

void HelixSpecial_GenerateDay1Stray(void)
{
    u16 typeId = VarGet(VAR_HELIX_PLAYER_TYPE);
    const struct StrayEntry *pool;
    u16 species;
    u8 gender1, gender2;
    u8 genderRatio;
    u8 forcedGender = MON_GENDER_RANDOM;

    if (typeId >= HELIX_NUM_TYPES)
        typeId = HELIX_TYPE_NORMAL;

    pool = sStrayPools[typeId][HELIX_RARITY_UNCOMMON];
    species = pool[0].species;

    if (species == SPECIES_NONE)
    {
        // Fallback — shouldn't happen with real data
        VarSet(VAR_HELIX_STRAY_SPECIES, SPECIES_RATTATA);
        VarSet(VAR_HELIX_STRAY_ABILITY_OVERRIDE, STRAY_ABILITY_DEFAULT);
        VarSet(VAR_HELIX_STRAY_GENDER, MON_GENDER_RANDOM);
        return;
    }

    VarSet(VAR_HELIX_STRAY_SPECIES, species);
    VarSet(VAR_HELIX_STRAY_ABILITY_OVERRIDE, pool[0].abilityNum);

    // Check starter genders (party slots 0 and 1)
    gender1 = GetMonGender(&gPlayerParty[0]);
    gender2 = GetMonGender(&gPlayerParty[1]);
    genderRatio = gSpeciesInfo[species].genderRatio;

    if (gender1 == gender2
        && gender1 != MON_GENDERLESS
        && genderRatio != MON_MALE       // species is not male-only
        && genderRatio != MON_FEMALE     // species is not female-only
        && genderRatio != MON_GENDERLESS) // species is not genderless
    {
        // Both starters are the same gender — force the opposite on the stray
        forcedGender = (gender1 == MON_MALE) ? MON_FEMALE : MON_MALE;
    }

    VarSet(VAR_HELIX_STRAY_GENDER, forcedGender);

    // Show only the uncommon ball
    FlagSet(FLAG_HELIX_STRAY_HIDE_COMMON);
    FlagSet(FLAG_HELIX_STRAY_HIDE_UNCOMMON);
    FlagSet(FLAG_HELIX_STRAY_HIDE_RARE);
    FlagSet(FLAG_HELIX_STRAY_HIDE_LEGENDARY);
    FlagClear(FLAG_HELIX_STRAY_HIDE_UNCOMMON);

    TrySpawnObjectEvent(LOCALID_HELIX_ISLAND_STRAY_UNCOMMON,
                        gSaveBlock1Ptr->location.mapNum,
                        gSaveBlock1Ptr->location.mapGroup);
}

// ── GiveStray: create mon with neutral nature + ability override ─────────
// Called from script instead of givemon.  Sets VAR_RESULT to
// MON_GIVEN_TO_PARTY, MON_GIVEN_TO_PC, or MON_CANT_GIVE.

void HelixSpecial_GiveStray(void)
{
    u16 species = VarGet(VAR_HELIX_STRAY_SPECIES);
    u8 abilityOverride = (u8)VarGet(VAR_HELIX_STRAY_ABILITY_OVERRIDE);
    struct Pokemon mon;
    u8 nature;
    u32 personality;
    u8 result;

    if (species == SPECIES_NONE)
    {
        gSpecialVar_Result = MON_CANT_GIVE;
        return;
    }

    // Randomise forms for species with multiple visual forms
    if (species == SPECIES_TATSUGIRI)
    {
        static const u16 sTatsugiriForms[] = {
            SPECIES_TATSUGIRI_CURLY, SPECIES_TATSUGIRI_DROOPY, SPECIES_TATSUGIRI_STRETCHY
        };
        species = sTatsugiriForms[Random() % ARRAY_COUNT(sTatsugiriForms)];
    }

    // Random neutral nature (same pool as starters)
    nature = sNeutralNatures[Random() % ARRAY_COUNT(sNeutralNatures)];

    // Use gender override if set (day-1 stray may force opposite gender)
    // Safety: drop the override if the species can't be that gender
    {
        u8 gender = (u8)VarGet(VAR_HELIX_STRAY_GENDER);
        u8 ratio = gSpeciesInfo[species].genderRatio;

        if (gender == MON_MALE && (ratio == MON_FEMALE || ratio == MON_GENDERLESS))
            gender = MON_GENDER_RANDOM;
        else if (gender == MON_FEMALE && (ratio == MON_MALE || ratio == MON_GENDERLESS))
            gender = MON_GENDER_RANDOM;
        else if (gender != MON_MALE && gender != MON_FEMALE)
            gender = MON_GENDER_RANDOM;

        personality = GetMonPersonality(species, gender, nature, RANDOM_UNOWN_LETTER);
    }
    CreateMon(&mon, species, 5, personality, OTID_STRUCT_PLAYER_ID);
    HelixSetRandomIVs(&mon);
    GiveMonInitialMoveset(&mon);

    // Apply ability override if specified
    if (abilityOverride != STRAY_ABILITY_DEFAULT && abilityOverride < NUM_ABILITY_SLOTS)
        SetMonData(&mon, MON_DATA_ABILITY_NUM, &abilityOverride);

    result = GiveScriptedMonToPlayer(&mon, PARTY_SIZE);
    gSpecialVar_Result = result;
}

// ── Guide NPC Sprite System ─────────────────────────────────────────
// Maps player type → overworld sprite for the guide NPC.
static const u16 sGuideGraphics[HELIX_NUM_TYPES] = {
    [HELIX_TYPE_NORMAL]   = OBJ_EVENT_GFX_NORMAN,
    [HELIX_TYPE_FIGHTING] = OBJ_EVENT_GFX_BRUNO,
    [HELIX_TYPE_FLYING]   = OBJ_EVENT_GFX_WINONA,
    [HELIX_TYPE_POISON]   = OBJ_EVENT_GFX_KOGA,
    [HELIX_TYPE_GROUND]   = OBJ_EVENT_GFX_GIOVANNI,
    [HELIX_TYPE_ROCK]     = OBJ_EVENT_GFX_BROCK,
    [HELIX_TYPE_BUG]      = OBJ_EVENT_GFX_BUG_CATCHER_FRLG,
    [HELIX_TYPE_GHOST]    = OBJ_EVENT_GFX_AGATHA,
    [HELIX_TYPE_STEEL]    = OBJ_EVENT_GFX_STEVEN,
    [HELIX_TYPE_FIRE]     = OBJ_EVENT_GFX_FLANNERY,
    [HELIX_TYPE_WATER]    = OBJ_EVENT_GFX_MISTY,
    [HELIX_TYPE_GRASS]    = OBJ_EVENT_GFX_ERIKA,
    [HELIX_TYPE_ELECTRIC] = OBJ_EVENT_GFX_LT_SURGE,
    [HELIX_TYPE_PSYCHIC]  = OBJ_EVENT_GFX_SABRINA,
    [HELIX_TYPE_ICE]      = OBJ_EVENT_GFX_LORELEI,
    [HELIX_TYPE_DRAGON]   = OBJ_EVENT_GFX_LANCE,
    [HELIX_TYPE_DARK]     = OBJ_EVENT_GFX_SIDNEY,
    [HELIX_TYPE_FAIRY]    = OBJ_EVENT_GFX_WALLY,
};

// Apply the type-specific guide sprite to a guide object on the current map.
void HelixSetGuideGraphicsForLocalId(u8 localId)
{
    u16 typeId = VarGet(VAR_HELIX_PLAYER_TYPE);
    u16 gfxId;
    u8 objEventId;
    u32 i;

    if (typeId >= HELIX_NUM_TYPES)
        typeId = HELIX_TYPE_NORMAL;

    gfxId = sGuideGraphics[typeId];

    // Update the template so the sprite is created with the correct graphics
    for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
    {
        if (gSaveBlock1Ptr->objectEventTemplates[i].localId == localId)
        {
            gSaveBlock1Ptr->objectEventTemplates[i].graphicsId = gfxId;
            break;
        }
    }

    // Also update the live object event if it already exists on screen
    objEventId = GetObjectEventIdByLocalIdAndMap(
        localId,
        gSaveBlock1Ptr->location.mapNum,
        gSaveBlock1Ptr->location.mapGroup);

    if (objEventId < OBJECT_EVENTS_COUNT)
        ObjectEventSetGraphicsId(&gObjectEvents[objEventId], gfxId);
}

void HelixSpecial_SetGuideGraphics(void)
{
    HelixSetGuideGraphicsForLocalId(LOCALID_HELIX_ISLAND_GUIDE);
}

