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
	lda.b SRAMCopySrcHi	;\$DA -> Source bank
	xba			;|
	ora.w #$0054		;|Setup MVN instruction (opcode+first byte of operand)
	sta.b SRAMCopyRt	;/
	lda.w #$6B70		;\Setup MVN instruction (second byte of operand)+RTL instruction
	sta.b SRAMCopyRt+2	;/
	ldx.b SRAMCopySrcLo	; Setup X register for MVN
	lda.w #$4000		; Setup A register for MVN
	jsl.l SRAMCopyRt	; Run created routine
	rtl
Copy32KBToBank71:
	lda.b SRAMCopySrcHi	;\$DA -> Source bank
	xba			;|
	ora.w #$0054		;|Setup MVN instruction (opcode+first byte of operand)
	sta.b SRAMCopyRt	;/
	lda.w #$6B71		;\Setup MVN instruction (second byte of operand)+RTL instruction
	sta.b SRAMCopyRt+2	;/
	ldx.b SRAMCopySrcLo	; Setup X register for MVN
	lda.w #$8000		; Setup A register for MVN
	jsl.l SRAMCopyRt	; Run created routine
	rtl

SetSpriteDataPointer:
	lda.l $17F7C6,x				;\Get sprite data pointer
	sta.l $702600				;|
	sta.b SRAMCopySrcLo			;|
	lda.l $17F7C8,x				;|
	and.w #$00FF				;|
	sta.l $702602				;|
	sta.b SRAMCopySrcHi			;/
	ldy.w #$C000				;\Copy sprite data to SRAM
	jsl.l Copy16KBToBank70			;/
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
	sta.w R10				;|
	sta.b SRAMCopySrcLo			;|
	tyx					;|
	lda.l $515348,x				;|
	sta.w R0				;|
	sta.b SRAMCopySrcHi			;/
	ldy.w #$8000				;\Copy level name data to SRAM
	jsl.l Copy16KBToBank70			;/
	lda.w #$8000				;\Modify level name pointer
	sta.w R10				;|
	lda.w #$0070				;|
	sta.w R0				;/
	ply					;\Restore regs
	plx					;/
	jml.l $00C795

SetLevelMessageDataPointer:
	lda.l $5110DB,x				;\Get level message data pointer
	sta.l $704096				;|
	sta.b SRAMCopySrcLo			;|
	txa					;|
	lsr					;|
	tax					;|
	lda.l $515390,x				;|
	and.w #$00FF				;|
	sta.l $704098				;|
	sta.b SRAMCopySrcHi			;/
	ldy.w #$8000				;\Copy level message data to SRAM
	jsl.l Copy16KBToBank70			;/
	lda.w #$8000				;\Modify level message pointer
	sta.w $704096				;|
	lda.w #$0070				;|
	sta.w $704098				;/
	jmp.w $01E1A9

arch superfx
org $09E92F
	romb			;\Skip over some code which calculates level name data address
	iwt r15,#$E93B		;|since we do this manually
	nop			;/
arch 65816

;Extra bank pointers
org $22D348	;(actually $515348 but ASAR doesn't like SuperFX mappings)
LevelNameBankTable:	;$515348
fillbyte $51
fill $48
LevelMessageBankTable:	;$515390
fillbyte $51
fill $012C
