;Allow sprite data to be located in extended region
.org $01B09A
	jml.l SetSpriteDataPointer

.padbyte $FF
.pad $01B0AD

.org $01EDB0
SetSpriteDataPointer:
	lda.l $17F7C6,x
	sta.l $702600
	sta.b $02
	lda.l $17F7C8,x
	and.w #$00FF
	sta.l $702600
	sta.b $04
	bpl SetSpriteDataPointer_Return
	ldx.w #$0000
	ldy.w #$0000
SetSpriteDataPointer_CopyData:
	lda.b [$02],y
	sta.l $710000,x
	cmp.w #$FFFF
	beq SetSpriteDataPointer_EndCopy
	inx
	iny
	lda.b [$02],y
	sta.l $710000,x
	inx
	inx
	iny
	iny
	bra SetSpriteDataPointer_CopyData
SetSpriteDataPointer_EndCopy:
	stz.l $702600
	lda.w #$0071
	sta.l $702602
SetSpriteDataPointer_Return:
	jml.l $01B0AD
SetSpriteDataPointer_CODE_END:
