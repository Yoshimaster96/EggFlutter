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
;	lda.b ExMAP16
;	;TODO
;	lda.w #$8000
;	sta.b SRAMCopyDstLo
;	lda.w #$0071
;	sta.b SRAMCopyDstHi
;	lda.w #$5800
;	sta.b SRAMCopySize
;	jsl.l CopyDataToSRAM
;	;TODO
;	rtl

;Process new custom Map16 graphics
arch superfx
macro MAC_SetMap16Graphics(ra1,ra2,ninc)
	ldw (r1)			;\Check for custom Map16 tile
	iwt r9,#$C000			;|
	and r9				;|
	cmp r9				;|
	beq +				;/
	nop
	ldw (r1)			;\Recalculate normal Map16 tile data pointer
	to r9				;|
	umult r8			;/
	iwt r15,#<ra1>
	nop
+
	ldw (r1)			;\Calculate custom Map16 tile data pointer
	iwt r9,#$07FF			;|
	and r9				;|
	to r14				;|
	umult r8			;|
	iwt r9,#$8000			;|
	with r14			;|
	add r9				;/
	iwt r9,#<ninc>			;\Go to next row/column
	with r1				;|
	add r9				;/
	ibt r0,#$71			;\Get byte for top/left
	ramb				;|
	move r0,r14			;|
	ldw (r0)			;|
	inc r14				;|
	inc r14				;|
	ibt r9,#$70			;|
	from r9				;|
	ramb				;|
	stw (r4)			;|
	inc r4				;|
	inc r4				;/
	ibt r0,#$71			;\Get byte for top/left
	ramb				;|
	move r0,r14			;|
	ldw (r0)			;|
	inc r14				;|
	inc r14				;|
	ibt r9,#$70			;|
	from r9				;|
	ramb				;|
	stw (r4)			;|
	inc r4				;|
	inc r4				;/
	ibt r0,#$71			;\Get byte for bottom/right
	ramb				;|
	move r0,r14			;|
	ldw (r0)			;|
	inc r14				;|
	inc r14				;|
	ibt r9,#$70			;|
	from r9				;|
	ramb				;|
	stw (r5)			;|
	inc r5				;|
	inc r5				;/
	ibt r0,#$71			;\Get byte for bottom/right
	ramb				;|
	move r0,r14			;|
	ldw (r0)			;|
	inc r14				;|
	inc r14				;|
	ibt r9,#$70			;|
	from r9				;|
	ramb				;|
	stw (r5)			;|
	inc r5				;/
	iwt r15,#<ra2>
	nop
endmacro
org $09FA2A
	iwt r15,#SetMap16Graphics
	nop
org $09FA93
	iwt r15,#SetMap16Graphics2
	nop
org MoveGELevs_Bank09_End
SetMap16Graphics:
	%MAC_SetMap16Graphics($FA2E,$FA64,$0040)
SetMap16Graphics2:
	%MAC_SetMap16Graphics($FA97,$FACB,$0002)
;Process new custom Map16 "act-as"
macro MAC_SetMap16ActAs(ra1,ra2)
	move r5,r14
	sms ($0036),r12			;\Check for custom Map16 tile
	iwt r12,#$C000			;|
	from r6				;|
	and r12				;|
	cmp r12				;|
	beq +				;/
	nop
	move r0,r6			;\Recalculate normal Map16 tile data pointer
	hib				;|
	umult #3			;|
	iwt r14,#$BB12			;|
	to r14				;|
	add r14				;/
	lms r12,($0036)
	iwt r15,#<ra1>
	nop
+
	iwt r0,#$07FF			;\Calculate custom Map16 tile data pointer
	and r6				;|
	umult #3			;|
	iwt r14,#$C000			;|
	to r14				;|
	add r14				;/
	sms ($0000),r8
	with r7
	asr
	sms ($0002),r7
	ibt r0,#$71			;\Get first byte
	ramb				;|
	move r0,r14			;|
	to r7				;|
	ldb (r0)			;|
	inc r14				;/
	move r0,r14			;\Get second byte
	ldb (r0)			;|
	inc r14				;|
	swap				;|
	to r7				;|
	or r7				;|
	ibt r0,#$70			;|
	ramb				;/
	move r0,r7			;\Check for dynamic collision
	ibt r8,#$F8			;|
	hib				;|
	and r8				;|
	ibt r8,#$72			;|
	sub r8				;|
	sub #15				;|
	bcs ++				;|
	to r8				;/
	ibt r8,#$11			;\Check if dynamic collision is solid
	add r8				;|
	lm r8,($1E08)			;|
	and r8				;|
	beq ++				;|
	to r8				;/
	with r7				;\Make solid
	or #2				;|
	to r8				;/
++
	ibt r0,#$71			;\Get third byte
	ramb				;|
	move r0,r14			;|
	to r8				;|
	ldb (r0)			;|
	ibt r0,#$70			;|
	ramb				;/
	move r14,r5
	lms r12,($0036)
	iwt r15,#<ra2>
	nop
endmacro
org $0BD381
	iwt r15,#SetMap16ActAs
	nop
org $0BA6AD
arch superfx
	iwt r15,#SetMap16ActAs2
	nop
org $0BF3B7
SetMap16ActAs:
	%MAC_SetMap16ActAs($D388,$D3B7)
SetMap16ActAs2:
	%MAC_SetMap16ActAs($A6B4,$A6E3)
arch 65816
