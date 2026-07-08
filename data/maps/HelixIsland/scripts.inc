HelixIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, HelixIsland_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, HelixIsland_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, HelixIsland_OnFrame
	.byte 0

@ ══════════════════════════════════════════════════════════════════════
@ MAP_SCRIPT_ON_LOAD (tile changes — fires before rendering)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_OnLoad:
	compare VAR_HELIX_DAY_COUNT, 2
	goto_if_ge HelixIsland_OnLoad_Day2
	@ Day 1: ensure finished boat area is CalmWater (impassable)
	setmetatile 23, 22, 0x170, 1
	setmetatile 24, 22, 0x170, 1
	setmetatile 25, 22, 0x170, 1
	setmetatile 23, 23, 0x170, 1
	setmetatile 24, 23, 0x170, 1
	setmetatile 25, 23, 0x170, 1
	setmetatile 23, 24, 0x170, 1
	setmetatile 24, 24, 0x170, 1
	setmetatile 25, 24, 0x170, 1
	end
HelixIsland_OnLoad_Day2:
	@ Clear under-construction boat (passable grass)
	setmetatile 47, 22, METATILE_General_Grass, 0
	setmetatile 48, 22, METATILE_General_Grass, 0
	setmetatile 49, 22, METATILE_General_Grass, 0
	setmetatile 47, 23, METATILE_General_Grass, 0
	setmetatile 48, 23, METATILE_General_Grass, 0
	setmetatile 49, 23, METATILE_General_Grass, 0
	@ Place finished boat (impassable)
	setmetatile 24, 22, 0x339, 1
	setmetatile 25, 22, 0x33A, 1
	setmetatile 23, 23, 0x340, 1
	setmetatile 24, 23, 0x341, 1
	setmetatile 25, 23, 0x342, 1
	setmetatile 23, 24, 0x348, 1
	setmetatile 24, 24, 0x349, 1
	setmetatile 25, 24, 0x34A, 1
	end

@ ══════════════════════════════════════════════════════════════════════
@ MAP_SCRIPT_ON_TRANSITION
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_OnTransition:
	setflag FLAG_VISITED_LITTLEROOT_TOWN
	special HelixSpecial_SetGuideGraphics
	@ First visit: spawn a scripted uncommon stray (gender-aware)
	compare VAR_HELIX_STRAY_SPECIES, 0
	goto_if_ne HelixIsland_OnTransition_GuideCheck
	special HelixSpecial_GenerateDay1Stray
HelixIsland_OnTransition_GuideCheck:
	@ Day 2+: reposition guide next to boat
	compare VAR_HELIX_DAY_COUNT, 2
	goto_if_lt HelixIsland_OnTransition_End
	setobjectxyperm LOCALID_HELIX_ISLAND_GUIDE, 28, 20
	setobjectxy LOCALID_HELIX_ISLAND_GUIDE, 28, 20
	setobjectmovementtype LOCALID_HELIX_ISLAND_GUIDE, MOVEMENT_TYPE_FACE_DOWN
HelixIsland_OnTransition_End:
	end

@ ══════════════════════════════════════════════════════════════════════
@ MAP_SCRIPT_ON_FRAME_TABLE
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_OnFrame:
	map_script_2 VAR_HELIX_INTRO_STATE, 2, HelixIsland_EventScript_Tutorial
	map_script_2 VAR_HELIX_RUN_ACTIVE, HELIX_RUN_BLACKOUT_RETURN, HelixIsland_EventScript_BlackoutReturn
	.2byte 0

@ Run blackout: the player was carried back to the island (see DoWhiteOut)
HelixIsland_EventScript_BlackoutReturn::
	lockall
	setvar VAR_HELIX_RUN_ACTIVE, HELIX_RUN_INACTIVE
	msgbox HelixIsland_Text_BlackoutReturn, MSGBOX_DEFAULT
	releaseall
	end

@ ══════════════════════════════════════════════════════════════════════
@ DAY-1 TUTORIAL CUTSCENE
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_EventScript_Tutorial::
	lockall
	@ Prevent re-trigger — advance state but boundaries not active yet
	setvar VAR_HELIX_INTRO_STATE, 3
	@ --- Segment 1: Guide walks down stairs toward player ---
	applymovement LOCALID_HELIX_ISLAND_GUIDE, Movement_GuideApproach
	waitmovement 0
	applymovement LOCALID_HELIX_ISLAND_GUIDE, Movement_EmotionExclamation
	waitmovement 0
	applymovement OBJ_EVENT_ID_PLAYER, Movement_FaceRight
	waitmovement 0
	call HelixIsland_GuideMsg_Greeting
	@ Guide steps to player's right
	applymovement LOCALID_HELIX_ISLAND_GUIDE, Movement_GuideToPlayerSide
	waitmovement 0
	call HelixIsland_GuideMsg_FollowMe
	closemessage
	@ --- Segment 2: Both walk to ball area ---
	applymovement LOCALID_HELIX_ISLAND_GUIDE, Movement_GuideToBall
	applymovement OBJ_EVENT_ID_PLAYER, Movement_PlayerToBall
	waitmovement 0
	call HelixIsland_GuideMsg_BallHint
	@ Release player for ball pickup (boundaries now active at state 3)
	releaseall
	end

@ ── Post-Ball Tutorial (chained from StrayAccept) ────────────────────
HelixIsland_EventScript_TutorialPostBall::
	lockall
	@ Walk player to (54, 24) if not already there
	getplayerxy VAR_0x8004, VAR_0x8005
HelixIsland_PostBall_FixX:
	compare VAR_0x8004, 54
	goto_if_eq HelixIsland_PostBall_FixY
	goto_if_gt HelixIsland_PostBall_WalkLeft
	applymovement OBJ_EVENT_ID_PLAYER, Movement_WalkRight1
	waitmovement 0
	addvar VAR_0x8004, 1
	goto HelixIsland_PostBall_FixX
HelixIsland_PostBall_WalkLeft:
	applymovement OBJ_EVENT_ID_PLAYER, Movement_WalkLeft1
	waitmovement 0
	subvar VAR_0x8004, 1
	goto HelixIsland_PostBall_FixX
HelixIsland_PostBall_FixY:
	compare VAR_0x8005, 24
	goto_if_eq HelixIsland_PostBall_Ready
	goto_if_gt HelixIsland_PostBall_WalkUp
	applymovement OBJ_EVENT_ID_PLAYER, Movement_WalkDown1
	waitmovement 0
	addvar VAR_0x8005, 1
	goto HelixIsland_PostBall_FixY
HelixIsland_PostBall_WalkUp:
	applymovement OBJ_EVENT_ID_PLAYER, Movement_WalkUp1
	waitmovement 0
	subvar VAR_0x8005, 1
	goto HelixIsland_PostBall_FixY
HelixIsland_PostBall_Ready:
	applymovement OBJ_EVENT_ID_PLAYER, Movement_FaceLeft
	waitmovement 0
	call HelixIsland_GuideMsg_GoodCatch
	closemessage
	@ --- Segment 3: Ball area to boat ---
	applymovement LOCALID_HELIX_ISLAND_GUIDE, Movement_GuideToBoat
	applymovement OBJ_EVENT_ID_PLAYER, Movement_PlayerToBoat
	waitmovement 0
	call HelixIsland_GuideMsg_BoatIntro
	closemessage
	@ --- Segment 4: Boat to shops ---
	applymovement LOCALID_HELIX_ISLAND_GUIDE, Movement_GuideToShops
	applymovement OBJ_EVENT_ID_PLAYER, Movement_PlayerToShops
	waitmovement 0
	call HelixIsland_GuideMsg_ShopsIntro
	closemessage
	@ --- Segment 5: Shops to PC ---
	applymovement LOCALID_HELIX_ISLAND_GUIDE, Movement_GuideToPC
	applymovement OBJ_EVENT_ID_PLAYER, Movement_PlayerToPC
	waitmovement 0
	call HelixIsland_GuideMsg_PCIntro
	@ --- Wrap up ---
	call HelixIsland_GuideMsg_Rest
	@ Day-1 comfort boost (90% breed chance)
	setvar VAR_HELIX_COMFORT, 240
	@ Mark tutorial complete
	setvar VAR_HELIX_INTRO_STATE, 4
	setflag FLAG_HELIX_TUTORIAL_DONE
	releaseall
	end


@ ══════════════════════════════════════════════════════════════════════
@ BOUNDARY ENFORCEMENT (ball pickup zone)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_EventScript_BoundaryLeft::
	lockall
	applymovement LOCALID_HELIX_ISLAND_GUIDE, Movement_EmotionExclamation
	waitmovement 0
	call HelixIsland_GuideMsg_ComeBack
	applymovement OBJ_EVENT_ID_PLAYER, Movement_BoundaryPushRight
	waitmovement 0
	releaseall
	end

HelixIsland_EventScript_BoundaryTop::
	lockall
	applymovement LOCALID_HELIX_ISLAND_GUIDE, Movement_EmotionExclamation
	waitmovement 0
	call HelixIsland_GuideMsg_ComeBack
	applymovement OBJ_EVENT_ID_PLAYER, Movement_BoundaryPushDown
	waitmovement 0
	releaseall
	end

