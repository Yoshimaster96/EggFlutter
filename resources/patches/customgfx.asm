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
;TODO

;Process new custom Map16 graphics
;TODO
org $09FACF
SetMap16Graphics:
;TODO
SetMap16Graphics2:
;TODO
CustomMap16PageOffsetData:
	dw $8000,$8800,$9000,$9800
	dw $A000,$A800,$B000,$B800

;Process new custom Map16 "act-as"
org $0BD381
arch superfx
	iwt r15,#SetMap16ActAs
	nop
org $0BA6AD
arch superfx
	iwt r15,#SetMap16ActAs2
	nop
org $0BF3B7
SetMap16ActAs:
	move r5,r14
	iwt r2,#$C000			;\Check for custom Map16 tile
	from r6				;|
	and r2				;|
	cmp r2				;|
	beq SetMap16ActAs_Custom	;/
	nop
	ibt r0,#$0A			;\Recalculate normal Map16 tile data pointer
	romb				;|
	move r0,r6			;|
	hib				;|
	umult #$03			;|
	iwt r14,#$BB12			;/
	bra SetMap16ActAs_Back
	nop
SetMap16ActAs_Custom:
	ibt r0,#$71			;\Calculate custom Map16 tile data pointer
	romb				;|
	iwt r2,#$07FF			;|
	from r6				;|
	and r2				;|
	umult #$03			;|
	iwt r14,#$C000			;/
SetMap16ActAs_Back:
	iwt r15,#$D386
	nop
;duplicate of above but with different return address, dunno of a way to merge the two...
SetMap16ActAs2:
	move r5,r14
	iwt r2,#$C000			;\Check for custom Map16 tile
	from r6				;|
	and r2				;|
	cmp r2				;|
	beq SetMap16ActAs2_Custom	;/
	nop
	ibt r0,#$0A			;\Recalculate normal Map16 tile data pointer
	romb				;|
	move r0,r6			;|
	hib				;|
	umult #$03			;|
	iwt r14,#$BB12			;/
	bra SetMap16ActAs2_Back
	nop
SetMap16ActAs2_Custom:
	ibt r0,#$71			;\Calculate custom Map16 tile data pointer
	romb				;|
	iwt r2,#$07FF			;|
	from r6				;|
	and r2				;|
	umult #$03			;|
	iwt r14,#$C000			;/
SetMap16ActAs2_Back:
	iwt r15,#$A6B2
	nop
arch 65816
