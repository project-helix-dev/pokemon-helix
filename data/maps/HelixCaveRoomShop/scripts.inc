HelixCaveRoomShop_MapScripts::
	.byte 0

HelixCaveRoomShop_EventScript_Clerk::
	lock
	faceplayer
	message HelixCaveRoomShop_Text_Welcome
	waitmessage
	pokemart HelixCaveRoomShop_Items
	msgbox HelixCaveRoomShop_Text_Bye, MSGBOX_DEFAULT
	release
	end

	.align 2
HelixCaveRoomShop_Items:
	.2byte ITEM_POKE_BALL
	.2byte ITEM_POTION
	.2byte ITEM_MOON_STONE
	pokemartlistend

@ ══════════════════════════════════════════════════════════════════════
@ TEXT
@ ══════════════════════════════════════════════════════════════════════

HelixCaveRoomShop_Text_Welcome:
	.string "Oh! A customer, down here!\p"
	.string "Take a look. Everything a\n"
	.string "spelunker could want.$"

HelixCaveRoomShop_Text_Bye:
	.string "Watch your step out there.$"
