;;;;;;;;;;;;;;;;;;;;;;;;;
;EGGFLUTTER EDITOR PATCH;
;;;;;;;;;;;;;;;;;;;;;;;;;

;Include hardware registers and variables
.include "regs.asm"
.include "regs_gsu.asm"
.include "vars.asm"

;Expand ROM to 8MB and SRAM to 128KB
.org $FFBD
	.db $07
.org $FFD7
	.db $0D

;Allow sprite data to be located in extended region
.include "movegelevs.asm"
;Apply midpoints patch
.include "moremidpoints.asm"
;Allow for easier custom GFX
.include "customgfx.asm"
