CANVAS        = $1000
CANVAS_WIDTH  = 64
CANVAS_HEIGHT = 64
KEYBOARD      = $2000
LINE          = $00
BOX_X         = $02
BOX_Y         = $03
KEYBOARD_PTR  = $04

    org $8000
init:
    lda #<update
    sta $FFFE
    lda #>update
    sta $FFFE+1

    lda #$05
    sta BOX_X
    sta BOX_Y

    lda #<KEYBOARD
    sta KEYBOARD_PTR,
    lda #>KEYBOARD
    sta KEYBOARD_PTR+1

    rts

update:

    ldy #68
    lda (KEYBOARD_PTR), Y
    cmp #00
    beq .skip_right
    inc BOX_X
.skip_right:

    ldy #65
    lda (KEYBOARD_PTR), Y
    cmp #00
    beq .skip_left
    dec BOX_X
.skip_left:

    ldy #87
    lda (KEYBOARD_PTR), Y
    cmp #00
    beq .skip_up
    dec BOX_Y
.skip_up:

    ldy #83
    lda (KEYBOARD_PTR), Y
    cmp #00
    beq .skip_down
    inc BOX_Y
.skip_down:

    jsr clear_canvas
    jsr box

    rts

clear_canvas:
    jsr line_reset
    ldx #00
.loop_row:
    cpx #CANVAS_HEIGHT
    beq .over_row

    ldy #00
.loop:
    cpy #CANVAS_WIDTH
    beq .over
    lda #00
    sta ($00),Y
    iny
    jmp .loop
.over:

    jsr line_next
    inx
    jmp .loop_row
.over_row:
    rts

box:
    jsr line_reset

    ldy BOX_Y

.loop:
    cpy #$00
    beq .over

    jsr line_next

    dey
    jmp .loop
.over:

    ldy BOX_X
    lda #$FF
    sta ($00),Y
    iny
    sta ($00),Y
    iny
    sta ($00),Y
    iny
    jsr line_next

    ldy BOX_X
    lda #$FF
    sta ($00),Y
    iny
    sta ($00),Y
    iny
    sta ($00),Y
    iny
    jsr line_next

    ldy BOX_X
    lda #$FF
    sta ($00),Y
    iny
    sta ($00),Y
    iny
    sta ($00),Y
    iny
    jsr line_next

    rts

line_next:
    clc
    lda #64
    adc $00
    sta $00
    lda #00
    adc $01
    sta $01
    rts

line_reset:
    lda #<CANVAS
    sta LINE
    lda #>CANVAS
    sta LINE+1
    rts
