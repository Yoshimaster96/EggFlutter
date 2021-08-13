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
CopyDataToSRAM:
	phb
	phk
	plb
	ldx.b SRAMCopySrcLo	; Setup X register for MVN
	ldy.b SRAMCopyDstLo	; Setup Y register for MVN
	lda.w SRAMCopyDstHi	;\Setup MVN instruction (opcode+first byte of operand)
	xba			;|
	ora.w #$0054		;|
	sta.b SRAMCopyRt	;/
	lda.b SRAMCopySrcHi	;\Setup MVN instruction (second byte of operand)+RTL instruction
	ora.w #$6B00		;|
	sta.b SRAMCopyRt+2	;/
	lda.b SRAMCopySize	; Setup A register for MVN
	jsl.l SRAMCopyRt	; Run created routine
	plb
	rtl

SetSpriteDataPointer:
	lda.l $17F7C6,x				;\Get sprite data pointer
	sta.b SRAMCopySrcLo			;|
	lda.l $17F7C8,x				;|
	and.w #$00FF				;|
	sta.b SRAMCopySrcHi			;/
	lda.w #$C000				;\Copy level message data to SRAM
	sta.b SRAMCopyDstLo			;|
	lda.w #$0070				;|
	sta.b SRAMCopyDstHi			;|
	lda.w #$3FFF				;|
	sta.b SRAMCopySize			;|
	jsl.l CopyDataToSRAM			;/
	lda.w #$C000				;\Modify sprite data pointer
	sta.l $702600				;|
	lda.w #$0070				;|
	sta.l $702602				;/
	jmp.w $01B0AD
SetLevelNameDataPointer:
	phx					;\Save regs
	phy					;/
	tya					;\Get level name index
	asl					;|
	tax					;/
	lda.l $5149BC,x				;\Get level name data pointer
	sta.b SRAMCopySrcLo			;|
	tyx					;|
	lda.l $515348,x				;|
	and.w #$00FF				;|
	sta.b SRAMCopySrcHi			;/
	lda.w #$8000				;\Copy level message data to SRAM
	sta.b SRAMCopyDstLo			;|
	lda.w #$0070				;|
	sta.b SRAMCopyDstHi			;|
	lda.w #$03FF				;|
	sta.b SRAMCopySize			;|
	jsl.l CopyDataToSRAM			;/
	lda.w #$8000				;\Modify level name pointer
	sta.l R10				;|
	lda.w #$0070				;|
	sta.l R0				;/
	ply					;\Restore regs
	plx					;/
	jml.l $00C795
SetLevelMessageDataPointer:
	lda.l $5110DB,x				;\Get level message data pointer
	sta.b SRAMCopySrcLo			;|
	txa					;|
	lsr					;|
	tax					;|
	lda.l $515390,x				;|
	and.w #$00FF				;|
	sta.b SRAMCopySrcHi			;/
	lda.w #$8000				;\Copy level message data to SRAM
	sta.b SRAMCopyDstLo			;|
	lda.w #$0070				;|
	sta.b SRAMCopyDstHi			;|
	lda.w #$03FF				;|
	sta.b SRAMCopySize			;|
	jsl.l CopyDataToSRAM			;/
	lda.w #$8000				;\Modify level message pointer
	sta.l $704096				;|
	lda.w #$0070				;|
	sta.l $704098				;/
	jmp.w $01E1A9

arch superfx
org $098041
	iwt r15,#CheckNewSprites_Pt1
	nop
org $09804D
	iwt r15,#CheckNewSprites_Pt2
	nop
org $09E92F
	iwt r15,#SetupLevName
	nop
org $09E95F
	iwt r15,#SetupLevName_CodeFE
	nop
org $09E963
	iwt r15,#SetupLevName_CodeFF
	nop
org $09B0B7
	iwt r15,#SetupLevMsgCtrl
	nop
org $09B9F5
	iwt r15,#SetupLevMsgImage
	nop
org $09FACF
CheckNewSprites_Pt1:
	move r0,r14		;\Load from SRAM instead
	to r12			;|
	ldb (r0)		;|
	inc r14			;|
	move r0,r14		;|
	ldb (r0)		;|
	swap			;|
	to r12			;|
	or r12			;/
	iwt r15,#$8045		;\Return to normal code
	nop			;/
CheckNewSprites_Pt2:
	move r0,r14		;\Load from SRAM instead
	to r11			;|
	ldb (r0)		;/
	inc r14			;\Copied code
	from r12		;|
	hib			;|
	lsr			;|
	move r12,r0		;/
	iwt r15,#$8055		;\Return to normal code
	nop			;/
SetupLevName:
	ibt r0,#$02		;\Copied code to make room for `ldb`
	color			;|
	iwt r13,#$E988		;|
	cache			;|
	iwt r3,#$00FF		;|
	from r10		;|
	to r14			;|
	add r11			;/
	move r0,r14		;\Load from SRAM instead
	ldb (r0)		;/
	iwt r15,#$E949		;\Return to normal code
	nop			;/
SetupLevName_CodeFE:
	inc r11			;\Copied code
	inc r14			;/
	move r0,r14		;\Load from SRAM instead
	to r8			;|
	ldb (r0)		;/
	;iwt r15,#$E963		;\Return to normal code
	;nop			;/
SetupLevName_CodeFF:
	inc r11			;\Copied code
	inc r14			;/
	move r0,r14		;\Load from SRAM instead
	to r9			;|
	ldb (r0)		;/
	inc r11			;\Copied code
	inc r14			;/
	move r0,r14		;\Load from SRAM instead
	ldb (r0)		;/
	iwt r15,#$E96A		;\Return to normal code
	nop			;/
SetupLevMsgCtrl:
	lm r14,($407C)		;\Load from SRAM instead
	move r0,r14		;|
	to r2			;|
	ldb (r0)		;|
	inc r14			;|
	move r0,r14		;|
	ldb (r0)		;|
	swap			;|
	or r2			;/
	iwt r15,#$B0C5		;\Return to normal code
	nop			;/
SetupLevMsgImage:
	lm r0,($407C)		;\Load from SRAM instead
	to r14			;|
	add #2			;|
	move r0,r14		;|
	ldb (r0)		;|
	inc r14			;|
	lsr			;|
	lsr			;|
	lsr			;|
	move r11,r0		;|
	move r10,r0		;|
	move r0,r14		;|
	to r9			;|
	ldb (r0)		;|
	inc r14			;|
	move r0,r14		;|
	ldb (r0)		;|
	inc r14			;|
	swap			;|
	or r9			;|
	lsr			;|
	to r9			;|
	lsr			;|
	move r0,r14		;|
	ldb (r0)		;|
	inc r14			;|
	add #7			;|
	lsr			;|
	lsr			;|
	to r8			;|
	lsr			;|
	move r0,r14		;|
	to r7			;|
	ldb (r0)		;|
	inc r14			;|
	move r0,r14		;|
	to r3			;|
	ldb (r0)		;|
	inc r14			;|
	move r0,r14		;|
	ldb (r0)		;/
	iwt r15,#$BA24		;\Return to normal code
	nop			;/
MoveGELevs_Bank09_End:
arch 65816

;Extra bank pointers
org $22D348	;(actually $515348 but ASAR doesn't like SuperFX mappings)
LevelNameBankTable:	;$515348
fillbyte $51
fill $48
LevelMessageBankTable:	;$515390
fillbyte $51
fill $012C
