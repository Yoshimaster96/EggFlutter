;Allow sprite data to be located in extended region
org $01B09A
	jmp.w SetSpriteDataPointer
padbyte $FF
pad $01B0AD

;Allow level names to be located in extended region
org $00C789
	jml.l SetLevelNameDataPointer
padbyte $FF
pad $00C795

;Allow level messages to be located in extended region
org $01E19A
	jmp.w SetLevelMessageDataPointer
padbyte $FF
org $01E1A9

;Extra code
org $01EDB0
Copy16KBToBank70:
	lda.b $DA
	xba
	ora.w #$0054
	sta.b $DC
	lda.w #$6B70
	sta.b $DE
	ldx.b $D8
	lda.w #$4000
	jsl.l $0000DC
	rtl
Copy32KBToBank71:
	lda.b $DA
	xba
	ora.w #$0054
	sta.b $DC
	lda.w #$6B71
	sta.b $DE
	ldx.b $D8
	lda.w #$8000
	jsl.l $0000DC
	rtl

SetSpriteDataPointer:
	lda.l $17F7C6,x
	sta.l $702600
	sta.b $D8
	lda.l $17F7C8,x
	and.w #$00FF
	sta.l $702602
	sta.b $DA
	and.w #$0080
	beq SetSpriteDataPointer_Return
	ldy.w #$C000
	jsl.l Copy16KBToBank70
	lda.w #$C000
	sta.l $702600
	lda.w #$0070
	sta.l $702602
SetSpriteDataPointer_Return:
	jmp.w $01B0AD

SetLevelNameDataPointer:
	phx
	phy
	tya
	asl
	tax
	lda.l $5149BC,x
	sta.w R10
	sta.b $D8
	tyx
	lda.l $515348,x
	sta.w R0
	sta.b $DA
	and.w #$0080
	beq SetLevelNameDataPointer_Return
	ldy.w #$8000
	jsl.l Copy16KBToBank70
	lda.w #$8000
	sta.w R10
	lda.w #$0070
	sta.w R0
SetLevelNameDataPointer_Return:
	ply
	plx
	jml.l $00C795

SetLevelMessageDataPointer:
	lda.l $5110DB,x
	sta.l $704096
	sta.b $D8
	txa
	lsr
	tax
	lda.l $515390,x
	and.w #$00FF
	sta.l $704098
	sta.b $DA
	and.w #$0080
	beq SetLevelMessageDataPointer_Return
	ldy.w #$8000
	jsl.l Copy16KBToBank70
	lda.w #$8000
	sta.w $704096
	lda.w #$0070
	sta.w $704098
SetLevelMessageDataPointer_Return:
	jmp.w $01E1A9

org $09E92F
arch superfx
LevelNamePointerLogic:
	romb
	iwt r15,#$E93B
	nop
arch 65816

;Extra bank pointers
org $22D348	;(actually $515348 but ASAR doesn't like SuperFX mappings)
LevelNameBankTable:	;$515348
fillbyte $51
fill $48
LevelMessageBankTable:	;$515390
fillbyte $51
fill $012C
