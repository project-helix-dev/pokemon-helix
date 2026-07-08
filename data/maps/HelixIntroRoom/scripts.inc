@ HelixIntroRoom — Legacy map, no longer part of the intro flow.
@ Quiz and starter selection now run in the Birch speech cutscene (main_menu.c).
@ This map is unreachable in normal gameplay. Kept as a stub for map registration.

HelixIntroRoom_MapScripts::
	.byte 0

HelixIntroRoom_EventScript_Guide::
	lock
	faceplayer
	msgbox HelixIntroRoom_Text_GoodLuck, MSGBOX_DEFAULT
	release
	end

HelixIntroRoom_Text_GoodLuck:
	.string "This room is no longer in use.$"
