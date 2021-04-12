;;;;;;;;;;;;;;;;;;;;;;;;;
;EGGFLUTTER EDITOR PATCH;
;;;;;;;;;;;;;;;;;;;;;;;;;

;Include hardware registers and variables
.include "regs.asm"
.include "regs_gsu.asm"
.include "vars.asm"

;Expand ROM to 8MB and SRAM to 128KB
.org $7FBD
	.db $07
.org $7FD7
	.db $0D

;Move GE levels from bank $23 to extended region
.include "movegelevs.asm"
;Apply midpoints patch
.include "moremidpoints.asm"
;Allow for easier custom GFX
.include "customgfx.asm"
