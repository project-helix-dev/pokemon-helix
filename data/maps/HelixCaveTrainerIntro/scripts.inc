HelixCaveTrainerIntro_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, HelixCaveTrainerIntro_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, HelixCaveTrainerIntro_OnFrame
	.byte 0

HelixCaveTrainerIntro_OnTransition:
	cleartrainerflag TRAINER_HELIX_CAVE_INTRO
	end

@ The Hiker challenges the player the moment they walk in.
HelixCaveTrainerIntro_OnFrame:
	map_script_2 VAR_TEMP_4, 0, HelixCaveTrainerIntro_EventScript_Challenge
	.2byte 0

HelixCaveTrainerIntro_EventScript_Challenge::
	lockall
	setvar VAR_TEMP_4, 1
	goto_if_eq VAR_HELIX_RUN_ROOM_COMPLETE, 1, HelixCaveTrainerIntro_EventScript_AlreadyDone
	applymovement LOCALID_HELIX_CAVE_INTRO_TRAINER, HelixRun_Movement_Exclamation
	waitmovement 0
	msgbox HelixCaveTrainerIntro_Text_Challenge, MSGBOX_DEFAULT
	closemessage
	trainerbattle_no_intro TRAINER_HELIX_CAVE_INTRO, HelixCaveTrainerIntro_Text_Defeat
	special HelixSpecial_OnRunTrainerWon
	copyvar VAR_0x8009, VAR_RESULT
	call HelixRun_EventScript_ReportLostMons
	msgbox HelixCaveTrainerIntro_Text_PostBattle, MSGBOX_DEFAULT
	call HelixRun_EventScript_TryIVReward
	setvar VAR_HELIX_RUN_ROOM_COMPLETE, 1
	msgbox HelixRun_Text_PathOpen, MSGBOX_DEFAULT
	releaseall
	end

HelixCaveTrainerIntro_EventScript_AlreadyDone::
	releaseall
	end

@ Talking to the Hiker after the battle
HelixCaveTrainerIntro_EventScript_Trainer::
	lock
	faceplayer
	msgbox HelixCaveTrainerIntro_Text_PostBattle, MSGBOX_DEFAULT
	release
	end

@ ══════════════════════════════════════════════════════════════════════
@ TEXT
@ ══════════════════════════════════════════════════════════════════════

HelixCaveTrainerIntro_Text_Challenge:
	.string "You won't reach the summit by\n"
	.string "standing still!$"

HelixCaveTrainerIntro_Text_Defeat:
	.string "Hah! Not bad at all!$"

HelixCaveTrainerIntro_Text_PostBattle:
	.string "Hah! The cave likes you better.\p"
	.string "Keep moving. The route only gets\n"
	.string "stranger from here.$"
