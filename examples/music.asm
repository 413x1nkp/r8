FPS_CONFIG    = $FFFD

SCH1          = $3000
SCH2          = $3008
SCH3          = $3010
SCH4          = $3018

SFCTR         = $3020

LAST_LO       = $00
LAST_HI       = $01

NOTE_PTRL     = $02
NOTE_PTRH     = $03

TEMP_LO       = $04
TEMP_HI       = $05

DURATION      = $06


    org $8000

init:
  lda #<update
  sta $FFFE
  lda #>update
  sta $FFFE+1

  lda #10
  sta FPS_CONFIG

  lda #<NOTE_TABLE
  sta NOTE_PTRL
  lda #>NOTE_TABLE
  sta NOTE_PTRH

  lda SFCTR
  sta LAST_LO
  lda SFCTR+1
  sta LAST_HI

  ;; attack+decay
  lda #0
  sta SCH1+3

  ;; sustain+release
  lda #240
  sta SCH1+4

  ;; channel master volume
  lda #$FF
  sta SCH1+6

  ;; control
  lda #2
  sta SCH1+7

  jsr gate_off
  jsr load_next_note

  rts



update:
  lda SFCTR
  cmp LAST_LO
  bne clock_changed

  lda SFCTR+1
  cmp LAST_HI
  bne clock_changed

  rts



clock_changed:
  lda SFCTR+1
  sta LAST_HI

  lda SFCTR
  sta LAST_LO

  dec DURATION
  bne update

  jsr load_next_note
  jmp update



load_next_note:
  ldy #0

  lda (NOTE_PTRL),Y
  sta TEMP_LO
  iny

  lda (NOTE_PTRL),Y
  sta TEMP_HI
  iny

  lda (NOTE_PTRL),Y
  beq end_of_song

  sta DURATION

  lda TEMP_LO
  ora TEMP_HI
  beq is_rest

  lda TEMP_LO
  sta SCH1
  lda TEMP_HI
  sta SCH1+1

  jsr gate_on
  jmp advance_ptr



is_rest:
  jsr gate_off
  jmp advance_ptr



advance_ptr:
  clc
  lda NOTE_PTRL
  adc #3
  sta NOTE_PTRL
  bcc .skip_hi
  inc NOTE_PTRH
.skip_hi:
  rts



end_of_song:
  ;; loop around
  lda #<NOTE_TABLE
  sta NOTE_PTRL
  lda #>NOTE_TABLE
  sta NOTE_PTRH

  jsr load_next_note
  jmp update



gate_on:
  lda SCH1+7
  ora #$80
  sta SCH1+7
  rts

gate_off:
  lda SCH1+7
  and #$7F
  sta SCH1+7
  rts



;; first  value - lower byte of frequency
;  second value - upper byte of frequency
;; third  value - duration in ticks
;
;; for the frequency, take 293.66Hz for D,
;  and round it up to nearest: 294.
;  then, convert to hex: 0x0126.
;  write the 0x26 in the first field
;; write the 0x01 in the second field
;
;; byte $06, $01, 01 ; C4
;; byte $15, $01, 01 ; C#4
;; byte $26, $01, 10 ; D4
;; byte $37, $01, 10 ; D#4
;; byte $4A, $01, 10 ; E4
;; byte $5D, $01, 10 ; F4
;; byte $72, $01, 10 ; F#4
;; byte $88, $01, 10 ; G4
;; byte $9F, $01, 10 ; G#4
;; byte $B8, $01, 10 ; A4
;; byte $D2, $01, 10 ; A#4
;; byte $EE, $01, 10 ; B4
NOTE_TABLE:
    byte $0B, $02,  9 ; C5
    byte $B8, $01,  9 ; A4
    byte $5D, $01,  9 ; F4
    byte $06, $01,  9 ; C4
    byte $26, $01,  3 ; D4
    byte $4A, $01,  3 ; E4
    byte $5D, $01,  3 ; F4
    byte $26, $01,  6 ; D4
    byte $5D, $01,  3 ; F4
    byte $06, $01, 15 ; C4
    byte $00, $00,  1 ; pause

    byte $88, $01,  9 ; G4
    byte $0B, $02,  9 ; C5
    byte $B8, $01,  9 ; A4
    byte $5D, $01,  9 ; F4
    byte $26, $01,  3 ; D4
    byte $4A, $01,  3 ; E4
    byte $5D, $01,  3 ; F4
    byte $88, $01,  6 ; G4
    byte $B8, $01,  3 ; A4
    byte $88, $01, 15 ; G4
    byte $00, $00,  1 ; pause

    byte $B8, $01,  3 ; A4
    byte $88, $01,  3 ; G4
    byte $B8, $01,  3 ; A4
    byte $88, $01,  3 ; G4
    byte $0B, $02,  6 ; C5
    byte $B8, $01,  3 ; A4
    byte $88, $01,  3 ; G4
    byte $5D, $01, 15 ; F4
    byte $00, $00,  1 ; pause
    byte $88, $01,  3 ; G4
    byte $B8, $01,  6 ; A4
    byte $5D, $01,  3 ; F4
    byte $26, $01,  6 ; D4
    byte $5D, $01,  3 ; F4
    byte $26, $01,  3 ; D4
    byte $06, $01, 15 ; C4
    byte $00, $00,  1 ; pause

    byte $06, $01,  3 ; C4
    byte $5D, $01,  6 ; F4
    byte $B8, $01,  3 ; A4
    byte $88, $01,  6 ; G4
    byte $06, $01,  3 ; C4
    byte $5D, $01,  6 ; F4
    byte $B8, $01,  3 ; A4
    byte $88, $01,  3 ; G4
    byte $B8, $01,  3 ; A4
    byte $D2, $01,  3 ; A#4
    byte $0B, $02,  3 ; C5
    byte $B8, $01,  3 ; A4
    byte $5D, $01,  3 ; F4
    byte $88, $01,  6 ; G4
    byte $06, $01,  3 ; C4
    byte $5D, $01, 15 ; F4
    byte $00, $00,  5 ; pause