@ ══════════════════════════════════════════════════════════════════════
@ GUIDE NPC INTERACTION (day 2+)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_EventScript_GuideInteract::
	lock
	faceplayer
	@ During ball pickup: remind player
	compare VAR_HELIX_INTRO_STATE, 3
	goto_if_eq HelixIsland_EventScript_GuideRemindBall
	@ Post-tutorial: check if boat is ready (day >= 2)
	goto_if_set FLAG_HELIX_TUTORIAL_DONE, HelixIsland_EventScript_GuidePostTutorial
	@ Fallback (shouldn't reach here)
	call HelixIsland_GuideMsg_Idle
	release
	end

HelixIsland_EventScript_GuideRemindBall::
	call HelixIsland_GuideMsg_RemindBall
	release
	end

HelixIsland_EventScript_GuidePostTutorial::
	compare VAR_HELIX_DAY_COUNT, 2
	goto_if_ge HelixIsland_EventScript_GuideBoatPrompt
	@ Day 1 post-tutorial: just idle
	call HelixIsland_GuideMsg_RestReminder
	release
	end

HelixIsland_EventScript_GuideBoatPrompt::
	call HelixIsland_GuideMsg_ReadyPrompt
	goto_if_eq VAR_RESULT, YES, HelixIsland_EventScript_GuideBoatYes
	call HelixIsland_GuideMsg_BoatNo
	release
	end

HelixIsland_EventScript_GuideBoatYes::
	call HelixIsland_GuideMsg_BoatYes
	closemessage
	warp MAP_HELIX_NEXUS, 5, 7
	waitstate
	release
	end

@ ══════════════════════════════════════════════════════════════════════
@ BED / SLEEP
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_EventScript_BedNPC::
	lock
	faceplayer
	msgbox HelixIsland_Text_Sleep, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, HelixIsland_EventScript_DoSleep
	msgbox HelixIsland_Text_NotTired, MSGBOX_DEFAULT
	release
	end

HelixIsland_EventScript_DoSleep::
	@ Count population first — needed for food check
	special HelixSpecial_CountPopulation
	@ Check if we have enough food
	copyvar VAR_0x8005, VAR_HELIX_FOOD
	compare VAR_0x8005, VAR_HELIX_ISLAND_POP
	goto_if_lt HelixIsland_EventScript_NotEnoughFood
	@ Sleep sequence
	msgbox HelixIsland_Text_Sleeping, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	delay 60
	@ Breeding system: hatch existing eggs first, then breed new ones
	special HelixSpecial_HatchEggs
	special HelixSpecial_BreedBoxes
	addvar VAR_HELIX_DAY_COUNT, 1
	@ Reset comfort boost after day 1
	compare VAR_HELIX_DAY_COUNT, 2
	goto_if_ne HelixIsland_EventScript_SkipComfortReset
	setvar VAR_HELIX_COMFORT, 50
HelixIsland_EventScript_SkipComfortReset:
	@ Food consumption: subtract population from food
	subvar VAR_HELIX_FOOD, VAR_HELIX_ISLAND_POP
	special HealPlayerParty
	@ Recount after breeding (new eggs = new mouths)
	special HelixSpecial_CountPopulation
	@ Refresh stray for the new day
	clearflag FLAG_HELIX_STRAY_MSG_SHOWN
	special HelixSpecial_GenerateStray
	@ Day 2+ tile/guide changes while screen is still black
	compare VAR_HELIX_DAY_COUNT, 2
	goto_if_lt HelixIsland_EventScript_PostSleepFadeIn
	@ Clear under-construction boat
	setmetatile 47, 22, METATILE_General_Grass, 0
	setmetatile 48, 22, METATILE_General_Grass, 0
	setmetatile 49, 22, METATILE_General_Grass, 0
	setmetatile 47, 23, METATILE_General_Grass, 0
	setmetatile 48, 23, METATILE_General_Grass, 0
	setmetatile 49, 23, METATILE_General_Grass, 0
	@ Place finished boat
	setmetatile 24, 22, 0x339, 1
	setmetatile 25, 22, 0x33A, 1
	setmetatile 23, 23, 0x340, 1
	setmetatile 24, 23, 0x341, 1
	setmetatile 25, 23, 0x342, 1
	setmetatile 23, 24, 0x348, 1
	setmetatile 24, 24, 0x349, 1
	setmetatile 25, 24, 0x34A, 1
	@ Reposition guide
	setobjectxyperm LOCALID_HELIX_ISLAND_GUIDE, 28, 20
	setobjectxy LOCALID_HELIX_ISLAND_GUIDE, 28, 20
	setobjectmovementtype LOCALID_HELIX_ISLAND_GUIDE, MOVEMENT_TYPE_FACE_DOWN
	special DrawWholeMapView
HelixIsland_EventScript_PostSleepFadeIn:
	fadescreenswapbuffers FADE_FROM_BLACK
	buffernumberstring STR_VAR_1, VAR_HELIX_FOOD
	buffernumberstring STR_VAR_2, VAR_HELIX_DAY_COUNT
	msgbox HelixIsland_Text_NewDay, MSGBOX_DEFAULT
	release
	end

HelixIsland_EventScript_NotEnoughFood::
	buffernumberstring STR_VAR_1, VAR_HELIX_ISLAND_POP
	buffernumberstring STR_VAR_2, VAR_HELIX_FOOD
	msgbox HelixIsland_Text_NotEnoughFood, MSGBOX_DEFAULT
	release
	end

@ ══════════════════════════════════════════════════════════════════════
@ PC TERMINAL
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_EventScript_PC::
	lock
	faceplayer
	setvar VAR_0x8004, PC_LOCATION_OTHER
	playse SE_PC_ON
	msgbox HelixIsland_Text_PCBoot, MSGBOX_DEFAULT
	msgbox gText_StorageSystemOpened, MSGBOX_DEFAULT
	special ShowPokemonStorageSystemPC
	waitstate
	playse SE_PC_OFF
	release
	end

@ ══════════════════════════════════════════════════════════════════════
@ STRAY POKEBALL INTERACTION
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_EventScript_StrayBall::
	lock
	faceplayer
	@ First interaction this day: show "washed up" message
	goto_if_set FLAG_HELIX_STRAY_MSG_SHOWN, HelixIsland_EventScript_StrayAsk
	setflag FLAG_HELIX_STRAY_MSG_SHOWN
	msgbox HelixIsland_Text_StrayWashedUp, MSGBOX_DEFAULT
HelixIsland_EventScript_StrayAsk:
	@ Show the Pokémon sprite and ask
	bufferspeciesname STR_VAR_1, VAR_HELIX_STRAY_SPECIES
	showmonpic VAR_HELIX_STRAY_SPECIES, 10, 3
	msgbox HelixIsland_Text_StrayAdd, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, HelixIsland_EventScript_StrayAccept
	@ Player said No — hide pic, ball stays
	hidemonpic
	release
	end

HelixIsland_EventScript_StrayAccept:
	hidemonpic
	special HelixSpecial_GiveStray
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, HelixIsland_EventScript_StrayGotParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, HelixIsland_EventScript_StrayGotPC
	@ Party and PC both full
	msgbox HelixIsland_Text_StrayNoRoom, MSGBOX_DEFAULT
	release
	end

HelixIsland_EventScript_StrayGotParty:
	bufferspeciesname STR_VAR_1, VAR_HELIX_STRAY_SPECIES
	playse SE_SUCCESS
	msgbox HelixIsland_Text_StrayJoined, MSGBOX_DEFAULT
	goto HelixIsland_EventScript_StrayHideAll

HelixIsland_EventScript_StrayGotPC:
	bufferspeciesname STR_VAR_1, VAR_HELIX_STRAY_SPECIES
	playse SE_SUCCESS
	msgbox HelixIsland_Text_StrayJoinedPC, MSGBOX_DEFAULT
	goto HelixIsland_EventScript_StrayHideAll

@ Hide all 4 ball objects after collection
HelixIsland_EventScript_StrayHideAll:
	setflag FLAG_HELIX_STRAY_HIDE_COMMON
	setflag FLAG_HELIX_STRAY_HIDE_UNCOMMON
	setflag FLAG_HELIX_STRAY_HIDE_RARE
	setflag FLAG_HELIX_STRAY_HIDE_LEGENDARY
	removeobject LOCALID_HELIX_ISLAND_STRAY_COMMON
	removeobject LOCALID_HELIX_ISLAND_STRAY_UNCOMMON
	removeobject LOCALID_HELIX_ISLAND_STRAY_RARE
	removeobject LOCALID_HELIX_ISLAND_STRAY_LEGENDARY
	@ If tutorial active, chain to post-ball segment
	compare VAR_HELIX_INTRO_STATE, 3
	goto_if_eq HelixIsland_EventScript_StrayHideTutorial
	release
	end

HelixIsland_EventScript_StrayHideTutorial:
	release
	goto HelixIsland_EventScript_TutorialPostBall

@ ══════════════════════════════════════════════════════════════════════
@ DEV KIT NPC (testing only — remove before release)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_EventScript_DevKit::
	lock
	faceplayer
	msgbox HelixIsland_Text_DevKitOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HelixIsland_EventScript_DevKitDecline
	@ All 8 badges so the level 100 mon obeys
	setflag FLAG_BADGE01_GET
	setflag FLAG_BADGE02_GET
	setflag FLAG_BADGE03_GET
	setflag FLAG_BADGE04_GET
	setflag FLAG_BADGE05_GET
	setflag FLAG_BADGE06_GET
	setflag FLAG_BADGE07_GET
	setflag FLAG_BADGE08_GET
	givemon SPECIES_NUZLEAF, 100, move1=MOVE_FALSE_SWIPE, move2=MOVE_LEAF_BLADE
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, HelixIsland_EventScript_DevKitParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, HelixIsland_EventScript_DevKitPC
	@ Party and PC both full
	msgbox HelixIsland_Text_DevKitNoRoom, MSGBOX_DEFAULT
	release
	end

HelixIsland_EventScript_DevKitParty:
	playse SE_SUCCESS
	msgbox HelixIsland_Text_DevKitParty, MSGBOX_DEFAULT
	release
	end

HelixIsland_EventScript_DevKitPC:
	playse SE_SUCCESS
	msgbox HelixIsland_Text_DevKitPC, MSGBOX_DEFAULT
	release
	end

HelixIsland_EventScript_DevKitDecline:
	msgbox HelixIsland_Text_DevKitDecline, MSGBOX_DEFAULT
	release
	end

@ ══════════════════════════════════════════════════════════════════════
@ MOVEMENT DATA
@ ══════════════════════════════════════════════════════════════════════

@ Segment 1: Guide walks from (42,17) to (42,23), faces left
Movement_GuideApproach:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	face_left
	step_end

Movement_EmotionExclamation:
	emote_exclamation_mark
	step_end

Movement_FaceRight:
	face_right
	step_end

Movement_FaceLeft:
	face_left
	step_end

Movement_WalkRight1:
	walk_right
	step_end

Movement_WalkLeft1:
	walk_left
	step_end

Movement_WalkUp1:
	walk_up
	step_end

Movement_WalkDown1:
	walk_down
	step_end

@ Guide walks from (42,23) to (40,23)
Movement_GuideToPlayerSide:
	walk_left
	walk_left
	face_left
	step_end

@ Segment 2: Guide (40,23)→(45,23)→(45,25)→(53,25)→(53,23), face right
@ 5R + 2D + 8R + 2U = 17 steps. NPC leads, player follows 2 tiles behind.
Movement_GuideToBall:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	face_right
	step_end

@ Segment 2: Player (38,23)→(45,23)→(45,25)→(54,25)→(54,24), face right
@ 2 delays so NPC leads by ~2 tiles, then 7R + 2D + 9R + 1U = 19 steps
Movement_PlayerToBall:
	delay_16
	delay_16
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	face_right
	step_end

@ Segment 3: Guide (53,23)→(53,24)→(47,24), face up
@ 1D + 6L = 7 steps. Pad to 7 for player sync (1D + 5L + face = 6 steps + face, pad player).
Movement_GuideToBoat:
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	face_up
	step_end

@ Player (54,24)→(49,24), face up
@ 5L = 5 steps. Pad with 2 delays to sync with guide (7).
Movement_PlayerToBoat:
	delay_16
	delay_16
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	face_up
	step_end

@ Segment 4: Guide (47,24)→(42,24)→(42,7), face down
@ 5L + 17U = 22 steps
Movement_GuideToShops:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	face_down
	step_end

@ Player (49,24)→(42,24)→(42,9), face up
@ 7L + 15U = 22 steps
Movement_PlayerToShops:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	face_up
	step_end

@ Segment 5: Guide (42,7)→(51,7), face down
@ 9R = 9 steps
Movement_GuideToPC:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	face_down
	step_end

@ Player (42,9)→(50,9), face right
@ 8R = 8 steps. Pad with 1 delay to sync with guide (9).
Movement_PlayerToPC:
	delay_16
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	face_right
	step_end

@ Boundary pushback movements
Movement_BoundaryPushRight:
	face_right
	walk_right
	step_end

Movement_BoundaryPushDown:
	face_down
	walk_down
	step_end

@ ══════════════════════════════════════════════════════════════════════
@ TEXT — Tutorial (Norman style: direct, fatherly, practical)
@ ══════════════════════════════════════════════════════════════════════

HelixIsland_Text_GuideGreeting:
	.string "Hey kiddo.\p"
	.string "Glad you made it in one piece.\p"
	.string "This island is our home now.\n"
	.string "Let's get to work.$"

HelixIsland_Text_GuideFollowMe:
	.string "Follow me. There's something\n"
	.string "on the shore I want you to see.$"

HelixIsland_Text_GuideBallHint:
	.string "See that ball?\p"
	.string "A POKéMON washed up. It happens\n"
	.string "all the time here.\p"
	.string "Go ahead, pick it up.$"

HelixIsland_Text_GuideGoodCatch:
	.string "Good.\p"
	.string "You'll see a new one wash up\n"
	.string "every morning.\p"
	.string "Now, over here.$"

HelixIsland_Text_GuideBoatIntro:
	.string "See that boat? It still needs work.\p"
	.string "Once it's ready, I'll take us\n"
	.string "out to the Nexus.\p"
	.string "That's where things get serious.\n"
	.string "We'll save that for tomorrow, though.$"

HelixIsland_Text_GuideShopsIntro:
	.string "Up ahead is where I've set up some\n"
	.string "storefronts.\p"
	.string "Some merchants were supposed to get\n"
	.string "here a while ago, but I haven't\p"
	.string "heard from them in some time.$"

HelixIsland_Text_GuidePCIntro:
	.string "This here is our little 'magic sapling'.\p"
	.string "I can't seem to figure out how to use it,\n"
	.string "that's why Deoxys sent you here.\p"
	.string "You can use it to commune with the\n"
	.string "'real island', whatever that means.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "If you're lucky, you might wake up\n"
	.string "to an egg or two.\p"
	.string "Even luckier, and those eggs might\n"
	.string "hatch with some interesting traits.$"

HelixIsland_Text_GuideRest:
	.string "Those are the basics.\p"
	.string "Get some rest tonight,\n"
	.string "tomorrow we start for real.\p"
	.string "I'll be by the boat in the\n"
	.string "morning.$"

HelixIsland_Text_GuideComeBack:
	.string "Hey. The ball's over there.\n"
	.string "Don't wander off.$"

HelixIsland_Text_GuideRemindBall:
	.string "Go on. Pick up the ball.\n"
	.string "I'll wait.$"

HelixIsland_Text_GuideIdle:
	.string "Take your time. Look around.$"

HelixIsland_Text_GuideRestReminder:
	.string "You should get some sleep.\n"
	.string "Big day tomorrow.$"

@ ── Day-2+ Boat Dialogue ──────────────────────────────────────────

HelixIsland_Text_GuideReadyPrompt:
	.string "Ready to head out?$"

HelixIsland_Text_GuideBoatYes:
	.string "Alright, let's go!$"

HelixIsland_Text_GuideBoatNo:
	.string "Take your time.\n"
	.string "I'll be here when you're ready.$"

@ ══════════════════════════════════════════════════════════════════════
@ TEXT — Island Systems
@ ══════════════════════════════════════════════════════════════════════

HelixIsland_Text_Sleep:
	.string "Rest for the night?$"

HelixIsland_Text_NotTired:
	.string "You decided to stay up a bit longer.$"

HelixIsland_Text_Sleeping:
	.string "You drifted off to sleep...$"

HelixIsland_Text_NewDay:
	.string "A new day dawns on the island.\p"
	.string "Food remaining: {STR_VAR_1}.\p"
	.string "Day: {STR_VAR_2}.$"

HelixIsland_Text_PCBoot:
	.string "You feel a connection to a distant land.$"

HelixIsland_Text_NotEnoughFood:
	.string "You don't have enough food!\p"
	.string "Your population needs {STR_VAR_1}\n"
	.string "food, but you only have {STR_VAR_2}.$"

@ ── Stray Text ────────────────────────────────────────────────────

HelixIsland_Text_StrayWashedUp:
	.string "A POKéMON has washed up on shore!$"

HelixIsland_Text_StrayAdd:
	.string "Add {STR_VAR_1} to your party?$"

HelixIsland_Text_StrayJoined:
	.string "{STR_VAR_1} joined your team!$"

HelixIsland_Text_StrayJoinedPC:
	.string "{STR_VAR_1} was sent to your PC!$"

HelixIsland_Text_StrayNoRoom:
	.string "You have no room for more POKéMON!$"

HelixIsland_Text_BlackoutReturn:
	.string "Your expedition was lost in\n"
	.string "the dark...\p"
	.string "You wake beside the island bed,\n"
	.string "alone.\p"
	.string "The island's POKéMON are still\n"
	.string "here. Rebuild, and try again.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE DIALOGUE ROUTING
@ Each beat switches on VAR_HELIX_PLAYER_TYPE and calls the correct
@ type-specific handler. Normal (0) uses existing Norman text.
@ ══════════════════════════════════════════════════════════════════════

HelixIsland_GuideMsg_Greeting:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_Greeting_Normal
	case 1, HelixIsland_GuideMsg_Greeting_Fighting
	case 2, HelixIsland_GuideMsg_Greeting_Flying
	case 3, HelixIsland_GuideMsg_Greeting_Poison
	case 4, HelixIsland_GuideMsg_Greeting_Ground
	case 5, HelixIsland_GuideMsg_Greeting_Rock
	case 6, HelixIsland_GuideMsg_Greeting_Bug
	case 7, HelixIsland_GuideMsg_Greeting_Ghost
	case 8, HelixIsland_GuideMsg_Greeting_Steel
	case 9, HelixIsland_GuideMsg_Greeting_Fire
	case 10, HelixIsland_GuideMsg_Greeting_Water
	case 11, HelixIsland_GuideMsg_Greeting_Grass
	case 12, HelixIsland_GuideMsg_Greeting_Electric
	case 13, HelixIsland_GuideMsg_Greeting_Psychic
	case 14, HelixIsland_GuideMsg_Greeting_Ice
	case 15, HelixIsland_GuideMsg_Greeting_Dragon
	case 16, HelixIsland_GuideMsg_Greeting_Dark
	case 17, HelixIsland_GuideMsg_Greeting_Fairy
	return

HelixIsland_GuideMsg_FollowMe:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_FollowMe_Normal
	case 1, HelixIsland_GuideMsg_FollowMe_Fighting
	case 2, HelixIsland_GuideMsg_FollowMe_Flying
	case 3, HelixIsland_GuideMsg_FollowMe_Poison
	case 4, HelixIsland_GuideMsg_FollowMe_Ground
	case 5, HelixIsland_GuideMsg_FollowMe_Rock
	case 6, HelixIsland_GuideMsg_FollowMe_Bug
	case 7, HelixIsland_GuideMsg_FollowMe_Ghost
	case 8, HelixIsland_GuideMsg_FollowMe_Steel
	case 9, HelixIsland_GuideMsg_FollowMe_Fire
	case 10, HelixIsland_GuideMsg_FollowMe_Water
	case 11, HelixIsland_GuideMsg_FollowMe_Grass
	case 12, HelixIsland_GuideMsg_FollowMe_Electric
	case 13, HelixIsland_GuideMsg_FollowMe_Psychic
	case 14, HelixIsland_GuideMsg_FollowMe_Ice
	case 15, HelixIsland_GuideMsg_FollowMe_Dragon
	case 16, HelixIsland_GuideMsg_FollowMe_Dark
	case 17, HelixIsland_GuideMsg_FollowMe_Fairy
	return

HelixIsland_GuideMsg_BallHint:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_BallHint_Normal
	case 1, HelixIsland_GuideMsg_BallHint_Fighting
	case 2, HelixIsland_GuideMsg_BallHint_Flying
	case 3, HelixIsland_GuideMsg_BallHint_Poison
	case 4, HelixIsland_GuideMsg_BallHint_Ground
	case 5, HelixIsland_GuideMsg_BallHint_Rock
	case 6, HelixIsland_GuideMsg_BallHint_Bug
	case 7, HelixIsland_GuideMsg_BallHint_Ghost
	case 8, HelixIsland_GuideMsg_BallHint_Steel
	case 9, HelixIsland_GuideMsg_BallHint_Fire
	case 10, HelixIsland_GuideMsg_BallHint_Water
	case 11, HelixIsland_GuideMsg_BallHint_Grass
	case 12, HelixIsland_GuideMsg_BallHint_Electric
	case 13, HelixIsland_GuideMsg_BallHint_Psychic
	case 14, HelixIsland_GuideMsg_BallHint_Ice
	case 15, HelixIsland_GuideMsg_BallHint_Dragon
	case 16, HelixIsland_GuideMsg_BallHint_Dark
	case 17, HelixIsland_GuideMsg_BallHint_Fairy
	return

HelixIsland_GuideMsg_GoodCatch:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_GoodCatch_Normal
	case 1, HelixIsland_GuideMsg_GoodCatch_Fighting
	case 2, HelixIsland_GuideMsg_GoodCatch_Flying
	case 3, HelixIsland_GuideMsg_GoodCatch_Poison
	case 4, HelixIsland_GuideMsg_GoodCatch_Ground
	case 5, HelixIsland_GuideMsg_GoodCatch_Rock
	case 6, HelixIsland_GuideMsg_GoodCatch_Bug
	case 7, HelixIsland_GuideMsg_GoodCatch_Ghost
	case 8, HelixIsland_GuideMsg_GoodCatch_Steel
	case 9, HelixIsland_GuideMsg_GoodCatch_Fire
	case 10, HelixIsland_GuideMsg_GoodCatch_Water
	case 11, HelixIsland_GuideMsg_GoodCatch_Grass
	case 12, HelixIsland_GuideMsg_GoodCatch_Electric
	case 13, HelixIsland_GuideMsg_GoodCatch_Psychic
	case 14, HelixIsland_GuideMsg_GoodCatch_Ice
	case 15, HelixIsland_GuideMsg_GoodCatch_Dragon
	case 16, HelixIsland_GuideMsg_GoodCatch_Dark
	case 17, HelixIsland_GuideMsg_GoodCatch_Fairy
	return

HelixIsland_GuideMsg_BoatIntro:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_BoatIntro_Normal
	case 1, HelixIsland_GuideMsg_BoatIntro_Fighting
	case 2, HelixIsland_GuideMsg_BoatIntro_Flying
	case 3, HelixIsland_GuideMsg_BoatIntro_Poison
	case 4, HelixIsland_GuideMsg_BoatIntro_Ground
	case 5, HelixIsland_GuideMsg_BoatIntro_Rock
	case 6, HelixIsland_GuideMsg_BoatIntro_Bug
	case 7, HelixIsland_GuideMsg_BoatIntro_Ghost
	case 8, HelixIsland_GuideMsg_BoatIntro_Steel
	case 9, HelixIsland_GuideMsg_BoatIntro_Fire
	case 10, HelixIsland_GuideMsg_BoatIntro_Water
	case 11, HelixIsland_GuideMsg_BoatIntro_Grass
	case 12, HelixIsland_GuideMsg_BoatIntro_Electric
	case 13, HelixIsland_GuideMsg_BoatIntro_Psychic
	case 14, HelixIsland_GuideMsg_BoatIntro_Ice
	case 15, HelixIsland_GuideMsg_BoatIntro_Dragon
	case 16, HelixIsland_GuideMsg_BoatIntro_Dark
	case 17, HelixIsland_GuideMsg_BoatIntro_Fairy
	return

HelixIsland_GuideMsg_ShopsIntro:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_ShopsIntro_Normal
	case 1, HelixIsland_GuideMsg_ShopsIntro_Fighting
	case 2, HelixIsland_GuideMsg_ShopsIntro_Flying
	case 3, HelixIsland_GuideMsg_ShopsIntro_Poison
	case 4, HelixIsland_GuideMsg_ShopsIntro_Ground
	case 5, HelixIsland_GuideMsg_ShopsIntro_Rock
	case 6, HelixIsland_GuideMsg_ShopsIntro_Bug
	case 7, HelixIsland_GuideMsg_ShopsIntro_Ghost
	case 8, HelixIsland_GuideMsg_ShopsIntro_Steel
	case 9, HelixIsland_GuideMsg_ShopsIntro_Fire
	case 10, HelixIsland_GuideMsg_ShopsIntro_Water
	case 11, HelixIsland_GuideMsg_ShopsIntro_Grass
	case 12, HelixIsland_GuideMsg_ShopsIntro_Electric
	case 13, HelixIsland_GuideMsg_ShopsIntro_Psychic
	case 14, HelixIsland_GuideMsg_ShopsIntro_Ice
	case 15, HelixIsland_GuideMsg_ShopsIntro_Dragon
	case 16, HelixIsland_GuideMsg_ShopsIntro_Dark
	case 17, HelixIsland_GuideMsg_ShopsIntro_Fairy
	return

HelixIsland_GuideMsg_PCIntro:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_PCIntro_Normal
	case 1, HelixIsland_GuideMsg_PCIntro_Fighting
	case 2, HelixIsland_GuideMsg_PCIntro_Flying
	case 3, HelixIsland_GuideMsg_PCIntro_Poison
	case 4, HelixIsland_GuideMsg_PCIntro_Ground
	case 5, HelixIsland_GuideMsg_PCIntro_Rock
	case 6, HelixIsland_GuideMsg_PCIntro_Bug
	case 7, HelixIsland_GuideMsg_PCIntro_Ghost
	case 8, HelixIsland_GuideMsg_PCIntro_Steel
	case 9, HelixIsland_GuideMsg_PCIntro_Fire
	case 10, HelixIsland_GuideMsg_PCIntro_Water
	case 11, HelixIsland_GuideMsg_PCIntro_Grass
	case 12, HelixIsland_GuideMsg_PCIntro_Electric
	case 13, HelixIsland_GuideMsg_PCIntro_Psychic
	case 14, HelixIsland_GuideMsg_PCIntro_Ice
	case 15, HelixIsland_GuideMsg_PCIntro_Dragon
	case 16, HelixIsland_GuideMsg_PCIntro_Dark
	case 17, HelixIsland_GuideMsg_PCIntro_Fairy
	return

HelixIsland_GuideMsg_Rest:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_Rest_Normal
	case 1, HelixIsland_GuideMsg_Rest_Fighting
	case 2, HelixIsland_GuideMsg_Rest_Flying
	case 3, HelixIsland_GuideMsg_Rest_Poison
	case 4, HelixIsland_GuideMsg_Rest_Ground
	case 5, HelixIsland_GuideMsg_Rest_Rock
	case 6, HelixIsland_GuideMsg_Rest_Bug
	case 7, HelixIsland_GuideMsg_Rest_Ghost
	case 8, HelixIsland_GuideMsg_Rest_Steel
	case 9, HelixIsland_GuideMsg_Rest_Fire
	case 10, HelixIsland_GuideMsg_Rest_Water
	case 11, HelixIsland_GuideMsg_Rest_Grass
	case 12, HelixIsland_GuideMsg_Rest_Electric
	case 13, HelixIsland_GuideMsg_Rest_Psychic
	case 14, HelixIsland_GuideMsg_Rest_Ice
	case 15, HelixIsland_GuideMsg_Rest_Dragon
	case 16, HelixIsland_GuideMsg_Rest_Dark
	case 17, HelixIsland_GuideMsg_Rest_Fairy
	return

HelixIsland_GuideMsg_ComeBack:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_ComeBack_Normal
	case 1, HelixIsland_GuideMsg_ComeBack_Fighting
	case 2, HelixIsland_GuideMsg_ComeBack_Flying
	case 3, HelixIsland_GuideMsg_ComeBack_Poison
	case 4, HelixIsland_GuideMsg_ComeBack_Ground
	case 5, HelixIsland_GuideMsg_ComeBack_Rock
	case 6, HelixIsland_GuideMsg_ComeBack_Bug
	case 7, HelixIsland_GuideMsg_ComeBack_Ghost
	case 8, HelixIsland_GuideMsg_ComeBack_Steel
	case 9, HelixIsland_GuideMsg_ComeBack_Fire
	case 10, HelixIsland_GuideMsg_ComeBack_Water
	case 11, HelixIsland_GuideMsg_ComeBack_Grass
	case 12, HelixIsland_GuideMsg_ComeBack_Electric
	case 13, HelixIsland_GuideMsg_ComeBack_Psychic
	case 14, HelixIsland_GuideMsg_ComeBack_Ice
	case 15, HelixIsland_GuideMsg_ComeBack_Dragon
	case 16, HelixIsland_GuideMsg_ComeBack_Dark
	case 17, HelixIsland_GuideMsg_ComeBack_Fairy
	return

HelixIsland_GuideMsg_RemindBall:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_RemindBall_Normal
	case 1, HelixIsland_GuideMsg_RemindBall_Fighting
	case 2, HelixIsland_GuideMsg_RemindBall_Flying
	case 3, HelixIsland_GuideMsg_RemindBall_Poison
	case 4, HelixIsland_GuideMsg_RemindBall_Ground
	case 5, HelixIsland_GuideMsg_RemindBall_Rock
	case 6, HelixIsland_GuideMsg_RemindBall_Bug
	case 7, HelixIsland_GuideMsg_RemindBall_Ghost
	case 8, HelixIsland_GuideMsg_RemindBall_Steel
	case 9, HelixIsland_GuideMsg_RemindBall_Fire
	case 10, HelixIsland_GuideMsg_RemindBall_Water
	case 11, HelixIsland_GuideMsg_RemindBall_Grass
	case 12, HelixIsland_GuideMsg_RemindBall_Electric
	case 13, HelixIsland_GuideMsg_RemindBall_Psychic
	case 14, HelixIsland_GuideMsg_RemindBall_Ice
	case 15, HelixIsland_GuideMsg_RemindBall_Dragon
	case 16, HelixIsland_GuideMsg_RemindBall_Dark
	case 17, HelixIsland_GuideMsg_RemindBall_Fairy
	return

HelixIsland_GuideMsg_Idle:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_Idle_Normal
	case 1, HelixIsland_GuideMsg_Idle_Fighting
	case 2, HelixIsland_GuideMsg_Idle_Flying
	case 3, HelixIsland_GuideMsg_Idle_Poison
	case 4, HelixIsland_GuideMsg_Idle_Ground
	case 5, HelixIsland_GuideMsg_Idle_Rock
	case 6, HelixIsland_GuideMsg_Idle_Bug
	case 7, HelixIsland_GuideMsg_Idle_Ghost
	case 8, HelixIsland_GuideMsg_Idle_Steel
	case 9, HelixIsland_GuideMsg_Idle_Fire
	case 10, HelixIsland_GuideMsg_Idle_Water
	case 11, HelixIsland_GuideMsg_Idle_Grass
	case 12, HelixIsland_GuideMsg_Idle_Electric
	case 13, HelixIsland_GuideMsg_Idle_Psychic
	case 14, HelixIsland_GuideMsg_Idle_Ice
	case 15, HelixIsland_GuideMsg_Idle_Dragon
	case 16, HelixIsland_GuideMsg_Idle_Dark
	case 17, HelixIsland_GuideMsg_Idle_Fairy
	return

HelixIsland_GuideMsg_RestReminder:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_RestReminder_Normal
	case 1, HelixIsland_GuideMsg_RestReminder_Fighting
	case 2, HelixIsland_GuideMsg_RestReminder_Flying
	case 3, HelixIsland_GuideMsg_RestReminder_Poison
	case 4, HelixIsland_GuideMsg_RestReminder_Ground
	case 5, HelixIsland_GuideMsg_RestReminder_Rock
	case 6, HelixIsland_GuideMsg_RestReminder_Bug
	case 7, HelixIsland_GuideMsg_RestReminder_Ghost
	case 8, HelixIsland_GuideMsg_RestReminder_Steel
	case 9, HelixIsland_GuideMsg_RestReminder_Fire
	case 10, HelixIsland_GuideMsg_RestReminder_Water
	case 11, HelixIsland_GuideMsg_RestReminder_Grass
	case 12, HelixIsland_GuideMsg_RestReminder_Electric
	case 13, HelixIsland_GuideMsg_RestReminder_Psychic
	case 14, HelixIsland_GuideMsg_RestReminder_Ice
	case 15, HelixIsland_GuideMsg_RestReminder_Dragon
	case 16, HelixIsland_GuideMsg_RestReminder_Dark
	case 17, HelixIsland_GuideMsg_RestReminder_Fairy
	return

HelixIsland_GuideMsg_ReadyPrompt:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_ReadyPrompt_Normal
	case 1, HelixIsland_GuideMsg_ReadyPrompt_Fighting
	case 2, HelixIsland_GuideMsg_ReadyPrompt_Flying
	case 3, HelixIsland_GuideMsg_ReadyPrompt_Poison
	case 4, HelixIsland_GuideMsg_ReadyPrompt_Ground
	case 5, HelixIsland_GuideMsg_ReadyPrompt_Rock
	case 6, HelixIsland_GuideMsg_ReadyPrompt_Bug
	case 7, HelixIsland_GuideMsg_ReadyPrompt_Ghost
	case 8, HelixIsland_GuideMsg_ReadyPrompt_Steel
	case 9, HelixIsland_GuideMsg_ReadyPrompt_Fire
	case 10, HelixIsland_GuideMsg_ReadyPrompt_Water
	case 11, HelixIsland_GuideMsg_ReadyPrompt_Grass
	case 12, HelixIsland_GuideMsg_ReadyPrompt_Electric
	case 13, HelixIsland_GuideMsg_ReadyPrompt_Psychic
	case 14, HelixIsland_GuideMsg_ReadyPrompt_Ice
	case 15, HelixIsland_GuideMsg_ReadyPrompt_Dragon
	case 16, HelixIsland_GuideMsg_ReadyPrompt_Dark
	case 17, HelixIsland_GuideMsg_ReadyPrompt_Fairy
	return

HelixIsland_GuideMsg_BoatYes:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_BoatYes_Normal
	case 1, HelixIsland_GuideMsg_BoatYes_Fighting
	case 2, HelixIsland_GuideMsg_BoatYes_Flying
	case 3, HelixIsland_GuideMsg_BoatYes_Poison
	case 4, HelixIsland_GuideMsg_BoatYes_Ground
	case 5, HelixIsland_GuideMsg_BoatYes_Rock
	case 6, HelixIsland_GuideMsg_BoatYes_Bug
	case 7, HelixIsland_GuideMsg_BoatYes_Ghost
	case 8, HelixIsland_GuideMsg_BoatYes_Steel
	case 9, HelixIsland_GuideMsg_BoatYes_Fire
	case 10, HelixIsland_GuideMsg_BoatYes_Water
	case 11, HelixIsland_GuideMsg_BoatYes_Grass
	case 12, HelixIsland_GuideMsg_BoatYes_Electric
	case 13, HelixIsland_GuideMsg_BoatYes_Psychic
	case 14, HelixIsland_GuideMsg_BoatYes_Ice
	case 15, HelixIsland_GuideMsg_BoatYes_Dragon
	case 16, HelixIsland_GuideMsg_BoatYes_Dark
	case 17, HelixIsland_GuideMsg_BoatYes_Fairy
	return

HelixIsland_GuideMsg_BoatNo:
	switch VAR_HELIX_PLAYER_TYPE
	case 0, HelixIsland_GuideMsg_BoatNo_Normal
	case 1, HelixIsland_GuideMsg_BoatNo_Fighting
	case 2, HelixIsland_GuideMsg_BoatNo_Flying
	case 3, HelixIsland_GuideMsg_BoatNo_Poison
	case 4, HelixIsland_GuideMsg_BoatNo_Ground
	case 5, HelixIsland_GuideMsg_BoatNo_Rock
	case 6, HelixIsland_GuideMsg_BoatNo_Bug
	case 7, HelixIsland_GuideMsg_BoatNo_Ghost
	case 8, HelixIsland_GuideMsg_BoatNo_Steel
	case 9, HelixIsland_GuideMsg_BoatNo_Fire
	case 10, HelixIsland_GuideMsg_BoatNo_Water
	case 11, HelixIsland_GuideMsg_BoatNo_Grass
	case 12, HelixIsland_GuideMsg_BoatNo_Electric
	case 13, HelixIsland_GuideMsg_BoatNo_Psychic
	case 14, HelixIsland_GuideMsg_BoatNo_Ice
	case 15, HelixIsland_GuideMsg_BoatNo_Dragon
	case 16, HelixIsland_GuideMsg_BoatNo_Dark
	case 17, HelixIsland_GuideMsg_BoatNo_Fairy
	return

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS — Normal (Norman)
@ Uses existing Norman text. Replace strings above to change Norman's dialogue.
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Normal:
	msgbox HelixIsland_Text_GuideGreeting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Normal:
	msgbox HelixIsland_Text_GuideFollowMe, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Normal:
	msgbox HelixIsland_Text_GuideBallHint, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Normal:
	msgbox HelixIsland_Text_GuideGoodCatch, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Normal:
	msgbox HelixIsland_Text_GuideBoatIntro, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Normal:
	msgbox HelixIsland_Text_GuideShopsIntro, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Normal:
	msgbox HelixIsland_Text_GuidePCIntro, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Normal:
	msgbox HelixIsland_Text_GuideRest, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Normal:
	msgbox HelixIsland_Text_GuideComeBack, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Normal:
	msgbox HelixIsland_Text_GuideRemindBall, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Normal:
	msgbox HelixIsland_Text_GuideIdle, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Normal:
	msgbox HelixIsland_Text_GuideRestReminder, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Normal:
	msgbox HelixIsland_Text_GuideReadyPrompt, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Normal:
	msgbox HelixIsland_Text_GuideBoatYes, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Normal:
	msgbox HelixIsland_Text_GuideBoatNo, MSGBOX_DEFAULT
	return

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Fighting (Bruno)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Fighting:
	msgbox HelixIsland_Text_Greeting_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Fighting:
	msgbox HelixIsland_Text_FollowMe_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Fighting:
	msgbox HelixIsland_Text_BallHint_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Fighting:
	msgbox HelixIsland_Text_GoodCatch_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Fighting:
	msgbox HelixIsland_Text_BoatIntro_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Fighting:
	msgbox HelixIsland_Text_ShopsIntro_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Fighting:
	msgbox HelixIsland_Text_PCIntro_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Fighting:
	msgbox HelixIsland_Text_Rest_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Fighting:
	msgbox HelixIsland_Text_ComeBack_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Fighting:
	msgbox HelixIsland_Text_RemindBall_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Fighting:
	msgbox HelixIsland_Text_Idle_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Fighting:
	msgbox HelixIsland_Text_RestReminder_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Fighting:
	msgbox HelixIsland_Text_ReadyPrompt_Fighting, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Fighting:
	msgbox HelixIsland_Text_BoatYes_Fighting, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Fighting:
	msgbox HelixIsland_Text_BoatNo_Fighting, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Fighting:
	.string "Hmph. You made landfall.\p"
	.string "Good. A strong body begins\n"
	.string "by surviving the first trial.\p"
	.string "This island is your training\n"
	.string "ground now. Let us begin.$"
HelixIsland_Text_FollowMe_Fighting:
	.string "Come. The shore offers your\n"
	.string "first lesson.$"
HelixIsland_Text_BallHint_Fighting:
	.string "That ball contains a POKéMON\n"
	.string "that washed ashore.\p"
	.string "This happens every day.\n"
	.string "Grip opportunity when it appears.\p"
	.string "Go. Claim it.$"
HelixIsland_Text_GoodCatch_Fighting:
	.string "Well done.\p"
	.string "Each morning, another will come.\n"
	.string "Train yourself to notice each one.\p"
	.string "Now follow.$"
HelixIsland_Text_BoatIntro_Fighting:
	.string "The boat is not ready.\p"
	.string "When it is, we will enter\n"
	.string "the Nexus.\p"
	.string "There, strength is tested\n"
	.string "without mercy.\p"
	.string "Tomorrow will be soon enough.$"
HelixIsland_Text_ShopsIntro_Fighting:
	.string "These fronts were built for\n"
	.string "merchants.\p"
	.string "They were expected some time ago,\n"
	.string "but none have arrived.\p"
	.string "We will endure without them\n"
	.string "for now.$"
HelixIsland_Text_PCIntro_Fighting:
	.string "This is the 'magic sapling'.\p"
	.string "I have struck it, studied it,\n"
	.string "and learned very little.\p"
	.string "Deoxys says you can commune\n"
	.string "with the 'real island' through it.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "With fortune and good pairing,\n"
	.string "you may wake to Eggs.\p"
	.string "Those Eggs may hatch carrying\n"
	.string "unusual strengths.$"
HelixIsland_Text_Rest_Fighting:
	.string "Enough for one day.\p"
	.string "Eat. Rest. Let your muscles heal.\p"
	.string "Tomorrow, true training begins.\n"
	.string "I will wait by the boat.$"
HelixIsland_Text_ComeBack_Fighting:
	.string "Stop. The ball is there.\n"
	.string "Do not leave a trial unfinished.$"
HelixIsland_Text_RemindBall_Fighting:
	.string "Claim the ball.\n"
	.string "Then we continue.$"
HelixIsland_Text_Idle_Fighting:
	.string "Study the ground.\n"
	.string "Every training field teaches.$"
HelixIsland_Text_RestReminder_Fighting:
	.string "Rest. Growth demands recovery.$"
HelixIsland_Text_ReadyPrompt_Fighting:
	.string "Ready to test yourself\n"
	.string "in the Nexus?$"
HelixIsland_Text_BoatYes_Fighting:
	.string "Good. Face it head-on.$"
HelixIsland_Text_BoatNo_Fighting:
	.string "Wise. Hesitation can mean\n"
	.string "preparation.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Flying (Winona)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Flying:
	msgbox HelixIsland_Text_Greeting_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Flying:
	msgbox HelixIsland_Text_FollowMe_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Flying:
	msgbox HelixIsland_Text_BallHint_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Flying:
	msgbox HelixIsland_Text_GoodCatch_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Flying:
	msgbox HelixIsland_Text_BoatIntro_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Flying:
	msgbox HelixIsland_Text_ShopsIntro_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Flying:
	msgbox HelixIsland_Text_PCIntro_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Flying:
	msgbox HelixIsland_Text_Rest_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Flying:
	msgbox HelixIsland_Text_ComeBack_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Flying:
	msgbox HelixIsland_Text_RemindBall_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Flying:
	msgbox HelixIsland_Text_Idle_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Flying:
	msgbox HelixIsland_Text_RestReminder_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Flying:
	msgbox HelixIsland_Text_ReadyPrompt_Flying, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Flying:
	msgbox HelixIsland_Text_BoatYes_Flying, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Flying:
	msgbox HelixIsland_Text_BoatNo_Flying, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Flying:
	.string "Ah, there you are.\p"
	.string "The winds carried you safely.\p"
	.string "This island will be your perch now.\n"
	.string "Shall we take flight?$"
HelixIsland_Text_FollowMe_Flying:
	.string "Come along. The shore has\n"
	.string "something waiting for you.$"
HelixIsland_Text_BallHint_Flying:
	.string "See that ball by the tide?\p"
	.string "A POKéMON has drifted in.\n"
	.string "The island sends one each morning.\p"
	.string "Go on. Let it join your flock.$"
HelixIsland_Text_GoodCatch_Flying:
	.string "Gracefully done.\p"
	.string "Each morning, another chance\n"
	.string "will arrive on the wind.\p"
	.string "Now, follow me.$"
HelixIsland_Text_BoatIntro_Flying:
	.string "The boat's wings are not ready yet.\p"
	.string "When they are, we can sail\n"
	.string "to the Nexus.\p"
	.string "That sky is harsher than this one.\n"
	.string "We wait until tomorrow.$"
HelixIsland_Text_ShopsIntro_Flying:
	.string "These storefronts were meant\n"
	.string "to welcome merchants.\p"
	.string "They should have arrived by now,\n"
	.string "but the horizon remains empty.$"
HelixIsland_Text_PCIntro_Flying:
	.string "This little 'magic sapling'...\n"
	.string "it feels like a nest between skies.\p"
	.string "I cannot guide its currents.\n"
	.string "Deoxys said you could.\p"
	.string "Through it, you may commune with\n"
	.string "some 'real island'.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "With luck, Eggs will be waiting\n"
	.string "when morning comes.\p"
	.string "With more luck, they may hatch\n"
	.string "with rare and graceful traits.$"
HelixIsland_Text_Rest_Flying:
	.string "That is enough flight for today.\p"
	.string "Rest your wings tonight.\n"
	.string "Tomorrow, we rise for real.\p"
	.string "I will be by the boat at dawn.$"
HelixIsland_Text_ComeBack_Flying:
	.string "Careful. The ball is this way.\n"
	.string "Do not drift off yet.$"
HelixIsland_Text_RemindBall_Flying:
	.string "The shore's gift is waiting.\n"
	.string "Please pick up the ball.$"
HelixIsland_Text_Idle_Flying:
	.string "Feel the wind.\n"
	.string "It tells you where you are.$"
HelixIsland_Text_RestReminder_Flying:
	.string "Rest now.\n"
	.string "Tomorrow's winds will be stronger.$"
HelixIsland_Text_ReadyPrompt_Flying:
	.string "Are you ready to fly toward\n"
	.string "the Nexus?$"
HelixIsland_Text_BoatYes_Flying:
	.string "Then spread your wings.$"
HelixIsland_Text_BoatNo_Flying:
	.string "Take your time.\n"
	.string "Even birds wait for the right wind.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Poison (Koga)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Poison:
	msgbox HelixIsland_Text_Greeting_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Poison:
	msgbox HelixIsland_Text_FollowMe_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Poison:
	msgbox HelixIsland_Text_BallHint_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Poison:
	msgbox HelixIsland_Text_GoodCatch_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Poison:
	msgbox HelixIsland_Text_BoatIntro_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Poison:
	msgbox HelixIsland_Text_ShopsIntro_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Poison:
	msgbox HelixIsland_Text_PCIntro_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Poison:
	msgbox HelixIsland_Text_Rest_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Poison:
	msgbox HelixIsland_Text_ComeBack_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Poison:
	msgbox HelixIsland_Text_RemindBall_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Poison:
	msgbox HelixIsland_Text_Idle_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Poison:
	msgbox HelixIsland_Text_RestReminder_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Poison:
	msgbox HelixIsland_Text_ReadyPrompt_Poison, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Poison:
	msgbox HelixIsland_Text_BoatYes_Poison, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Poison:
	msgbox HelixIsland_Text_BoatNo_Poison, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Poison:
	.string "So. You arrived.\p"
	.string "Few steps are wasted here.\p"
	.string "This island is a hidden dojo now.\n"
	.string "Attend closely.$"
HelixIsland_Text_FollowMe_Poison:
	.string "Come. The shore conceals\n"
	.string "your first lesson.$"
HelixIsland_Text_BallHint_Poison:
	.string "Observe the ball.\p"
	.string "A POKéMON washed ashore.\n"
	.string "This occurs each day.\p"
	.string "Approach. Claim it.$"
HelixIsland_Text_GoodCatch_Poison:
	.string "Hmph. Acceptable.\p"
	.string "Each morning brings another.\p"
	.string "The careless miss what lies\n"
	.string "in plain sight.\p"
	.string "Now, this way.$"
HelixIsland_Text_BoatIntro_Poison:
	.string "The boat remains unfinished.\p"
	.string "When repaired, it will carry us\n"
	.string "to the Nexus.\p"
	.string "There, danger creeps from\n"
	.string "every side.\p"
	.string "We enter tomorrow.$"
HelixIsland_Text_ShopsIntro_Poison:
	.string "These stalls were prepared\n"
	.string "for merchants.\p"
	.string "They were expected long ago,\n"
	.string "yet none have appeared.\p"
	.string "An absent ally is a lesson:\n"
	.string "rely first on yourself.$"
HelixIsland_Text_PCIntro_Poison:
	.string "This is the 'magic sapling'.\p"
	.string "A strange device, if device it is.\n"
	.string "My arts do not open it.\p"
	.string "Deoxys claims you can commune\n"
	.string "with the 'real island' through it.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "If fortune favors the hidden path,\n"
	.string "Eggs may appear by morning.\p"
	.string "Those Eggs may carry traits\n"
	.string "ordinary eyes would miss.$"
HelixIsland_Text_Rest_Poison:
	.string "The first lesson ends.\p"
	.string "Rest. A dull mind misses traps.\p"
	.string "Tomorrow, the true mission begins.\n"
	.string "I will wait by the boat.$"
HelixIsland_Text_ComeBack_Poison:
	.string "No. The ball is there.\n"
	.string "Do not break formation.$"
HelixIsland_Text_RemindBall_Poison:
	.string "Retrieve the ball.\n"
	.string "Swiftly.$"
HelixIsland_Text_Idle_Poison:
	.string "Look again.\n"
	.string "There is always something hidden.$"
HelixIsland_Text_RestReminder_Poison:
	.string "Sleep.\n"
	.string "Even shadows must wait.$"
HelixIsland_Text_ReadyPrompt_Poison:
	.string "Ready to enter the Nexus?$"
HelixIsland_Text_BoatYes_Poison:
	.string "Then move without hesitation.$"
HelixIsland_Text_BoatNo_Poison:
	.string "Good. Patience is also a weapon.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Ground (Giovanni)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Ground:
	msgbox HelixIsland_Text_Greeting_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Ground:
	msgbox HelixIsland_Text_FollowMe_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Ground:
	msgbox HelixIsland_Text_BallHint_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Ground:
	msgbox HelixIsland_Text_GoodCatch_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Ground:
	msgbox HelixIsland_Text_BoatIntro_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Ground:
	msgbox HelixIsland_Text_ShopsIntro_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Ground:
	msgbox HelixIsland_Text_PCIntro_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Ground:
	msgbox HelixIsland_Text_Rest_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Ground:
	msgbox HelixIsland_Text_ComeBack_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Ground:
	msgbox HelixIsland_Text_RemindBall_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Ground:
	msgbox HelixIsland_Text_Idle_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Ground:
	msgbox HelixIsland_Text_RestReminder_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Ground:
	msgbox HelixIsland_Text_ReadyPrompt_Ground, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Ground:
	msgbox HelixIsland_Text_BoatYes_Ground, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Ground:
	msgbox HelixIsland_Text_BoatNo_Ground, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Ground:
	.string "So, you've arrived.\p"
	.string "Good. This island has potential,\n"
	.string "if it is managed correctly.\p"
	.string "For now, consider it your base\n"
	.string "of operations.$"
HelixIsland_Text_FollowMe_Ground:
	.string "Come. There is an asset\n"
	.string "on the shore.$"
HelixIsland_Text_BallHint_Ground:
	.string "That ball contains a POKéMON\n"
	.string "washed in by the tide.\p"
	.string "This island produces one\n"
	.string "opportunity like it every morning.\p"
	.string "Take it. Don't waste resources.$"
HelixIsland_Text_GoodCatch_Ground:
	.string "Good.\p"
	.string "A new one appears each morning.\p"
	.string "Acquire what strengthens \n"
	.string "your position.\p"
	.string "Now, this way.$"
HelixIsland_Text_BoatIntro_Ground:
	.string "The boat is not seaworthy yet.\p"
	.string "When it is, it will carry us\n"
	.string "to the Nexus.\p"
	.string "That place is where power is won.\n"
	.string "We move tomorrow, not before.$"
HelixIsland_Text_ShopsIntro_Ground:
	.string "These storefronts were prepared\n"
	.string "for merchants.\p"
	.string "They were expected some time ago,\n"
	.string "but they have failed to report.\p"
	.string "Unreliable partners.\n"
	.string "Plan accordingly.$"
HelixIsland_Text_PCIntro_Ground:
	.string "This is the so-called\n"
	.string "'magic sapling'.\p"
	.string "I have not determined its mechanism.\n"
	.string "Deoxys says you are the key.\p"
	.string "Use it to commune with the\n"
	.string "'real island', whatever that means.\p"
	.string "For now, transfer all three of your\n"
	.string "POKéMON over there.\p"
	.string "With favorable conditions, Eggs\n"
	.string "may be produced by morning.\p"
	.string "If fortune is generous, those Eggs\n"
	.string "may hatch with useful traits.\p"
	.string "Remember: growth is power.$"
HelixIsland_Text_Rest_Ground:
	.string "That concludes the briefing.\p"
	.string "Rest. Tomorrow, we act.\p"
	.string "I will be waiting by the boat.$"
HelixIsland_Text_ComeBack_Ground:
	.string "The ball is over there.\n"
	.string "Stay focused.$"
HelixIsland_Text_RemindBall_Ground:
	.string "Take the ball.\n"
	.string "We cannot proceed otherwise.$"
HelixIsland_Text_Idle_Ground:
	.string "Survey the island.\n"
	.string "Know every inch you intend to rule.$"
HelixIsland_Text_RestReminder_Ground:
	.string "Sleep now.\n"
	.string "Tomorrow requires a clear head.$"
HelixIsland_Text_ReadyPrompt_Ground:
	.string "Ready to depart for the Nexus?$"
HelixIsland_Text_BoatYes_Ground:
	.string "Excellent. Proceed.$"
HelixIsland_Text_BoatNo_Ground:
	.string "Very well.\n"
	.string "Return when your resolve is firm.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Rock (Brock)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Rock:
	msgbox HelixIsland_Text_Greeting_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Rock:
	msgbox HelixIsland_Text_FollowMe_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Rock:
	msgbox HelixIsland_Text_BallHint_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Rock:
	msgbox HelixIsland_Text_GoodCatch_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Rock:
	msgbox HelixIsland_Text_BoatIntro_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Rock:
	msgbox HelixIsland_Text_ShopsIntro_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Rock:
	msgbox HelixIsland_Text_PCIntro_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Rock:
	msgbox HelixIsland_Text_Rest_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Rock:
	msgbox HelixIsland_Text_ComeBack_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Rock:
	msgbox HelixIsland_Text_RemindBall_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Rock:
	msgbox HelixIsland_Text_Idle_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Rock:
	msgbox HelixIsland_Text_RestReminder_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Rock:
	msgbox HelixIsland_Text_ReadyPrompt_Rock, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Rock:
	msgbox HelixIsland_Text_BoatYes_Rock, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Rock:
	msgbox HelixIsland_Text_BoatNo_Rock, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Rock:
	.string "Hey, you made it!\p"
	.string "I'm glad you're safe.\p"
	.string "This island's rough right now,\n"
	.string "but we'll make it sturdy together.$"
HelixIsland_Text_FollowMe_Rock:
	.string "Come with me.\n"
	.string "There's something by the shore.$"
HelixIsland_Text_BallHint_Rock:
	.string "See that ball?\p"
	.string "A POKéMON washed up in it.\n"
	.string "Seems to happen every morning here.\p"
	.string "Go ahead and pick it up.\n"
	.string "We'll take care of it.$"
HelixIsland_Text_GoodCatch_Rock:
	.string "Nice work.\p"
	.string "A new one may wash up each morning,\n"
	.string "so check the shore every day.\p"
	.string "Now let's keep moving.$"
HelixIsland_Text_BoatIntro_Rock:
	.string "That boat still needs repairs.\p"
	.string "Once it's sturdy enough, we can\n"
	.string "head for the Nexus.\p"
	.string "That's where things get dangerous.\n"
	.string "We'll wait until tomorrow.$"
HelixIsland_Text_ShopsIntro_Rock:
	.string "I set aside space for shops here.\p"
	.string "The merchants should've arrived\n"
	.string "by now, but no one has come.\p"
	.string "So for now, we'll have to be\n"
	.string "careful with supplies.$"
HelixIsland_Text_PCIntro_Rock:
	.string "This little 'magic sapling'\n"
	.string "is something else.\p"
	.string "I can't make much sense of it,\n"
	.string "but Deoxys said you could.\p"
	.string "It connects to the 'real island',\n"
	.string "whatever that really means.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "If they're comfortable, you may\n"
	.string "wake up to an Egg or two.\p"
	.string "And if we're lucky, those Eggs\n"
	.string "may hatch with special traits.\p"
	.string "Treat them well from the start.$"
HelixIsland_Text_Rest_Rock:
	.string "That's enough for today.\p"
	.string "Get some food and sleep.\n"
	.string "Growing strong takes rest, too.\p"
	.string "I'll be by the boat tomorrow.$"
HelixIsland_Text_ComeBack_Rock:
	.string "Careful. The ball's this way.\n"
	.string "Don't leave it behind.$"
HelixIsland_Text_RemindBall_Rock:
	.string "Go ahead and pick up the ball.\n"
	.string "I'll wait right here.$"
HelixIsland_Text_Idle_Rock:
	.string "Take a look around.\n"
	.string "A good home starts with care.$"
HelixIsland_Text_RestReminder_Rock:
	.string "You should sleep.\n"
	.string "Tomorrow will take energy.$"
HelixIsland_Text_ReadyPrompt_Rock:
	.string "Ready to head for the Nexus?$"
HelixIsland_Text_BoatYes_Rock:
	.string "Right. I'll keep an eye on you.$"
HelixIsland_Text_BoatNo_Rock:
	.string "No problem.\n"
	.string "Preparation is part of care.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Bug (Bug Catcher)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Bug:
	msgbox HelixIsland_Text_Greeting_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Bug:
	msgbox HelixIsland_Text_FollowMe_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Bug:
	msgbox HelixIsland_Text_BallHint_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Bug:
	msgbox HelixIsland_Text_GoodCatch_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Bug:
	msgbox HelixIsland_Text_BoatIntro_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Bug:
	msgbox HelixIsland_Text_ShopsIntro_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Bug:
	msgbox HelixIsland_Text_PCIntro_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Bug:
	msgbox HelixIsland_Text_Rest_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Bug:
	msgbox HelixIsland_Text_ComeBack_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Bug:
	msgbox HelixIsland_Text_RemindBall_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Bug:
	msgbox HelixIsland_Text_Idle_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Bug:
	msgbox HelixIsland_Text_RestReminder_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Bug:
	msgbox HelixIsland_Text_ReadyPrompt_Bug, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Bug:
	msgbox HelixIsland_Text_BoatYes_Bug, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Bug:
	msgbox HelixIsland_Text_BoatNo_Bug, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Bug:
	.string "Whoa! You really made it!\p"
	.string "This island is crawling with\n"
	.string "possibilities!\p"
	.string "Come on, come on.\n"
	.string "We've got stuff to fix!$"
HelixIsland_Text_FollowMe_Bug:
	.string "Follow me!\n"
	.string "Something washed up by the shore!$"
HelixIsland_Text_BallHint_Bug:
	.string "See that ball?\p"
	.string "A POKéMON washed up inside it!\n"
	.string "This happens every single morning!\p"
	.string "Go pick it up! Hurry!$"
HelixIsland_Text_GoodCatch_Bug:
	.string "Yes! Nice catch!\p"
	.string "Every morning, another one can\n"
	.string "show up on the beach.\p"
	.string "Never skip a morning check!\n"
	.string "Now come on!$"
HelixIsland_Text_BoatIntro_Bug:
	.string "That boat's still busted.\p"
	.string "Once it's ready, we can go\n"
	.string "to the Nexus!\p"
	.string "That's where the BIG stuff happens.\n"
	.string "Tomorrow, though. Probably safer.$"
HelixIsland_Text_ShopsIntro_Bug:
	.string "These are supposed to be shops.\p"
	.string "The merchants were meant to be here,\n"
	.string "but... nope. Nothing.\p"
	.string "Maybe they're lost?\n"
	.string "Or hiding? I checked for tracks!$"
HelixIsland_Text_PCIntro_Bug:
	.string "Okay, this is the\n"
	.string "'magic sapling'.\p"
	.string "I poked it with a stick.\n"
	.string "It did not like that.\p"
	.string "Deoxys said you can use it to\n"
	.string "commune with the 'real island'.\p"
	.string "I don't know what that means,\n"
	.string "but it sounds amazing!\p"
	.string "Send all three of your POKéMON\n"
	.string "there for now.\p"
	.string "Maybe you'll wake up to Eggs!\p"
	.string "And maybe those Eggs hatch with\n"
	.string "weird, awesome traits!$"
HelixIsland_Text_Rest_Bug:
	.string "That's the tour!\p"
	.string "Get some sleep tonight.\n"
	.string "Tomorrow we go bug-hunting...\p"
	.string "I mean, Nexus-hunting!\n"
	.string "I'll be by the boat!$"
HelixIsland_Text_ComeBack_Bug:
	.string "Hey! The ball's over there!\n"
	.string "Don't wander away yet!$"
HelixIsland_Text_RemindBall_Bug:
	.string "Pick up the ball!\n"
	.string "This is the exciting part!$"
HelixIsland_Text_Idle_Bug:
	.string "Look around!\n"
	.string "There might be rare stuff anywhere!$"
HelixIsland_Text_RestReminder_Bug:
	.string "Sleep now!\n"
	.string "Tomorrow's gonna be huge!$"
HelixIsland_Text_ReadyPrompt_Bug:
	.string "Ready to head out?!$"
HelixIsland_Text_BoatYes_Bug:
	.string "Yes! Adventure time!$"
HelixIsland_Text_BoatNo_Bug:
	.string "Okay! I'll wait.\n"
	.string "Waiting is hard, but I can do it.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Ghost (Agatha)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Ghost:
	msgbox HelixIsland_Text_Greeting_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Ghost:
	msgbox HelixIsland_Text_FollowMe_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Ghost:
	msgbox HelixIsland_Text_BallHint_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Ghost:
	msgbox HelixIsland_Text_GoodCatch_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Ghost:
	msgbox HelixIsland_Text_BoatIntro_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Ghost:
	msgbox HelixIsland_Text_ShopsIntro_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Ghost:
	msgbox HelixIsland_Text_PCIntro_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Ghost:
	msgbox HelixIsland_Text_Rest_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Ghost:
	msgbox HelixIsland_Text_ComeBack_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Ghost:
	msgbox HelixIsland_Text_RemindBall_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Ghost:
	msgbox HelixIsland_Text_Idle_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Ghost:
	msgbox HelixIsland_Text_RestReminder_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Ghost:
	msgbox HelixIsland_Text_ReadyPrompt_Ghost, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Ghost:
	msgbox HelixIsland_Text_BoatYes_Ghost, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Ghost:
	msgbox HelixIsland_Text_BoatNo_Ghost, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Ghost:
	.string "Heh. So you made it.\p"
	.string "You've got more backbone than\n"
	.string "you look like.\p"
	.string "This island's yours now.\n"
	.string "Try not to haunt it too soon.$"
HelixIsland_Text_FollowMe_Ghost:
	.string "Come along.\n"
	.string "The shore has coughed up a surprise.$"
HelixIsland_Text_BallHint_Ghost:
	.string "See that ball?\p"
	.string "A POKéMON washed ashore.\p"
	.string "Happens every morning,\n"
	.string "like clockwork.\p"
	.string "Well? Pick it up.$"
HelixIsland_Text_GoodCatch_Ghost:
	.string "Not bad.\p"
	.string "A new one will appear each morning.\n"
	.string "Ignore gifts from the dead sea\p"
	.string "and you'll regret it. This way.$"
HelixIsland_Text_BoatIntro_Ghost:
	.string "That boat's still half-dead.\p"
	.string "When it's patched up, it'll take us\n"
	.string "to the Nexus.\p"
	.string "That place will chew up\n"
	.string "soft Trainers.\p"
	.string "We'll start tomorrow.$"
HelixIsland_Text_ShopsIntro_Ghost:
	.string "These storefronts were meant\n"
	.string "for merchants.\p"
	.string "They were supposed to arrive,\n"
	.string "but they've gone quiet.\p"
	.string "Maybe delayed. Maybe worse.\n"
	.string "Either way, we manage.$"
HelixIsland_Text_PCIntro_Ghost:
	.string "Here is the little\n"
	.string "'magic sapling'.\p"
	.string "Cute name for something that makes\n"
	.string "my skin crawl.\p"
	.string "Deoxys says you can commune with\n"
	.string "the 'real island' through it.\p"
	.string "Don't ask me what that means.\n"
	.string "Ask the thing whispering in roots.\p"
	.string "Send all three of your POKéMON\n"
	.string "over there for now.\p"
	.string "With luck, you'll wake to Eggs.\p"
	.string "With better luck, they'll hatch\n"
	.string "with traits worth keeping.$"
HelixIsland_Text_Rest_Ghost:
	.string "That's enough.\p"
	.string "Sleep while the island lets you.\n"
	.string "Tomorrow, we begin properly.\p"
	.string "I'll be by the boat.$"
HelixIsland_Text_ComeBack_Ghost:
	.string "Not that way.\n"
	.string "The ball, child. The ball.$"
HelixIsland_Text_RemindBall_Ghost:
	.string "Pick up the ball.\n"
	.string "It won't bite. Probably.$"
HelixIsland_Text_Idle_Ghost:
	.string "Take a look around.\n"
	.string "The island is watching too.$"
HelixIsland_Text_RestReminder_Ghost:
	.string "Go sleep.\n"
	.string "You look too alive to be tired.$"
HelixIsland_Text_ReadyPrompt_Ghost:
	.string "Ready for the Nexus?$"
HelixIsland_Text_BoatYes_Ghost:
	.string "Heh. Let's see what survives.$"
HelixIsland_Text_BoatNo_Ghost:
	.string "Fine.\n"
	.string "Fear can be sensible.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Steel (Steven)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Steel:
	msgbox HelixIsland_Text_Greeting_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Steel:
	msgbox HelixIsland_Text_FollowMe_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Steel:
	msgbox HelixIsland_Text_BallHint_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Steel:
	msgbox HelixIsland_Text_GoodCatch_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Steel:
	msgbox HelixIsland_Text_BoatIntro_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Steel:
	msgbox HelixIsland_Text_ShopsIntro_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Steel:
	msgbox HelixIsland_Text_PCIntro_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Steel:
	msgbox HelixIsland_Text_Rest_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Steel:
	msgbox HelixIsland_Text_ComeBack_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Steel:
	msgbox HelixIsland_Text_RemindBall_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Steel:
	msgbox HelixIsland_Text_Idle_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Steel:
	msgbox HelixIsland_Text_RestReminder_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Steel:
	msgbox HelixIsland_Text_ReadyPrompt_Steel, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Steel:
	msgbox HelixIsland_Text_BoatYes_Steel, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Steel:
	msgbox HelixIsland_Text_BoatNo_Steel, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Steel:
	.string "Ah, there you are.\p"
	.string "I'm pleased you arrived safely.\p"
	.string "This island is unrefined,\n"
	.string "but there is brilliance beneath it.\p"
	.string "Let us begin.$"
HelixIsland_Text_FollowMe_Steel:
	.string "Come with me.\p"
	.string "Something unusual is waiting\n"
	.string "near the shore.$"
HelixIsland_Text_BallHint_Steel:
	.string "Do you see that ball?\p"
	.string "A POKéMON washed ashore inside it.\p"
	.string "Apparently, this occurs\n"
	.string "each morning.\p"
	.string "Please pick it up.\n"
	.string "It may prove invaluable.$"
HelixIsland_Text_GoodCatch_Steel:
	.string "Excellent.\p"
	.string "Each morning may bring\n"
	.string "a new specimen.\p"
	.string "Do not overlook such gifts.\p"
	.string "Now, this way.$"
HelixIsland_Text_BoatIntro_Steel:
	.string "This boat still requires work.\p"
	.string "Once repaired, it will take us\n"
	.string "to the Nexus.\p"
	.string "A fascinating place, and dangerous.\n"
	.string "We should wait until tomorrow.$"
HelixIsland_Text_ShopsIntro_Steel:
	.string "These storefronts were prepared\n"
	.string "for incoming merchants.\p"
	.string "They were expected some time ago,\n"
	.string "but there has been no word.\p"
	.string "A disappointing delay,\n"
	.string "but not an insurmountable one.$"
HelixIsland_Text_PCIntro_Steel:
	.string "This is the so-called\n"
	.string "'magic sapling'.\p"
	.string "Its structure is... remarkable,\n"
	.string "though I cannot explain it.\p"
	.string "Deoxys indicated that you can\n"
	.string "commune with the 'real island'.\p"
	.string "A curious phrase, isn't it?\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "By morning, you may discover\n"
	.string "an Egg or two.\p"
	.string "More intriguing still, those Eggs\n"
	.string "may hatch with unusual traits.\p"
	.string "We will need to study this closely.$"
HelixIsland_Text_Rest_Steel:
	.string "That covers the essentials.\p"
	.string "Rest tonight.\n"
	.string "Tomorrow, our expedition begins.\p"
	.string "I will be waiting by the boat.$"
HelixIsland_Text_ComeBack_Steel:
	.string "This way, please.\n"
	.string "The ball is still waiting.$"
HelixIsland_Text_RemindBall_Steel:
	.string "Please pick up the ball.\n"
	.string "Then we can continue.$"
HelixIsland_Text_Idle_Steel:
	.string "Observe the island carefully.\n"
	.string "Details reveal value.$"
HelixIsland_Text_RestReminder_Steel:
	.string "You should rest.\n"
	.string "Discovery favors a clear mind.$"
HelixIsland_Text_ReadyPrompt_Steel:
	.string "Ready to depart for the Nexus?$"
HelixIsland_Text_BoatYes_Steel:
	.string "Splendid. Let us proceed.$"
HelixIsland_Text_BoatNo_Steel:
	.string "Of course.\n"
	.string "Preparedness is never wasted.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Fire (Flannery)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Fire:
	msgbox HelixIsland_Text_Greeting_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Fire:
	msgbox HelixIsland_Text_FollowMe_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Fire:
	msgbox HelixIsland_Text_BallHint_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Fire:
	msgbox HelixIsland_Text_GoodCatch_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Fire:
	msgbox HelixIsland_Text_BoatIntro_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Fire:
	msgbox HelixIsland_Text_ShopsIntro_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Fire:
	msgbox HelixIsland_Text_PCIntro_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Fire:
	msgbox HelixIsland_Text_Rest_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Fire:
	msgbox HelixIsland_Text_ComeBack_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Fire:
	msgbox HelixIsland_Text_RemindBall_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Fire:
	msgbox HelixIsland_Text_Idle_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Fire:
	msgbox HelixIsland_Text_RestReminder_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Fire:
	msgbox HelixIsland_Text_ReadyPrompt_Fire, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Fire:
	msgbox HelixIsland_Text_BoatYes_Fire, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Fire:
	msgbox HelixIsland_Text_BoatNo_Fire, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Fire:
	.string "Ah! You're here!\p"
	.string "Good! Great! I mean--\n"
	.string "I'm glad you're safe.\p"
	.string "This island's our home now,\n"
	.string "so let's fire it up!$"
HelixIsland_Text_FollowMe_Fire:
	.string "Follow me!\p"
	.string "There's something on the shore\n"
	.string "I need to show you!$"
HelixIsland_Text_BallHint_Fire:
	.string "See that ball?\p"
	.string "A POKéMON washed up in it.\p"
	.string "Apparently, that happens\n"
	.string "every morning!\p"
	.string "Go ahead! Pick it up!$"
HelixIsland_Text_GoodCatch_Fire:
	.string "Nice! Really nice!\p"
	.string "You'll see a new one wash up\n"
	.string "every morning, so don't miss it!\p"
	.string "Okay, next stop!$"
HelixIsland_Text_BoatIntro_Fire:
	.string "That boat still needs work.\p"
	.string "Once it's ready, I'll take us\n"
	.string "to the Nexus.\p"
	.string "That's where things get\n"
	.string "seriously hot!\p"
	.string "But, uh, tomorrow. Safer that way.$"
HelixIsland_Text_ShopsIntro_Fire:
	.string "These are the storefronts\n"
	.string "I set up.\p"
	.string "Some merchants were supposed to\n"
	.string "come ages ago...\p"
	.string "But they haven't shown up.\n"
	.string "We'll just have to handle it!$"
HelixIsland_Text_PCIntro_Fire:
	.string "This is our little\n"
	.string "'magic sapling'.\p"
	.string "I've tried asking it nicely,\n"
	.string "but I still don't get it.\p"
	.string "Deoxys said you can commune with\n"
	.string "the 'real island' through it.\p"
	.string "Whatever that means, it's important!\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "If things heat up just right,\n"
	.string "you might wake up to Eggs.\p"
	.string "And those Eggs might hatch\n"
	.string "with some blazing traits!$"
HelixIsland_Text_Rest_Fire:
	.string "That's the basics!\p"
	.string "Get some rest tonight.\n"
	.string "Tomorrow, we start for real!\p"
	.string "I'll be by the boat in the morning!$"
HelixIsland_Text_ComeBack_Fire:
	.string "Whoa, hold up!\n"
	.string "The ball's over there!$"
HelixIsland_Text_RemindBall_Fire:
	.string "Go on! Pick up the ball!\n"
	.string "You've got this!$"
HelixIsland_Text_Idle_Fire:
	.string "Take a look around.\n"
	.string "We'll get this place burning bright!$"
HelixIsland_Text_RestReminder_Fire:
	.string "You should sleep!\n"
	.string "Big day tomorrow!$"
HelixIsland_Text_ReadyPrompt_Fire:
	.string "Ready to head out?$"
HelixIsland_Text_BoatYes_Fire:
	.string "All right! Let's blaze a trail!$"
HelixIsland_Text_BoatNo_Fire:
	.string "No worries!\n"
	.string "We'll go when your fire's ready.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Water (Misty)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Water:
	msgbox HelixIsland_Text_Greeting_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Water:
	msgbox HelixIsland_Text_FollowMe_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Water:
	msgbox HelixIsland_Text_BallHint_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Water:
	msgbox HelixIsland_Text_GoodCatch_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Water:
	msgbox HelixIsland_Text_BoatIntro_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Water:
	msgbox HelixIsland_Text_ShopsIntro_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Water:
	msgbox HelixIsland_Text_PCIntro_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Water:
	msgbox HelixIsland_Text_Rest_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Water:
	msgbox HelixIsland_Text_ComeBack_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Water:
	msgbox HelixIsland_Text_RemindBall_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Water:
	msgbox HelixIsland_Text_Idle_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Water:
	msgbox HelixIsland_Text_RestReminder_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Water:
	msgbox HelixIsland_Text_ReadyPrompt_Water, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Water:
	msgbox HelixIsland_Text_BoatYes_Water, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Water:
	msgbox HelixIsland_Text_BoatNo_Water, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Water:
	.string "There you are!\p"
	.string "Glad you didn't wash up face-first.\p"
	.string "This island is home now,\n"
	.string "so we'd better make it work.$"
HelixIsland_Text_FollowMe_Water:
	.string "Come on.\p"
	.string "There's something on the shore\n"
	.string "you need to see.$"
HelixIsland_Text_BallHint_Water:
	.string "See that ball?\p"
	.string "A POKéMON washed up in it.\n"
	.string "This tide brings one every morning.\p"
	.string "Go ahead. Pick it up.$"
HelixIsland_Text_GoodCatch_Water:
	.string "Nice catch!\p"
	.string "Check the shore every morning.\n"
	.string "You never know what the tide brings.\p"
	.string "Now come on, over here.$"
HelixIsland_Text_BoatIntro_Water:
	.string "That boat still needs work.\p"
	.string "Once it's ready, we'll ride out\n"
	.string "to the Nexus.\p"
	.string "That's where the waves get rough.\n"
	.string "We'll save it for tomorrow.$"
HelixIsland_Text_ShopsIntro_Water:
	.string "These storefronts are supposed\n"
	.string "to have merchants.\p"
	.string "They should've shown up by now,\n"
	.string "but nobody's here.\p"
	.string "Typical. Guess we'll make do.$"
HelixIsland_Text_PCIntro_Water:
	.string "This is the little\n"
	.string "'magic sapling'.\p"
	.string "I can't get it to do much,\n"
	.string "but Deoxys said you can.\p"
	.string "Apparently it lets you commune\n"
	.string "with the 'real island'.\p"
	.string "Don't ask me how that works.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "If the current's kind,\n"
	.string "you might wake up to Eggs.\p"
	.string "If it's really kind, they may hatch\n"
	.string "with interesting traits.$"
HelixIsland_Text_Rest_Water:
	.string "That's enough for today.\p"
	.string "Get some sleep.\n"
	.string "Tomorrow, we dive in for real.\p"
	.string "I'll be by the boat.$"
HelixIsland_Text_ComeBack_Water:
	.string "Hey! Ball first.\n"
	.string "Don't drift off.$"
HelixIsland_Text_RemindBall_Water:
	.string "Pick up the ball.\n"
	.string "The tide won't wait forever.$"
HelixIsland_Text_Idle_Water:
	.string "Look around.\n"
	.string "Know your island before you sail.$"
HelixIsland_Text_RestReminder_Water:
	.string "Go sleep.\n"
	.string "Tomorrow's not going easy on us.$"
HelixIsland_Text_ReadyPrompt_Water:
	.string "Ready to head out?$"
HelixIsland_Text_BoatYes_Water:
	.string "Good. Let's make waves.$"
HelixIsland_Text_BoatNo_Water:
	.string "Fine by me.\n"
	.string "Better ready than sorry.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Grass (Erika)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Grass:
	msgbox HelixIsland_Text_Greeting_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Grass:
	msgbox HelixIsland_Text_FollowMe_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Grass:
	msgbox HelixIsland_Text_BallHint_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Grass:
	msgbox HelixIsland_Text_GoodCatch_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Grass:
	msgbox HelixIsland_Text_BoatIntro_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Grass:
	msgbox HelixIsland_Text_ShopsIntro_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Grass:
	msgbox HelixIsland_Text_PCIntro_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Grass:
	msgbox HelixIsland_Text_Rest_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Grass:
	msgbox HelixIsland_Text_ComeBack_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Grass:
	msgbox HelixIsland_Text_RemindBall_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Grass:
	msgbox HelixIsland_Text_Idle_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Grass:
	msgbox HelixIsland_Text_RestReminder_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Grass:
	msgbox HelixIsland_Text_ReadyPrompt_Grass, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Grass:
	msgbox HelixIsland_Text_BoatYes_Grass, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Grass:
	msgbox HelixIsland_Text_BoatNo_Grass, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Grass:
	.string "Welcome.\p"
	.string "I am pleased you arrived unharmed.\p"
	.string "This island is a young garden.\n"
	.string "Let us tend it with care.$"
HelixIsland_Text_FollowMe_Grass:
	.string "Please, follow me.\n"
	.string "There is something by the shore.$"
HelixIsland_Text_BallHint_Grass:
	.string "Do you see that ball?\p"
	.string "A POKéMON has washed ashore.\n"
	.string "The island offers one each morning.\p"
	.string "Please pick it up gently.$"
HelixIsland_Text_GoodCatch_Grass:
	.string "Very good.\p"
	.string "Each morning may bring another\n"
	.string "small blessing from the shore.\p"
	.string "Now, let us continue.$"
HelixIsland_Text_BoatIntro_Grass:
	.string "This boat still needs tending.\p"
	.string "Once it is ready, it will carry us\n"
	.string "to the Nexus.\p"
	.string "The journey will not be gentle.\n"
	.string "We should wait until tomorrow.$"
HelixIsland_Text_ShopsIntro_Grass:
	.string "These storefronts were prepared\n"
	.string "for merchants.\p"
	.string "They were expected some time ago,\n"
	.string "but no one has arrived.\p"
	.string "For now, patience and care\n"
	.string "must sustain us.$"
HelixIsland_Text_PCIntro_Grass:
	.string "This little 'magic sapling'\n"
	.string "has such a curious presence.\p"
	.string "I cannot coax it to answer me,\n"
	.string "but Deoxys said it will answer you.\p"
	.string "Through it, you may commune with\n"
	.string "the 'real island'.\p"
	.string "What a strange and lovely phrase.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "With care, you may wake to Eggs.\p"
	.string "With fortune, those Eggs may hatch\n"
	.string "with beautiful new traits.$"
HelixIsland_Text_Rest_Grass:
	.string "That is enough for today.\p"
	.string "Rest well tonight.\n"
	.string "Tomorrow, our roots reach deeper.\p"
	.string "I will be by the boat.$"
HelixIsland_Text_ComeBack_Grass:
	.string "Please, not yet.\n"
	.string "The ball is over there.$"
HelixIsland_Text_RemindBall_Grass:
	.string "Please pick up the ball.\n"
	.string "I will wait.$"
HelixIsland_Text_Idle_Grass:
	.string "Take your time.\n"
	.string "Every island has a fragrance.$"
HelixIsland_Text_RestReminder_Grass:
	.string "You should rest.\n"
	.string "Growth needs quiet hours.$"
HelixIsland_Text_ReadyPrompt_Grass:
	.string "Are you ready to depart?$"
HelixIsland_Text_BoatYes_Grass:
	.string "Then let us bloom.$"
HelixIsland_Text_BoatNo_Grass:
	.string "Of course.\n"
	.string "A flower opens in its own time.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Electric (Lt. Surge)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Electric:
	msgbox HelixIsland_Text_Greeting_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Electric:
	msgbox HelixIsland_Text_FollowMe_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Electric:
	msgbox HelixIsland_Text_BallHint_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Electric:
	msgbox HelixIsland_Text_GoodCatch_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Electric:
	msgbox HelixIsland_Text_BoatIntro_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Electric:
	msgbox HelixIsland_Text_ShopsIntro_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Electric:
	msgbox HelixIsland_Text_PCIntro_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Electric:
	msgbox HelixIsland_Text_Rest_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Electric:
	msgbox HelixIsland_Text_ComeBack_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Electric:
	msgbox HelixIsland_Text_RemindBall_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Electric:
	msgbox HelixIsland_Text_Idle_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Electric:
	msgbox HelixIsland_Text_RestReminder_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Electric:
	msgbox HelixIsland_Text_ReadyPrompt_Electric, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Electric:
	msgbox HelixIsland_Text_BoatYes_Electric, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Electric:
	msgbox HelixIsland_Text_BoatNo_Electric, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Electric:
	.string "Hey, cadet!\p"
	.string "You made it in one piece.\n"
	.string "Good hustle!\p"
	.string "This island's our base now.\n"
	.string "Time to power it up!$"
HelixIsland_Text_FollowMe_Electric:
	.string "Move out!\n"
	.string "There's something on the shore!$"
HelixIsland_Text_BallHint_Electric:
	.string "See that ball?\p"
	.string "A POKéMON washed up inside.\p"
	.string "Happens every morning,\n"
	.string "like reveille.\p"
	.string "Grab it, cadet!$"
HelixIsland_Text_GoodCatch_Electric:
	.string "That's the spirit!\p"
	.string "A new one hits the shore\n"
	.string "every morning.\p"
	.string "Don't miss free reinforcements!\n"
	.string "Now move!$"
HelixIsland_Text_BoatIntro_Electric:
	.string "That boat's not mission-ready.\p"
	.string "Once it is, we're shipping out\n"
	.string "to the Nexus.\p"
	.string "That's the real battlefield.\n"
	.string "We deploy tomorrow!$"
HelixIsland_Text_ShopsIntro_Electric:
	.string "These storefronts were built\n"
	.string "for merchants.\p"
	.string "They were supposed to report in\n"
	.string "ages ago, but no signal.\p"
	.string "Until then, we run lean.\n"
	.string "No complaints!$"
HelixIsland_Text_PCIntro_Electric:
	.string "This is the little\n"
	.string "'magic sapling'.\p"
	.string "I've barked orders at it.\n"
	.string "No response.\p"
	.string "Deoxys says you can use it\n"
	.string "to commune with the 'real island'.\p"
	.string "Sounds above my pay grade,\n"
	.string "but it's your assignment.\p"
	.string "Send all three of your POKéMON\n"
	.string "over there for now.\p"
	.string "By morning, you may have Eggs.\p"
	.string "With luck, those Eggs hatch\n"
	.string "with high-voltage traits!$"
HelixIsland_Text_Rest_Electric:
	.string "Briefing complete!\p"
	.string "Get some sleep and recharge.\n"
	.string "Tomorrow we hit the Nexus.\p"
	.string "I'll be by the boat, cadet!$"
HelixIsland_Text_ComeBack_Electric:
	.string "Hold it!\n"
	.string "The ball's over there!$"
HelixIsland_Text_RemindBall_Electric:
	.string "Grab that ball, cadet!\n"
	.string "Double time!$"
HelixIsland_Text_Idle_Electric:
	.string "Inspect the base.\n"
	.string "Know your terrain!$"
HelixIsland_Text_RestReminder_Electric:
	.string "Sleep and recharge.\n"
	.string "Big mission tomorrow!$"
HelixIsland_Text_ReadyPrompt_Electric:
	.string "Ready to ship out?$"
HelixIsland_Text_BoatYes_Electric:
	.string "Outstanding! Move out!$"
HelixIsland_Text_BoatNo_Electric:
	.string "Stand down, then.\n"
	.string "Report back when ready.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Psychic (Sabrina)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Psychic:
	msgbox HelixIsland_Text_Greeting_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Psychic:
	msgbox HelixIsland_Text_FollowMe_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Psychic:
	msgbox HelixIsland_Text_BallHint_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Psychic:
	msgbox HelixIsland_Text_GoodCatch_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Psychic:
	msgbox HelixIsland_Text_BoatIntro_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Psychic:
	msgbox HelixIsland_Text_ShopsIntro_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Psychic:
	msgbox HelixIsland_Text_PCIntro_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Psychic:
	msgbox HelixIsland_Text_Rest_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Psychic:
	msgbox HelixIsland_Text_ComeBack_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Psychic:
	msgbox HelixIsland_Text_RemindBall_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Psychic:
	msgbox HelixIsland_Text_Idle_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Psychic:
	msgbox HelixIsland_Text_RestReminder_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Psychic:
	msgbox HelixIsland_Text_ReadyPrompt_Psychic, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Psychic:
	msgbox HelixIsland_Text_BoatYes_Psychic, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Psychic:
	msgbox HelixIsland_Text_BoatNo_Psychic, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Psychic:
	.string "You are here.\p"
	.string "I saw the shape of your arrival\n"
	.string "before the tide did.\p"
	.string "This island is now bound to you.\n"
	.string "Come.$"
HelixIsland_Text_FollowMe_Psychic:
	.string "Follow me.\p"
	.string "The shore has already chosen\n"
	.string "your next step.$"
HelixIsland_Text_BallHint_Psychic:
	.string "That ball.\p"
	.string "A POKéMON washed ashore within it.\n"
	.string "It will happen each morning.\p"
	.string "Pick it up.$"
HelixIsland_Text_GoodCatch_Psychic:
	.string "Yes.\p"
	.string "Another will appear with morning.\p"
	.string "Do not ignore what fate places\n"
	.string "before you.\p"
	.string "Now, this way.$"
HelixIsland_Text_BoatIntro_Psychic:
	.string "The boat is incomplete.\p"
	.string "When it is ready, it will take us\n"
	.string "to the Nexus.\p"
	.string "Beyond it, futures split and vanish.\n"
	.string "We wait until tomorrow.$"
HelixIsland_Text_ShopsIntro_Psychic:
	.string "These storefronts are empty.\p"
	.string "Merchants were expected,\n"
	.string "but their paths have gone silent.\p"
	.string "For now, absence is part of\n"
	.string "the lesson.$"
HelixIsland_Text_PCIntro_Psychic:
	.string "This is the 'magic sapling'.\p"
	.string "It thinks in roots.\n"
	.string "It does not think for me.\p"
	.string "Deoxys says your mind can commune\n"
	.string "with the 'real island'.\p"
	.string "The words are unclear,\n"
	.string "but the pull is real.\p"
	.string "Send all three of your POKéMON\n"
	.string "over there.\p"
	.string "When you wake, Eggs may be waiting.\p"
	.string "Some may hatch carrying traits\n"
	.string "that were only possibilities.$"
HelixIsland_Text_Rest_Psychic:
	.string "The first vision ends here.\p"
	.string "Sleep.\n"
	.string "Tomorrow, the path sharpens.\p"
	.string "I will wait by the boat.$"
HelixIsland_Text_ComeBack_Psychic:
	.string "No.\n"
	.string "The ball is the next step.$"
HelixIsland_Text_RemindBall_Psychic:
	.string "Pick up the ball.\n"
	.string "It is already part of your path.$"
HelixIsland_Text_Idle_Psychic:
	.string "Look closely.\n"
	.string "Some things look back.$"
HelixIsland_Text_RestReminder_Psychic:
	.string "Sleep.\n"
	.string "Dreams may prepare you.$"
HelixIsland_Text_ReadyPrompt_Psychic:
	.string "Are you ready for the Nexus?$"
HelixIsland_Text_BoatYes_Psychic:
	.string "Then the path opens.$"
HelixIsland_Text_BoatNo_Psychic:
	.string "Then wait.\n"
	.string "The path remains.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Ice (Lorelei)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Ice:
	msgbox HelixIsland_Text_Greeting_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Ice:
	msgbox HelixIsland_Text_FollowMe_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Ice:
	msgbox HelixIsland_Text_BallHint_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Ice:
	msgbox HelixIsland_Text_GoodCatch_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Ice:
	msgbox HelixIsland_Text_BoatIntro_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Ice:
	msgbox HelixIsland_Text_ShopsIntro_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Ice:
	msgbox HelixIsland_Text_PCIntro_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Ice:
	msgbox HelixIsland_Text_Rest_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Ice:
	msgbox HelixIsland_Text_ComeBack_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Ice:
	msgbox HelixIsland_Text_RemindBall_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Ice:
	msgbox HelixIsland_Text_Idle_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Ice:
	msgbox HelixIsland_Text_RestReminder_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Ice:
	msgbox HelixIsland_Text_ReadyPrompt_Ice, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Ice:
	msgbox HelixIsland_Text_BoatYes_Ice, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Ice:
	msgbox HelixIsland_Text_BoatNo_Ice, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Ice:
	.string "You made it.\p"
	.string "Good. Panic wastes energy,\n"
	.string "and you seem composed.\p"
	.string "This island is our position now.\n"
	.string "We will secure it carefully.$"
HelixIsland_Text_FollowMe_Ice:
	.string "Follow me.\n"
	.string "There is something on the shore.$"
HelixIsland_Text_BallHint_Ice:
	.string "See that ball?\p"
	.string "A POKéMON has washed ashore.\n"
	.string "This occurs once each morning.\p"
	.string "Pick it up.\n"
	.string "No need to overthink it.$"
HelixIsland_Text_GoodCatch_Ice:
	.string "Good.\p"
	.string "Each morning brings another chance.\n"
	.string "Cold judgment means taking it.\p"
	.string "Now, this way.$"
HelixIsland_Text_BoatIntro_Ice:
	.string "The boat remains unfinished.\p"
	.string "When repaired, it will take us\n"
	.string "to the Nexus.\p"
	.string "That place will punish mistakes.\n"
	.string "We begin tomorrow, prepared.$"
HelixIsland_Text_ShopsIntro_Ice:
	.string "These storefronts were reserved\n"
	.string "for merchants.\p"
	.string "They were expected already,\n"
	.string "but none have arrived.\p"
	.string "We adjust.\n"
	.string "That is how one survives.$"
HelixIsland_Text_PCIntro_Ice:
	.string "This little 'magic sapling'\n"
	.string "is difficult to classify.\p"
	.string "I cannot operate it.\n"
	.string "Deoxys says you can.\p"
	.string "It may let you commune with the\n"
	.string "'real island', whatever that means.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "By morning, you may find Eggs.\p"
	.string "If conditions are favorable,\n"
	.string "they may hatch with useful traits.\p"
	.string "Record the results in your head.$"
HelixIsland_Text_Rest_Ice:
	.string "That is sufficient for today.\p"
	.string "Rest, then.\n"
	.string "Tomorrow we proceed in earnest.\p"
	.string "I will wait by the boat.$"
HelixIsland_Text_ComeBack_Ice:
	.string "Stop.\n"
	.string "The ball is over there.$"
HelixIsland_Text_RemindBall_Ice:
	.string "Pick up the ball.\n"
	.string "Then we continue.$"
HelixIsland_Text_Idle_Ice:
	.string "Study the island calmly.\n"
	.string "Emotion clouds the details.$"
HelixIsland_Text_RestReminder_Ice:
	.string "Sleep.\n"
	.string "Fatigue makes fools of Trainers.$"
HelixIsland_Text_ReadyPrompt_Ice:
	.string "Ready to proceed to the Nexus?$"
HelixIsland_Text_BoatYes_Ice:
	.string "Good. Keep your head cool.$"
HelixIsland_Text_BoatNo_Ice:
	.string "Acceptable.\n"
	.string "Return when prepared.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Dragon (Lance)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Dragon:
	msgbox HelixIsland_Text_Greeting_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Dragon:
	msgbox HelixIsland_Text_FollowMe_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Dragon:
	msgbox HelixIsland_Text_BallHint_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Dragon:
	msgbox HelixIsland_Text_GoodCatch_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Dragon:
	msgbox HelixIsland_Text_BoatIntro_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Dragon:
	msgbox HelixIsland_Text_ShopsIntro_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Dragon:
	msgbox HelixIsland_Text_PCIntro_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Dragon:
	msgbox HelixIsland_Text_Rest_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Dragon:
	msgbox HelixIsland_Text_ComeBack_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Dragon:
	msgbox HelixIsland_Text_RemindBall_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Dragon:
	msgbox HelixIsland_Text_Idle_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Dragon:
	msgbox HelixIsland_Text_RestReminder_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Dragon:
	msgbox HelixIsland_Text_ReadyPrompt_Dragon, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Dragon:
	msgbox HelixIsland_Text_BoatYes_Dragon, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Dragon:
	msgbox HelixIsland_Text_BoatNo_Dragon, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Dragon:
	.string "Ah! You have arrived.\p"
	.string "Good. I sensed a strong spirit\n"
	.string "when Deoxys chose you.\p"
	.string "This island is your domain now.\n"
	.string "Let us raise it with pride.$"
HelixIsland_Text_FollowMe_Dragon:
	.string "Come.\n"
	.string "The shore holds your first ally.$"
HelixIsland_Text_BallHint_Dragon:
	.string "See that ball?\p"
	.string "A POKéMON washed ashore.\n"
	.string "Such a gift comes every morning.\p"
	.string "Go. Welcome it to your cause.$"
HelixIsland_Text_GoodCatch_Dragon:
	.string "Well done!\p"
	.string "Each morning may bring a new ally.\n"
	.string "Treasure every bond you forge.\p"
	.string "Now, onward!$"
HelixIsland_Text_BoatIntro_Dragon:
	.string "This boat still needs work.\p"
	.string "When ready, it will carry us\n"
	.string "to the Nexus.\p"
	.string "There, courage and strength\n"
	.string "will be tested.\p"
	.string "Tomorrow, we begin that challenge.$"
HelixIsland_Text_ShopsIntro_Dragon:
	.string "These storefronts await merchants.\p"
	.string "They were meant to arrive by now,\n"
	.string "but the island has heard nothing.\p"
	.string "Then we will stand without them\n"
	.string "until they come.$"
HelixIsland_Text_PCIntro_Dragon:
	.string "This is the little\n"
	.string "'magic sapling'.\p"
	.string "Even dragons would hesitate before\n"
	.string "such a strange power.\p"
	.string "Deoxys says you can commune with\n"
	.string "the 'real island' through it.\p"
	.string "I do not yet know what that means,\n"
	.string "but I believe it matters.\p"
	.string "Send all three of your POKéMON\n"
	.string "over there for now.\p"
	.string "By morning, Eggs may await you.\p"
	.string "Those Eggs may hatch with traits\n"
	.string "worthy of future champions.$"
HelixIsland_Text_Rest_Dragon:
	.string "That is enough for today.\p"
	.string "Rest well.\n"
	.string "Tomorrow, your legend begins.\p"
	.string "I will be by the boat.$"
HelixIsland_Text_ComeBack_Dragon:
	.string "Hold!\n"
	.string "Your ally waits in that ball.$"
HelixIsland_Text_RemindBall_Dragon:
	.string "Go. Claim the ball.\n"
	.string "Every bond begins somewhere.$"
HelixIsland_Text_Idle_Dragon:
	.string "Look around.\n"
	.string "A champion knows what they defend.$"
HelixIsland_Text_RestReminder_Dragon:
	.string "Rest now.\n"
	.string "Courage needs strength behind it.$"
HelixIsland_Text_ReadyPrompt_Dragon:
	.string "Ready to face the Nexus?$"
HelixIsland_Text_BoatYes_Dragon:
	.string "Then let us go, with pride!$"
HelixIsland_Text_BoatNo_Dragon:
	.string "Very well.\n"
	.string "A true challenge deserves readiness.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Dark (Sidney)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Dark:
	msgbox HelixIsland_Text_Greeting_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Dark:
	msgbox HelixIsland_Text_FollowMe_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Dark:
	msgbox HelixIsland_Text_BallHint_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Dark:
	msgbox HelixIsland_Text_GoodCatch_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Dark:
	msgbox HelixIsland_Text_BoatIntro_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Dark:
	msgbox HelixIsland_Text_ShopsIntro_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Dark:
	msgbox HelixIsland_Text_PCIntro_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Dark:
	msgbox HelixIsland_Text_Rest_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Dark:
	msgbox HelixIsland_Text_ComeBack_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Dark:
	msgbox HelixIsland_Text_RemindBall_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Dark:
	msgbox HelixIsland_Text_Idle_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Dark:
	msgbox HelixIsland_Text_RestReminder_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Dark:
	msgbox HelixIsland_Text_ReadyPrompt_Dark, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Dark:
	msgbox HelixIsland_Text_BoatYes_Dark, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Dark:
	msgbox HelixIsland_Text_BoatNo_Dark, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Dark:
	.string "Hey, there you are!\p"
	.string "Made it through the weird part, huh?\p"
	.string "Good. This island's ours now,\n"
	.string "so let's make the best of it.$"
HelixIsland_Text_FollowMe_Dark:
	.string "Come on.\p"
	.string "There's something on the shore\n"
	.string "that's worth your time.$"
HelixIsland_Text_BallHint_Dark:
	.string "See that ball?\p"
	.string "A POKéMON washed up in it.\n"
	.string "Happens every morning here.\p"
	.string "Pretty generous, right?\n"
	.string "Go pick it up.$"
HelixIsland_Text_GoodCatch_Dark:
	.string "Nice!\p"
	.string "A new one can wash up every morning.\p"
	.string "Around here, you take\n"
	.string "the good breaks.\p"
	.string "when they show up. Now, this way.$"
HelixIsland_Text_BoatIntro_Dark:
	.string "That boat still needs fixing.\p"
	.string "Once it's ready, we ride out\n"
	.string "to the Nexus.\p"
	.string "That's where things get rough\n"
	.string "in the fun way.\p"
	.string "Tomorrow, though. No need to trip\n"
	.string "before the first punch.$"
HelixIsland_Text_ShopsIntro_Dark:
	.string "These storefronts were supposed\n"
	.string "to have merchants.\p"
	.string "They were due a while back,\n"
	.string "but nobody's shown.\p"
	.string "Heh. Guess we're not the only ones\n"
	.string "with bad directions.$"
HelixIsland_Text_PCIntro_Dark:
	.string "This is our little\n"
	.string "'magic sapling'.\p"
	.string "I tried acting casual around it.\n"
	.string "Still no clue how it works.\p"
	.string "Deoxys says you can commune with\n"
	.string "the 'real island' through it.\p"
	.string "Sounds shady. I like it.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "Maybe you'll wake up to Eggs.\p"
	.string "Maybe those Eggs hatch with\n"
	.string "some nasty little traits.\p"
	.string "That's the fun part.$"
HelixIsland_Text_Rest_Dark:
	.string "That's the tour.\p"
	.string "Get some sleep tonight.\n"
	.string "Tomorrow, we start causing trouble.\p"
	.string "I'll be by the boat.$"
HelixIsland_Text_ComeBack_Dark:
	.string "Hey, hey.\n"
	.string "Ball first, wandering later.$"
HelixIsland_Text_RemindBall_Dark:
	.string "Go pick up the ball.\n"
	.string "Don't make it awkward.$"
HelixIsland_Text_Idle_Dark:
	.string "Look around.\n"
	.string "This place has secrets. Good ones.$"
HelixIsland_Text_RestReminder_Dark:
	.string "Get some sleep.\n"
	.string "Trouble's better when rested.$"
HelixIsland_Text_ReadyPrompt_Dark:
	.string "Ready to head out?$"
HelixIsland_Text_BoatYes_Dark:
	.string "Heh. Now we're talking.$"
HelixIsland_Text_BoatNo_Dark:
	.string "No sweat.\p"
	.string "I'll be here when you're\n"
	.string "feeling bold.$"

@ ══════════════════════════════════════════════════════════════════════
@ PER-TYPE HANDLERS + TEXT — Fairy (Wally)
@ ══════════════════════════════════════════════════════════════════════
HelixIsland_GuideMsg_Greeting_Fairy:
	msgbox HelixIsland_Text_Greeting_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_FollowMe_Fairy:
	msgbox HelixIsland_Text_FollowMe_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BallHint_Fairy:
	msgbox HelixIsland_Text_BallHint_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_GoodCatch_Fairy:
	msgbox HelixIsland_Text_GoodCatch_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatIntro_Fairy:
	msgbox HelixIsland_Text_BoatIntro_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ShopsIntro_Fairy:
	msgbox HelixIsland_Text_ShopsIntro_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_PCIntro_Fairy:
	msgbox HelixIsland_Text_PCIntro_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Rest_Fairy:
	msgbox HelixIsland_Text_Rest_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ComeBack_Fairy:
	msgbox HelixIsland_Text_ComeBack_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RemindBall_Fairy:
	msgbox HelixIsland_Text_RemindBall_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_Idle_Fairy:
	msgbox HelixIsland_Text_Idle_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_RestReminder_Fairy:
	msgbox HelixIsland_Text_RestReminder_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_ReadyPrompt_Fairy:
	msgbox HelixIsland_Text_ReadyPrompt_Fairy, MSGBOX_YESNO
	return
HelixIsland_GuideMsg_BoatYes_Fairy:
	msgbox HelixIsland_Text_BoatYes_Fairy, MSGBOX_DEFAULT
	return
HelixIsland_GuideMsg_BoatNo_Fairy:
	msgbox HelixIsland_Text_BoatNo_Fairy, MSGBOX_DEFAULT
	return

HelixIsland_Text_Greeting_Fairy:
	.string "Oh! You made it.\p"
	.string "I'm really glad.\p"
	.string "This island feels strange,\n"
	.string "but... kind, somehow.\p"
	.string "Let's take care of it together.$"
HelixIsland_Text_FollowMe_Fairy:
	.string "Please follow me.\p"
	.string "There's something by the shore\n"
	.string "I want to show you.$"
HelixIsland_Text_BallHint_Fairy:
	.string "See that ball?\p"
	.string "A POKéMON washed up inside it.\n"
	.string "It seems to happen every morning.\p"
	.string "Go ahead and pick it up.\n"
	.string "Gently, if you can.$"
HelixIsland_Text_GoodCatch_Fairy:
	.string "You did it.\p"
	.string "Every morning, another one may\n"
	.string "wash up here.\p"
	.string "I think each one matters.\n"
	.string "Now, let's keep going.$"
HelixIsland_Text_BoatIntro_Fairy:
	.string "That boat still needs work.\p"
	.string "Once it's ready, it can take us\n"
	.string "to the Nexus.\p"
	.string "I'm nervous too, but that's where\n"
	.string "we have to grow.\p"
	.string "We'll start tomorrow.$"
HelixIsland_Text_ShopsIntro_Fairy:
	.string "These storefronts were set up\n"
	.string "for merchants.\p"
	.string "They were supposed to arrive,\n"
	.string "but no one has yet.\p"
	.string "I hope they're okay.\n"
	.string "For now, we'll manage.$"
HelixIsland_Text_PCIntro_Fairy:
	.string "This little 'magic sapling'\n"
	.string "feels... warm.\p"
	.string "I can't really use it,\n"
	.string "but Deoxys said you could.\p"
	.string "It lets you commune with the\n"
	.string "'real island'.\p"
	.string "I don't know what that means yet,\n"
	.string "but I believe it's important.\p"
	.string "For now, send all three of your\n"
	.string "POKéMON over there.\p"
	.string "If they're comfortable,\n"
	.string "you might wake up to Eggs.\p"
	.string "And those Eggs might hatch\n"
	.string "with wonderful new traits.\p"
	.string "That's how we all grow, right?$"
HelixIsland_Text_Rest_Fairy:
	.string "That's everything for today.\p"
	.string "Please get some rest.\n"
	.string "Tomorrow, we'll try for real.\p"
	.string "I'll be by the boat in the morning.$"
HelixIsland_Text_ComeBack_Fairy:
	.string "Wait.\n"
	.string "The ball is over there.$"
HelixIsland_Text_RemindBall_Fairy:
	.string "Please pick up the ball.\n"
	.string "I'll wait right here.$"
HelixIsland_Text_Idle_Fairy:
	.string "Take your time.\n"
	.string "This place is new to both of us.$"
HelixIsland_Text_RestReminder_Fairy:
	.string "You should sleep.\n"
	.string "Tomorrow will take courage.$"
HelixIsland_Text_ReadyPrompt_Fairy:
	.string "Are you ready to go?$"
HelixIsland_Text_BoatYes_Fairy:
	.string "Okay. Let's be brave.$"
HelixIsland_Text_BoatNo_Fairy:
	.string "That's okay.\n"
	.string "We'll go when you're ready.$"

HelixIsland_Text_DevKitOffer:
	.string "DEV KIT: Want a level 100 NUZLEAF\n"
	.string "with FALSE SWIPE and LEAF BLADE?$"
HelixIsland_Text_DevKitParty:
	.string "DEV KIT: NUZLEAF was added\n"
	.string "to your party.$"
HelixIsland_Text_DevKitPC:
	.string "DEV KIT: Your party is full.\n"
	.string "NUZLEAF was sent to the PC.$"
HelixIsland_Text_DevKitNoRoom:
	.string "DEV KIT: No room in party or PC.$"
HelixIsland_Text_DevKitDecline:
	.string "DEV KIT: Come back anytime.$"

