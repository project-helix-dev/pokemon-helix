@ Wild encounter room. Battle outcomes are recorded by HelixOnWildBattleEnd()
@ in src/helix_run.c, which signals this map through temp vars:
@   VAR_TEMP_1 = 1 when a wild battle was just resolved
@   VAR_TEMP_2 = food gathered by the KO (0 if none)
@   VAR_TEMP_3 = party mons lost to the battle
@   VAR_TEMP_5 = 1 once the entry message has been shown

HelixCaveRoomWild_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, HelixCaveRoomWild_OnFrame
	.byte 0

HelixCaveRoomWild_OnFrame:
	map_script_2 VAR_TEMP_5, 0, HelixCaveRoomWild_EventScript_Entry
	map_script_2 VAR_TEMP_1, 1, HelixCaveRoomWild_EventScript_BattleResolved
	.2byte 0

HelixCaveRoomWild_EventScript_Entry::
	lockall
	setvar VAR_TEMP_5, 1
	goto_if_eq VAR_HELIX_RUN_ROOM_COMPLETE, 1, HelixCaveRoomWild_EventScript_EntryDone
	buffernumberstring STR_VAR_1, VAR_HELIX_RUN_WILD_TARGET
	msgbox HelixCaveRoomWild_Text_Entry, MSGBOX_DEFAULT
HelixCaveRoomWild_EventScript_EntryDone:
	releaseall
	end

HelixCaveRoomWild_EventScript_BattleResolved::
	lockall
	setvar VAR_TEMP_1, 0
	copyvar VAR_0x8009, VAR_TEMP_3
	call HelixRun_EventScript_ReportLostMons
	call HelixCaveRoomWild_EventScript_ReportFood
	@ Objective check
	copyvar VAR_0x8008, VAR_HELIX_RUN_WILD_DONE
	compare VAR_0x8008, VAR_HELIX_RUN_WILD_TARGET
	goto_if_lt HelixCaveRoomWild_EventScript_NotDoneYet
	goto_if_eq VAR_HELIX_RUN_ROOM_COMPLETE, 1, HelixCaveRoomWild_EventScript_NotDoneYet
	@ Room objective met
	goto_if_eq VAR_HELIX_RUN_WILD_FLED, 1, HelixCaveRoomWild_EventScript_CompleteNoReward
	call HelixRun_EventScript_TryIVReward
	goto HelixCaveRoomWild_EventScript_Complete

HelixCaveRoomWild_EventScript_CompleteNoReward:
	msgbox HelixCaveRoomWild_Text_Scattered, MSGBOX_DEFAULT
	goto HelixCaveRoomWild_EventScript_Complete

HelixCaveRoomWild_EventScript_Complete:
	setvar VAR_HELIX_RUN_ROOM_COMPLETE, 1
	msgbox HelixRun_Text_PathOpen, MSGBOX_DEFAULT
	releaseall
	end

HelixCaveRoomWild_EventScript_NotDoneYet::
	releaseall
	end

HelixCaveRoomWild_EventScript_ReportFood::
	goto_if_eq VAR_TEMP_2, 0, HelixCaveRoomWild_EventScript_ReportFoodEnd
	buffernumberstring STR_VAR_1, VAR_TEMP_2
	msgbox HelixCaveRoomWild_Text_GatheredFood, MSGBOX_DEFAULT
	return

HelixCaveRoomWild_EventScript_ReportFoodEnd::
	return

@ ══════════════════════════════════════════════════════════════════════
@ TEXT
@ ══════════════════════════════════════════════════════════════════════

HelixCaveRoomWild_Text_Entry:
	.string "Wild POKéMON prowl this cavern.\p"
	.string "You sense about {STR_VAR_1} presences\n"
	.string "between you and the way out.$"

HelixCaveRoomWild_Text_GatheredFood:
	.string "You gathered {STR_VAR_1} food.$"

HelixCaveRoomWild_Text_Scattered:
	.string "The remaining wild POKéMON\n"
	.string "scattered into the cracks.\p"
	.string "Fleeing earned you nothing extra.$"
