@ ══════════════════════════════════════════════════════════════════════
@ Helix expedition (Caves run) — shared scripts
@ Exit-door validation, room progression, IV reward flow, common text.
@ See docs/helix_nexus_run_level_system_brief_final.md
@ ══════════════════════════════════════════════════════════════════════

@ ── Exit doors ────────────────────────────────────────────────────────
@ Coord events on the door gap tiles. VAR_0x8008 = door direction.

HelixRun_EventScript_ExitDoorUp::
	lockall
	setvar VAR_0x8008, HELIX_RUN_DIR_UP
	goto HelixRun_EventScript_ExitDoorCommon

HelixRun_EventScript_ExitDoorLeft::
	lockall
	setvar VAR_0x8008, HELIX_RUN_DIR_LEFT
	goto HelixRun_EventScript_ExitDoorCommon

HelixRun_EventScript_ExitDoorRight::
	lockall
	setvar VAR_0x8008, HELIX_RUN_DIR_RIGHT
	goto HelixRun_EventScript_ExitDoorCommon

HelixRun_EventScript_ExitDoorCommon:
	goto_if_eq VAR_HELIX_RUN_ROOM_COMPLETE, 0, HelixRun_EventScript_DoorBlocked
	compare VAR_HELIX_RUN_EXIT_DIR, VAR_0x8008
	goto_if_ne HelixRun_EventScript_DoorSealed
	@ Advance the run: roll the next room and warp to its map
	special HelixSpecial_RollNextCaveRoom
	switch VAR_RESULT
	case HELIX_RUN_ROOM_WILD, HelixRun_EventScript_WarpWild
	case HELIX_RUN_ROOM_TRAINER, HelixRun_EventScript_WarpTrainer
	case HELIX_RUN_ROOM_SHOP, HelixRun_EventScript_WarpShop
	case HELIX_RUN_ROOM_BOSS, HelixRun_EventScript_WarpBoss
	goto HelixRun_EventScript_DoorBlocked

HelixRun_EventScript_WarpWild:
	warp MAP_HELIX_CAVE_ROOM_WILD, 5, 7
	waitstate
	releaseall
	end

HelixRun_EventScript_WarpTrainer:
	warp MAP_HELIX_CAVE_ROOM_TRAINER, 5, 7
	waitstate
	releaseall
	end

HelixRun_EventScript_WarpShop:
	warp MAP_HELIX_CAVE_ROOM_SHOP, 5, 7
	waitstate
	releaseall
	end

HelixRun_EventScript_WarpBoss:
	warp MAP_HELIX_CAVE_ROOM_BOSS, 5, 7
	waitstate
	releaseall
	end

HelixRun_EventScript_DoorBlocked:
	msgbox HelixRun_Text_DoorBlocked, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

HelixRun_EventScript_DoorSealed:
	msgbox HelixRun_Text_DoorSealed, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

@ Step the player back off the door tile (direction depends on the door)
HelixRun_EventScript_BounceBack:
	closemessage
	switch VAR_0x8008
	case HELIX_RUN_DIR_LEFT, HelixRun_EventScript_BounceRight
	case HELIX_RUN_DIR_RIGHT, HelixRun_EventScript_BounceLeft
	@ default: top door → step back down
	applymovement OBJ_EVENT_ID_PLAYER, HelixRun_Movement_BounceDown
	waitmovement 0
	releaseall
	end

HelixRun_EventScript_BounceRight:
	applymovement OBJ_EVENT_ID_PLAYER, HelixRun_Movement_BounceRight
	waitmovement 0
	releaseall
	end

HelixRun_EventScript_BounceLeft:
	applymovement OBJ_EVENT_ID_PLAYER, HelixRun_Movement_BounceLeft
	waitmovement 0
	releaseall
	end

@ ── Boss room has no onward exits ─────────────────────────────────────

HelixRun_EventScript_BossNoExit::
	lockall
	setvar VAR_0x8008, HELIX_RUN_DIR_UP
	msgbox HelixRun_Text_OnlyRockBeyond, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

HelixRun_EventScript_BossNoExitLeft::
	lockall
	setvar VAR_0x8008, HELIX_RUN_DIR_LEFT
	msgbox HelixRun_Text_OnlyRockBeyond, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

HelixRun_EventScript_BossNoExitRight::
	lockall
	setvar VAR_0x8008, HELIX_RUN_DIR_RIGHT
	msgbox HelixRun_Text_OnlyRockBeyond, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

@ ── Post-battle attrition report (use with `call`) ────────────────────
@ VAR_0x8009 must hold the number of party mons lost.

HelixRun_EventScript_ReportLostMons::
	goto_if_eq VAR_0x8009, 0, HelixRun_EventScript_ReportLostMons_End
	msgbox HelixRun_Text_MonsLost, MSGBOX_DEFAULT
	return

HelixRun_EventScript_ReportLostMons_End::
	return

@ ── Movement ──────────────────────────────────────────────────────────

HelixRun_Movement_BounceDown:
	walk_down
	step_end

HelixRun_Movement_BounceRight:
	walk_right
	step_end

HelixRun_Movement_BounceLeft:
	walk_left
	step_end

HelixRun_Movement_Exclamation:
	emote_exclamation_mark
	delay_16
	step_end

@ ── Shared text ───────────────────────────────────────────────────────

HelixRun_Text_DoorBlocked:
	.string "The way ahead is sealed...\n"
	.string "This room's trial isn't finished.$"

HelixRun_Text_DoorSealed:
	.string "Solid rock blocks this passage.\n"
	.string "The route continues elsewhere.$"

HelixRun_Text_OnlyRockBeyond:
	.string "There is nothing but solid rock\n"
	.string "beyond this chamber.$"

HelixRun_Text_PathOpen:
	.string "Somewhere in the cave, stone\n"
	.string "grinds aside. The path is open!$"

HelixRun_Text_MonsLost:
	.string "Some of your POKéMON were lost\n"
	.string "to the expedition...$"
