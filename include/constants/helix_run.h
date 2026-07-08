#ifndef GUARD_CONSTANTS_HELIX_RUN_H
#define GUARD_CONSTANTS_HELIX_RUN_H

// Helix expedition (run) constants shared between C code and event scripts.
// Pure #defines only — this header is included from data/event_scripts.s.

// VAR_HELIX_RUN_ACTIVE states
#define HELIX_RUN_INACTIVE        0
#define HELIX_RUN_IN_PROGRESS     1
#define HELIX_RUN_BLACKOUT_RETURN 2 // blacked out; island message pending

// Room types (VAR_HELIX_RUN_ROOM_TYPE)
#define HELIX_RUN_ROOM_NONE          0
#define HELIX_RUN_ROOM_FIXED_TRAINER 1
#define HELIX_RUN_ROOM_WILD          2
#define HELIX_RUN_ROOM_TRAINER       3
#define HELIX_RUN_ROOM_SHOP          4
#define HELIX_RUN_ROOM_BOSS          5

// Route/door directions (VAR_HELIX_RUN_EXIT_DIR)
#define HELIX_RUN_DIR_UP    0
#define HELIX_RUN_DIR_RIGHT 1
#define HELIX_RUN_DIR_DOWN  2
#define HELIX_RUN_DIR_LEFT  3

// HelixSpecial_CheckRunEntry results (VAR_RESULT)
#define HELIX_RUN_ENTRY_OK            0
#define HELIX_RUN_ENTRY_NO_PARTY      1 // 0 usable non-egg mons
#define HELIX_RUN_ENTRY_TOO_MANY      2 // more than HELIX_RUN_PARTY_MAX usable mons
#define HELIX_RUN_ENTRY_RETIRED       3 // a retired mon is in the party
#define HELIX_RUN_ENTRY_ALREADY_TODAY 4 // a run already happened this island day

// Trainer room class variants (VAR_HELIX_RUN_TRAINER_VARIANT)
#define HELIX_RUN_TRAINER_HIKER         0
#define HELIX_RUN_TRAINER_CAMPER        1
#define HELIX_RUN_TRAINER_NERD          2
#define HELIX_RUN_TRAINER_OLD_MAN       3
#define HELIX_RUN_TRAINER_VARIANT_COUNT 4

// Stat ids for the IV reward menu
#define HELIX_STAT_HP    0
#define HELIX_STAT_ATK   1
#define HELIX_STAT_DEF   2
#define HELIX_STAT_SPATK 3
#define HELIX_STAT_SPDEF 4
#define HELIX_STAT_SPEED 5
#define HELIX_STAT_COUNT 6

#endif // GUARD_CONSTANTS_HELIX_RUN_H
