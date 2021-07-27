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

;Process new custom Map16 "act-as"
;TODO
