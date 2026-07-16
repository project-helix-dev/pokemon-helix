HelixCaveRoomTrainer_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, HelixCaveRoomTrainer_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, HelixCaveRoomTrainer_OnFrame
	.byte 0

@ Pick the variant's sprite (VAR_OBJ_GFX_ID_0) and allow rematches, since
@ the same trainer ids are reused across rooms and runs.
HelixCaveRoomTrainer_OnTransition:
	special HelixSpecial_SetupRunTrainerGfx
	cleartrainerflag TRAINER_HELIX_CAVE_HIKER
	cleartrainerflag TRAINER_HELIX_CAVE_CAMPER
	cleartrainerflag TRAINER_HELIX_CAVE_NERD
	cleartrainerflag TRAINER_HELIX_CAVE_OLD_MAN
	end

HelixCaveRoomTrainer_OnFrame:
	map_script_2 VAR_TEMP_4, 0, HelixCaveRoomTrainer_EventScript_Challenge
	.2byte 0

HelixCaveRoomTrainer_EventScript_Challenge::
	lockall
	setvar VAR_TEMP_4, 1
	goto_if_eq VAR_HELIX_RUN_ROOM_COMPLETE, 1, HelixCaveRoomTrainer_EventScript_AlreadyDone
	applymovement LOCALID_HELIX_CAVE_ROOM_TRAINER, HelixRun_Movement_Exclamation
	waitmovement 0
	switch VAR_HELIX_RUN_TRAINER_VARIANT
	case HELIX_RUN_TRAINER_CAMPER, HelixCaveRoomTrainer_EventScript_BattleCamper
	case HELIX_RUN_TRAINER_NERD, HelixCaveRoomTrainer_EventScript_BattleNerd
	case HELIX_RUN_TRAINER_OLD_MAN, HelixCaveRoomTrainer_EventScript_BattleOldMan
	@ default: Hiker
	msgbox HelixCaveRoomTrainer_Text_HikerIntro, MSGBOX_DEFAULT
	closemessage
	trainerbattle_no_intro TRAINER_HELIX_CAVE_HIKER, HelixCaveRoomTrainer_Text_HikerDefeat
	goto HelixCaveRoomTrainer_EventScript_PostBattle

HelixCaveRoomTrainer_EventScript_BattleCamper:
	msgbox HelixCaveRoomTrainer_Text_CamperIntro, MSGBOX_DEFAULT
	closemessage
	trainerbattle_no_intro TRAINER_HELIX_CAVE_CAMPER, HelixCaveRoomTrainer_Text_CamperDefeat
	goto HelixCaveRoomTrainer_EventScript_PostBattle

HelixCaveRoomTrainer_EventScript_BattleNerd:
	msgbox HelixCaveRoomTrainer_Text_NerdIntro, MSGBOX_DEFAULT
	closemessage
	trainerbattle_no_intro TRAINER_HELIX_CAVE_NERD, HelixCaveRoomTrainer_Text_NerdDefeat
	goto HelixCaveRoomTrainer_EventScript_PostBattle

HelixCaveRoomTrainer_EventScript_BattleOldMan:
	msgbox HelixCaveRoomTrainer_Text_OldManIntro, MSGBOX_DEFAULT
	closemessage
	trainerbattle_no_intro TRAINER_HELIX_CAVE_OLD_MAN, HelixCaveRoomTrainer_Text_OldManDefeat
	goto HelixCaveRoomTrainer_EventScript_PostBattle

HelixCaveRoomTrainer_EventScript_PostBattle:
	special HelixSpecial_OnRunTrainerWon
	copyvar VAR_0x8009, VAR_RESULT
	call HelixRun_EventScript_ReportLostMons
	@ IV reward is granted inside the battle (BS_HelixTryEndBattleRewards)
	setvar VAR_HELIX_RUN_ROOM_COMPLETE, 1
	msgbox HelixRun_Text_PathOpen, MSGBOX_DEFAULT
	releaseall
	end

HelixCaveRoomTrainer_EventScript_AlreadyDone::
	releaseall
	end

@ Talking to the trainer after the battle
HelixCaveRoomTrainer_EventScript_Trainer::
	lock
	faceplayer
	msgbox HelixCaveRoomTrainer_Text_PostBattleChat, MSGBOX_DEFAULT
	release
	end

@ ══════════════════════════════════════════════════════════════════════
@ TEXT — sayings grouped by trainer class
@ ══════════════════════════════════════════════════════════════════════

HelixCaveRoomTrainer_Text_HikerIntro:
	.string "The cave keeps what it wants!$"

HelixCaveRoomTrainer_Text_HikerDefeat:
	.string "A good climb starts underground!$"

HelixCaveRoomTrainer_Text_CamperIntro:
	.string "I thought this trail looked safe!$"

HelixCaveRoomTrainer_Text_CamperDefeat:
	.string "Nothing like a battle under\n"
	.string "the rocks!$"

HelixCaveRoomTrainer_Text_NerdIntro:
	.string "A closed ecosystem inside a cave?\n"
	.string "Fascinating!$"

HelixCaveRoomTrainer_Text_NerdDefeat:
	.string "Your specimens are developing\n"
	.string "nicely.$"

HelixCaveRoomTrainer_Text_OldManIntro:
	.string "These caves remember more\n"
	.string "than we do.$"

HelixCaveRoomTrainer_Text_OldManDefeat:
	.string "Slow steps still reach the summit.$"

HelixCaveRoomTrainer_Text_PostBattleChat:
	.string "The route ahead is open.\n"
	.string "Good luck out there.$"
