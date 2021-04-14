;"Better Middle Rings" patch, adopted for use with EF

;Modify code
.org $01B084
	jml.l SetLevel

.org $01E656
	stz.w $038E
	phx
	rep #$30
	lda.w LevelID
	asl
	tax
	ldy.w #$7F7F
	phy
	plb
	plb
	lda.l $17F551,x
	sta.w $7E01
	txa
	lsr
	sep #$30
	sta.w $7E00
	stz.w $7E03
	plx
	plb
	plb
	rtl
SetLevel:
	and.w #$00FF
	sta.w LevelID
	jml.l $01B088

.padbyte $FF
.pad $01E687

;Modify midpoint data
.org $17F551
;TODO
