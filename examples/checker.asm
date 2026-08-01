  org $8000

CANVAS = $1000
WIDTH  = 64
HEIGHT = 64

init:
  lda #<update
  sta $FFFE
  lda #>update
  sta $FFFE+1

  lda #<CANVAS
  sta $00
  lda #>CANVAS
  sta $01

  ldx #00
.rows:
  cpx #64
  beq .rows_over

  ldy #$00
.row:
  cpy #64
  beq .row_over

  tya
  and #$01
  sta ($00),Y

  txa
  and #$01
  clc
  adc ($00),Y

  and #$01
  clc
  adc #$FF
  sta ($00),Y

  iny
  jmp .row
.row_over:

  clc
  lda #64
  adc $00
  sta $00
  lda #00
  adc $01
  sta $01

  inx
  jmp .rows
.rows_over:

  jmp $6969

update:
  ;;rti

  lda #<CANVAS
  sta $00
  lda #>CANVAS
  sta $01

  ldx #00
.rows:
  cpx #64
  beq .rows_over

  ldy #$00
.row:
  cpy #64
  beq .row_over

  lda ($00),Y
  eor #$FF
  sta ($00),Y

  iny
  jmp .row
.row_over:

  clc
  lda #64
  adc $00
  sta $00
  lda #00
  adc $01
  sta $01

  inx
  jmp .rows
.rows_over:

  rti
