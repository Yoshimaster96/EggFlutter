;Custom objects to facilitate various extra features
;TODO

;;;;;;;
;EXGFX;
;;;;;;;
;Create ExGFX pointer table
;TODO

;Allow custom LZ1 graphics files to be loaded
;TODO
;Allow custom LZ16 graphics files to be loaded
;TODO

;;;;;;;;;;
;PALETTES;
;;;;;;;;;;
;Create ExPalette pointer table
;TODO

;Allow custom palettes to be loaded
;TODO

;;;;;;;;;;;
;ANIMATION;
;;;;;;;;;;;
;Create ExAnimation pointer table
;TODO

;Allow custom animations to be loaded
;TODO

;;;;;;;;;;;;;
;BACKGROUNDS;
;;;;;;;;;;;;;
;Allow custom BG2 files to be loaded
;TODO
;Allow custom BG3 files to be loaded
;TODO

;;;;;;;
;MAP16;
;;;;;;;
;Create Map16 pointer table
org $19FEB7	;(actually $5CFEB7 but ASAR doesn't like SuperFX mappings)
Map16BankOffsetTable:	;5CFEB7
fillbyte $00
fill $18

;Allow custom Map16 graphics and "act-as" to be loaded
;PostLoad_Map16:
;	;TODO
;	rtl

;Process new custom Map16 graphics
arch superfx
;org $09FA2A
;	iwt r15,#SetMap16Graphics
;	nop
;org $09FA93
;	iwt r15,#SetMap16Graphics2
;	nop
;org MoveGELevs_Bank09_End
;SetMap16Graphics:
;	MAC_SetMap16Graphics($FA2E,$FA39)
;SetMap16Graphics2:
;	MAC_SetMap16Graphics($FA97,$FAA4)
macro MAC_SetMap16Graphics(ra1,ra2)
	ldw (r1)			;\Check for custom Map16 tile
	iwt r11,#$C000			;|
	and r11				;|
	cmp r11				;|
	beq MAC_SetMap16Graphics_Custom	;/
	nop
	ldw (r1)			;\Recalculate normal Map16 tile data pointer
	to r9				;|
	umult r8			;/
	iwt r15,#<ra1>
	nop
MAC_SetMap16Graphics_Custom:
	ibt r0,#$71			;\Load from SRAM instead
	ramb				;/
	iwt r11,#$8000			;\Calculate custom Map16 tile data pointer
	ldw (r1)			;|
	to r9				;|
	umult r8			;|
	from r9				;|
	to r14				;|
	add r11				;/
	;TODO
	iwt r15,#<ra2>
	nop
endmacro
arch 65816

;Process new custom Map16 "act-as"
arch superfx
;org $0BD381
;	iwt r15,#SetMap16ActAs
;	nop
;org $0BA6AD
;arch superfx
;	iwt r15,#SetMap16ActAs2
;	nop
;org $0BF3B7
;SetMap16ActAs:
;	MAC_SetMap16ActAs($D386)
;SetMap16ActAs2:
;	MAC_SetMap16ActAs($A6B2)
macro MAC_SetMap16ActAs(ra)
	move r5,r14
	iwt r2,#$C000			;\Check for custom Map16 tile
	from r6				;|
	and r2				;|
	cmp r2				;|
	beq MAC_SetMap16ActAs_Custom	;/
	nop
	move r0,r6			;\Recalculate normal Map16 tile data pointer
	hib				;|
	umult #3			;|
	iwt r14,#$BB12			;/
	bra MAC_SetMap16ActAs_Back
	nop
MAC_SetMap16ActAs_Custom:
	ibt r0,#$71			;\Load from SRAM instead
	ramb				;/
	iwt r2,#$07FF			;\Calculate custom Map16 tile data pointer
	from r6				;|
	and r2				;|
	umult #3			;|
	iwt r14,#$C000			;/
MAC_SetMap16ActAs_Back:
	iwt r15,#<ra>
	nop
endmacro
arch 65816
