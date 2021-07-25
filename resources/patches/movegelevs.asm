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
SetSpriteDataPointer:
	lda.l $17F7C6,x
	sta.l $702600
	sta.b $08
	lda.l $17F7C8,x
	and.w #$00FF
	sta.l $702602
	sta.b $0A
	and.w #$0080
	beq SetSpriteDataPointer_Return
	ldx.w #$0000
	ldy.w #$0000
SetSpriteDataPointer_CopyData:
	lda.b [$08],y
	sta.l $70C000,x
	cmp.w #$FFFF
	beq SetSpriteDataPointer_EndCopy
	inx
	iny
	lda.b [$08],y
	sta.l $70C000,x
	inx
	inx
	iny
	iny
	bra SetSpriteDataPointer_CopyData
SetSpriteDataPointer_EndCopy:
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
	sta.b $08
	tyx
	lda.l $515348,x
	sta.w R0
	sta.b $0A
	and.w #$0080
	beq SetLevelNameDataPointer_Return
	ldx.w #$0000
	ldy.w #$0000
SetLevelNameDataPointer_CopyData:
	lda.b [$08],y
	sta.l $708000,x
	and.w #$FF00
	cmp.w #$FD00
	beq SetLevelNameDataPointer_EndCopy
	inx
	iny
	bra SetLevelNameDataPointer_CopyData
SetLevelNameDataPointer_EndCopy:
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
	sta.b $08
	txa
	lsr
	tax
	lda.l $515390,x
	and.w #$00FF
	sta.l $704098
	sta.b $0A
	and.w #$0080
	beq SetLevelMessageDataPointer_Return
	ldx.w #$0000
	ldy.w #$0000
SetLevelMessageDataPointer_CopyData:
	lda.b [$08],y
	sta.l $708000,x
	cmp.w #$FFFF
	beq SetLevelMessageDataPointer_EndCopy
	inx
	iny
	bra SetLevelMessageDataPointer_CopyData
SetLevelMessageDataPointer_EndCopy:
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
