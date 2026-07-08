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
	@ Endpoint reward, then summary, then bank + retire + warp home
	special HelixSpecial_RollEndpointReward
	giveitem VAR_HELIX_RUN_ENDPOINT_REWARD
	special HelixSpecial_BufferRunSummary
	bufferitemname STR_VAR_3, VAR_HELIX_RUN_ENDPOINT_REWARD
	msgbox HelixCaveRoomBoss_Text_Summary, MSGBOX_DEFAULT
	special HelixSpecial_CompleteCavesRun
	msgbox HelixCaveRoomBoss_Text_Retired, MSGBOX_DEFAULT
	closemessage
	warp MAP_HELIX_NEXUS, 5, 1
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

HelixCaveRoomBoss_Text_Summary:
	.string "Expedition complete!\p"
	.string "Money earned: ¥{STR_VAR_1}.\n"
	.string "Food gathered: {STR_VAR_2}.\p"
	.string "Endpoint prize: {STR_VAR_3}.$"

HelixCaveRoomBoss_Text_Retired:
	.string "Your surviving POKéMON have\n"
	.string "earned their retirement.\p"
	.string "They won't set out again, but\n"
	.string "the island will be glad to\l"
	.string "have them home.$"
