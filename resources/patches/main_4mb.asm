;;;;;;;;;;;;;;;;;;;;;;;;;
;EGGFLUTTER EDITOR PATCH;
;;;;;;;;;;;;;;;;;;;;;;;;;

;Include hardware registers and variables
incsrc "regs.asm"
incsrc "regs_gsu.asm"
incsrc "vars.asm"

;Expand ROM to 4MB and SRAM to 128KB
org $FFBD
	db $07
org $FFD7
	db $0C

;Allow sprite data to be located in extended region
incsrc "movegelevs.asm"
;Apply midpoints patch
incsrc "moremidpoints.asm"
;Allow for easier custom GFX
incsrc "customgfx.asm"
