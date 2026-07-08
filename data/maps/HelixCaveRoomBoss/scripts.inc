HelixCaveRoomBoss_MapScripts::
	.byte 0

@ ══════════════════════════════════════════════════════════════════════
@ REGIROCK ENDPOINT
@ ══════════════════════════════════════════════════════════════════════
HelixCaveRoomBoss_EventScript_Regirock::
	lock
	faceplayer
	msgbox HelixCaveRoomBoss_Text_Awakens, MSGBOX_DEFAULT
	closemessage
	waitse
	playmoncry SPECIES_REGIROCK, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setwildbattle SPECIES_REGIROCK, 5
	@ Totem-style aura: +1 Defense at battle start (with in-battle message)
	totemboost_def1 B_POSITION_OPPONENT_LEFT
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, HelixCaveRoomBoss_EventScript_Victory
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, HelixCaveRoomBoss_EventScript_Victory
	@ Fled or otherwise inconclusive — Regirock stands unmoved
	msgbox HelixCaveRoomBoss_Text_Unmoved, MSGBOX_DEFAULT
	release
	end

HelixCaveRoomBoss_EventScript_Victory:
	removeobject LOCALID_HELIX_CAVE_ROOM_REGIROCK
	msgbox HelixCaveRoomBoss_Text_FallsSilent, MSGBOX_DEFAULT
	@ Endpoint reward, then bank + retire; summary is delivered by the
	@ guide on the island beach (HelixIsland_EventScript_RunSuccessReturn)
	special HelixSpecial_RollEndpointReward
	giveitem VAR_HELIX_RUN_ENDPOINT_REWARD
	special HelixSpecial_CompleteCavesRun
	closemessage
	warp MAP_HELIX_ISLAND, 28, 21
	waitstate
	release
	end

@ ══════════════════════════════════════════════════════════════════════
@ TEXT
@ ══════════════════════════════════════════════════════════════════════

HelixCaveRoomBoss_Text_Awakens:
	.string "The stone giant stirs.\n"
	.string "The whole chamber trembles!$"

HelixCaveRoomBoss_Text_Unmoved:
	.string "REGIROCK stands unmoved,\n"
	.string "blocking the heart of the cave.$"

HelixCaveRoomBoss_Text_FallsSilent:
	.string "The chamber falls silent.\p"
	.string "Something glitters where the\n"
	.string "giant stood.$"

