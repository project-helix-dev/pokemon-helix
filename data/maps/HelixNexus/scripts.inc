HelixNexus_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, HelixNexus_OnTransition
	.byte 0

HelixNexus_OnTransition:
	special HelixSpecial_SetNexusGuideGraphics
	end

@ ══════════════════════════════════════════════════════════════════════
@ GUIDE NPC
@ ══════════════════════════════════════════════════════════════════════
HelixNexus_EventScript_Guide::
	lock
	faceplayer
	goto_if_eq VAR_HELIX_NEXUS_STATE, 0, HelixNexus_EventScript_GuideIntro
	msgbox HelixNexus_Text_HeadHome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, HelixNexus_EventScript_ReturnToIsland
	msgbox HelixNexus_Text_TakeYourTime, MSGBOX_DEFAULT
	release
	end

HelixNexus_EventScript_GuideIntro::
	msgbox HelixNexus_Text_RoutesIntro, MSGBOX_DEFAULT
	@ Party-size advice based on current entry status
	special HelixSpecial_CheckRunEntry
	switch VAR_RESULT
	case HELIX_RUN_ENTRY_TOO_MANY, HelixNexus_EventScript_GuideAdviseTooMany
	case HELIX_RUN_ENTRY_NO_PARTY, HelixNexus_EventScript_GuideAdviseNoParty
	msgbox HelixNexus_Text_PartyLooksReady, MSGBOX_DEFAULT
	goto HelixNexus_EventScript_GuideIntroEnd

HelixNexus_EventScript_GuideAdviseTooMany:
	msgbox HelixNexus_Text_OnlyThree, MSGBOX_DEFAULT
	goto HelixNexus_EventScript_GuideIntroEnd

HelixNexus_EventScript_GuideAdviseNoParty:
	msgbox HelixNexus_Text_NeedOne, MSGBOX_DEFAULT
	goto HelixNexus_EventScript_GuideIntroEnd

HelixNexus_EventScript_GuideIntroEnd:
	setvar VAR_HELIX_NEXUS_STATE, 1
	release
	end

HelixNexus_EventScript_ReturnToIsland::
	msgbox HelixNexus_Text_LetsGoHome, MSGBOX_DEFAULT
	closemessage
	warp MAP_HELIX_ISLAND, 27, 22
	waitstate
	release
	end

@ ══════════════════════════════════════════════════════════════════════
@ PC TERMINAL
@ ══════════════════════════════════════════════════════════════════════
HelixNexus_EventScript_PC::
	lock
	faceplayer
	setvar VAR_0x8004, PC_LOCATION_OTHER
	playse SE_PC_ON
	msgbox gText_StorageSystemOpened, MSGBOX_DEFAULT
	special ShowPokemonStorageSystemPC
	waitstate
	playse SE_PC_OFF
	release
	end

@ ══════════════════════════════════════════════════════════════════════
@ DOORS
@ ══════════════════════════════════════════════════════════════════════
HelixNexus_EventScript_CavesDoor::
	lockall
	setvar VAR_0x8008, HELIX_RUN_DIR_UP
	goto_if_eq VAR_HELIX_NEXUS_STATE, 0, HelixNexus_EventScript_DoorNotBriefed
	special HelixSpecial_CheckRunEntry
	switch VAR_RESULT
	case HELIX_RUN_ENTRY_OK, HelixNexus_EventScript_ConfirmCaves
	case HELIX_RUN_ENTRY_ALREADY_TODAY, HelixNexus_EventScript_RejectRest
	case HELIX_RUN_ENTRY_NO_PARTY, HelixNexus_EventScript_RejectNoParty
	case HELIX_RUN_ENTRY_TOO_MANY, HelixNexus_EventScript_RejectTooMany
	case HELIX_RUN_ENTRY_RETIRED, HelixNexus_EventScript_RejectRetired
	goto HelixRun_EventScript_BounceBack

HelixNexus_EventScript_ConfirmCaves:
	msgbox HelixNexus_Text_EnterCaves, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HelixRun_EventScript_BounceBack
	special HelixSpecial_StartCavesRun
	warp MAP_HELIX_CAVE_TRAINER_INTRO, 5, 7
	waitstate
	releaseall
	end

HelixNexus_EventScript_DoorNotBriefed:
	msgbox HelixNexus_Text_TalkToGuideFirst, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

HelixNexus_EventScript_RejectRest:
	msgbox HelixNexus_Text_RestFirst, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

HelixNexus_EventScript_RejectNoParty:
	msgbox HelixNexus_Text_NeedOne, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

HelixNexus_EventScript_RejectTooMany:
	msgbox HelixNexus_Text_OnlyThree, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

HelixNexus_EventScript_RejectRetired:
	msgbox HelixNexus_Text_RetiredBlocked, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

HelixNexus_EventScript_LockedDoorLeft::
	lockall
	setvar VAR_0x8008, HELIX_RUN_DIR_LEFT
	msgbox HelixNexus_Text_RouteClosed, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

HelixNexus_EventScript_LockedDoorRight::
	lockall
	setvar VAR_0x8008, HELIX_RUN_DIR_RIGHT
	msgbox HelixNexus_Text_RouteClosed, MSGBOX_DEFAULT
	goto HelixRun_EventScript_BounceBack

@ ══════════════════════════════════════════════════════════════════════
@ TEXT
@ ══════════════════════════════════════════════════════════════════════

HelixNexus_Text_RoutesIntro:
	.string "This is the Nexus.\p"
	.string "Three routes branch out from here.\n"
	.string "For now, only the top one is open:\l"
	.string "the CAVES.\p"
	.string "The Caves are the first stretch of a\n"
	.string "long climb toward the mountains.\p"
	.string "One more thing. Expeditions are\n"
	.string "small: three POKéMON at most.\p"
	.string "Use the PC in the corner to adjust\n"
	.string "your party before you set out.$"

HelixNexus_Text_PartyLooksReady:
	.string "Your group looks ready.\n"
	.string "Head through the top door when\l"
	.string "you want to set out.$"

HelixNexus_Text_OnlyThree:
	.string "You can only bring three POKéMON\n"
	.string "into the CAVES.\p"
	.string "Deposit some in the PC first.$"

HelixNexus_Text_NeedOne:
	.string "You need at least one POKéMON\n"
	.string "that can battle before setting out.$"

HelixNexus_Text_RetiredBlocked:
	.string "POKéMON that already returned from\n"
	.string "an expedition can't be sent out again.\p"
	.string "Leave them in the PC. They've\n"
	.string "earned it.$"

HelixNexus_Text_RestFirst:
	.string "You've already been out today.\n"
	.string "You should rest before setting\l"
	.string "out again.$"

HelixNexus_Text_TalkToGuideFirst:
	.string "You should hear the routes\n"
	.string "explained before setting out.$"

HelixNexus_Text_EnterCaves:
	.string "The CAVES stretch out ahead.\p"
	.string "Set out on an expedition?$"

HelixNexus_Text_RouteClosed:
	.string "This route is closed for now.$"

HelixNexus_Text_HeadHome:
	.string "Heading back to the island?$"

HelixNexus_Text_LetsGoHome:
	.string "Alright, hold on tight.$"

HelixNexus_Text_TakeYourTime:
	.string "Take your time.\n"
	.string "The caves aren't going anywhere.$"
