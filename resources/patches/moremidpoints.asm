;"Better Middle Rings" patch, adopted for use with EF

;Modify code
org $01B084
	jmp.w SetLevel
padbyte $FF
pad $01B088

org $01E656
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
	asl
	jmp.w $01B088
padbyte $FF
pad $01E687

;Modify midpoint data
org $17F551
	db $78,$76,$82,$7B,$00,$00,$F7,$78,$0E,$5B,$61,$70,$70,$75,$09,$37
	db $00,$00,$78,$53,$AD,$4B,$00,$00,$00,$00,$00,$00,$00,$00,$9E,$24
	db $00,$00,$00,$00,$00,$00,$AB,$7A,$F2,$78,$00,$00,$00,$00,$00,$00
	db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$6B,$3F,$00,$00,$41,$52
	db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$8A,$5E,$00,$00,$B6,$49
	db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
	db $00,$00,$00,$00,$4E,$55,$E1,$0A,$F7,$7A,$D7,$1C,$00,$00,$00,$00
	db $00,$00,$00,$00,$00,$00,$09,$49,$75,$68,$00,$00,$00,$00,$00,$00
	db $00,$00,$00,$00,$0C,$29,$0B,$2A,$5A,$61,$0B,$7A,$32,$6C,$00,$00
	db $A4,$77,$00,$00,$3A,$58,$00,$00,$86,$78,$ED,$38,$3D,$6A,$62,$59
	db $F1,$6B,$71,$6B,$0F,$67,$00,$00,$03,$3A,$78,$61,$00,$00,$14,$73
	db $03,$3A,$CB,$70,$00,$00,$30,$40,$93,$38,$1A,$34,$08,$4A,$0B,$4A
	db $02,$6A,$00,$00,$37,$15,$05,$2A,$12,$79,$0E,$7A,$07,$4A,$00,$00
	db $05,$68,$00,$00,$43,$1E,$75,$0A,$67,$4B,$00,$00,$EC,$68,$00,$00
	db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$03,$5A
	db $04,$27,$00,$00,$05,$6B,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
	db $1D,$6E,$3D,$4A,$00,$00,$00,$00,$00,$00,$00,$00,$2C,$59,$00,$00
	db $58,$44,$00,$00,$00,$00,$14,$46,$00,$00,$09,$39,$00,$00,$00,$00
	db $0A,$7A,$08,$78,$00,$00,$55,$27,$0A,$76,$00,$00,$00,$00,$00,$00
	db $00,$00,$00,$00,$00,$00,$E9,$3F,$00,$00,$00,$00,$00,$00,$00,$00
	db $00,$00,$00,$00,$00,$00,$09,$7A,$00,$00,$00,$00,$00,$00,$00,$00
	db $00,$00,$00,$00,$73,$7A,$00,$00,$00,$00,$00,$00,$00,$00,$04,$0A
	db $44,$7A,$00,$00,$00,$00,$4F,$6B,$00,$00,$00,$00,$52,$27,$00,$00
	db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$08,$7A,$1F,$34,$00,$00
	db $00,$00,$76,$79,$00,$00,$00,$00,$00,$00,$C8,$71,$00,$00,$00,$00
	db $0C,$39,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$06,$76,$00,$00
	db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
	db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$04,$7A

padbyte $FF
pad $17F7C3
