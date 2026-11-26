; DRAW.INC - pixels, rectangles, sprites, frame buffer, palette and text drawing.
; Owner: M Rehan - Graphics and Drawing (Modules 2, 4, 5).

SCREEN_WIDTH  equ 320
SCREEN_HEIGHT equ 200
VIDEO_SEGMENT equ 0A000h
FRAME_WORDS   equ 32000

; PutPixel - draws a single pixel at (x, y) in draw_segment if inside 320x200 bounds
; Inputs: [bp+4] = x, [bp+6] = y, [bp+8] = color.
; Outputs: one byte written into draw_segment (nothing happens when x or y is off-screen).
; Registers changed: none (all registers are saved and restored).
PutPixel:
push bp
mov  bp, sp
push ax
push bx
push cx
push di
push es

cmp  word [bp+4], SCREEN_WIDTH
jae  PutPixelDone
cmp  word [bp+6], SCREEN_HEIGHT
jae  PutPixelDone

mov  ax, [bp+6]
mov  bx, ax
mov  cl, 8
shl  ax, cl
mov  cl, 6
shl  bx, cl
add  ax, bx
add  ax, [bp+4]
mov  di, ax

mov  ax, [draw_segment]
mov  es, ax
mov  ax, [bp+8]
mov  [es:di], al

PutPixelDone:
pop  es
pop  di
pop  cx
pop  bx
pop  ax
pop  bp
ret

; FillRect - fills a solid clipped rectangle row by row in draw_segment
; Inputs: [bp+4] = x, [bp+6] = y, [bp+8] = width, [bp+10] = height, [bp+12] = color.
; Outputs: the rectangle is written into draw_segment (skipped when empty or not fully on screen).
; Registers changed: none (all registers are saved and restored).
FillRect:
push bp
mov  bp, sp
push ax
push bx
push cx
push dx
push si
push di
push es

cmp  word [bp+8], 0
je   FillRectDone
cmp  word [bp+10], 0
je   FillRectDone
cmp  word [bp+4], SCREEN_WIDTH
jae  FillRectDone
cmp  word [bp+6], SCREEN_HEIGHT
jae  FillRectDone

mov  ax, [bp+4]
add  ax, [bp+8]
jc   FillRectDone
cmp  ax, SCREEN_WIDTH
ja   FillRectDone

mov  ax, [bp+6]
add  ax, [bp+10]
jc   FillRectDone
cmp  ax, SCREEN_HEIGHT
ja   FillRectDone

mov  ax, [bp+6]
mov  bx, ax
mov  cl, 8
shl  ax, cl
mov  cl, 6
shl  bx, cl
add  ax, bx
add  ax, [bp+4]
mov  si, ax

mov  ax, [draw_segment]
mov  es, ax
mov  dx, [bp+10]
mov  al, [bp+12]

FillRectRow:
mov  di, si
mov  cx, [bp+8]
FillRectRowPixel:
mov  [es:di], al
inc  di
dec  cx
jnz  FillRectRowPixel
add  si, SCREEN_WIDTH
dec  dx
jnz  FillRectRow

FillRectDone:
pop  es
pop  di
pop  si
pop  dx
pop  cx
pop  bx
pop  ax
pop  bp
ret

; DrawSprite - renders a 0FFh-terminated list of 5-byte rectangles with optional horizontal flip and screen clipping
; Inputs: [bp+4] = x, [bp+6] = y, [bp+8] = shape address, [bp+10] = flip flag (1 = mirrored).
; Outputs: every visible rectangle of the sprite is drawn into draw_segment (clipped at the screen edges).
; Registers changed: none (all registers are saved and restored).
DrawSprite:
push bp
mov  bp, sp
sub  sp, 10
push ax
push bx
push cx
push dx
push si

mov  si, [bp+8]

DrawSpriteContinueRect:
cmp  byte [si], 0FFh
jne  DrawSpriteHasRect
jmp  DrawSpriteDone
DrawSpriteHasRect:
cmp  byte [si+4], 0
jne  DrawSpriteDrawRectRecord
jmp  DrawSpriteContinueRecord
DrawSpriteDrawRectRecord:

test word [bp+10], 1
jz   DrawSpriteNormalX
mov  dx, 16
xor  ax, ax
mov  al, [si]
sub  dx, ax
xor  cx, cx
mov  cl, [si+2]
sub  dx, cx
add  dx, [bp+4]
mov  [bp-2], dx
jmp  DrawSpriteSetY

DrawSpriteNormalX:
xor  ax, ax
mov  al, [si]
add  ax, [bp+4]
mov  [bp-2], ax

DrawSpriteSetY:
xor  ax, ax
mov  al, [si+1]
add  ax, [bp+6]
mov  [bp-4], ax

xor  ax, ax
mov  al, [si+2]
mov  [bp-6], ax
xor  ax, ax
mov  al, [si+3]
mov  [bp-8], ax
xor  ax, ax
mov  al, [si+4]
mov  [bp-10], ax

cmp  word [bp-2], 0
jge  DrawSpriteXNonnegative
mov  ax, [bp-2]
not  ax
inc  ax
mov  cx, [bp-6]
sub  cx, ax
jle  DrawSpriteContinueRecord
mov  [bp-6], cx
mov  word [bp-2], 0

DrawSpriteXNonnegative:
cmp  word [bp-2], SCREEN_WIDTH
jae  DrawSpriteContinueRecord
mov  ax, [bp-2]
add  ax, [bp-6]
cmp  ax, SCREEN_WIDTH
jbe  DrawSpriteClipTop
mov  ax, SCREEN_WIDTH
sub  ax, [bp-2]
mov  [bp-6], ax

DrawSpriteClipTop:
cmp  word [bp-4], 0
jge  DrawSpriteYNonnegative
mov  ax, [bp-4]
not  ax
inc  ax
mov  cx, [bp-8]
sub  cx, ax
jle  DrawSpriteContinueRecord
mov  [bp-8], cx
mov  word [bp-4], 0

DrawSpriteYNonnegative:
cmp  word [bp-4], SCREEN_HEIGHT
jae  DrawSpriteContinueRecord
mov  ax, [bp-4]
add  ax, [bp-8]
cmp  ax, SCREEN_HEIGHT
jbe  DrawSpriteDrawRect
mov  ax, SCREEN_HEIGHT
sub  ax, [bp-4]
mov  [bp-8], ax

DrawSpriteDrawRect:
push word [bp-10]
push word [bp-8]
push word [bp-6]
push word [bp-4]
push word [bp-2]
call FillRect
add  sp, 10

DrawSpriteContinueRecord:
add  si, 5
jmp  DrawSpriteContinueRect

DrawSpriteDone:
pop  si
pop  dx
pop  cx
pop  bx
pop  ax
mov  sp, bp
pop  bp
ret

; ClearFrame - fills the entire off-screen frame_segment buffer with a byte color repeated as words
; Inputs: [bp+4] = color, frame_segment.
; Outputs: all 32000 words of the off-screen buffer are filled using REP STOSW.
; Registers changed: none (all registers are saved and restored).
ClearFrame:
push bp
mov  bp, sp
push ax
push cx
push di
push es

mov  ax, [frame_segment]
mov  es, ax
xor  di, di
mov  al, [bp+4]
mov  ah, al
mov  cx, FRAME_WORDS
cld
rep  stosw

pop  es
pop  di
pop  cx
pop  ax
pop  bp
ret

; CopyFrame - copies the completed off-screen frame_segment buffer into VGA video memory at 0A000h
; Inputs: frame_segment.
; Outputs: the whole off-screen buffer is copied to video memory at 0A000h using REP MOVSW.
; Registers changed: none (all registers are saved and restored).
CopyFrame:
push ax
push cx
push si
push di
push ds
push es

mov  ax, [frame_segment]
mov  ds, ax
mov  ax, VIDEO_SEGMENT
mov  es, ax
xor  si, si
xor  di, di
mov  cx, FRAME_WORDS
cld
rep  movsw

pop  es
pop  ds
pop  di
pop  si
pop  cx
pop  ax
ret

NAVY_COLOR    equ 32
REDORANGE     equ 33

; InitFont - queries BIOS INT 10h (AX=1130h, BH=03h) to store the 8x8 ROM font pointer in font_seg:font_off
; Inputs: none.
; Outputs: font_seg and font_off hold the BIOS 8x8 ROM font address.
; Registers changed: none (all registers are saved and restored).
InitFont:
push ax
push bx
push cx
push dx
push bp
push es
mov  ax, 1130h
mov  bh, 03h
int  10h
mov  [font_seg], es
mov  [font_off], bp
pop  es
pop  bp
pop  dx
pop  cx
pop  bx
pop  ax
ret

; SetPalette - sets custom VGA DAC RGB entries for navy, red-orange, and ArtPaletteRGB colors
; Inputs: the ArtPaletteRGB table.
; Outputs: DAC colors 32 and 33 and the artwork colors starting at ART_COLOR_BASE are programmed.
; Registers changed: none (all registers are saved and restored).
SetPalette:
push ax
push bx
push cx
push dx
push si
push di
mov  ax, 1010h
mov  bx, NAVY_COLOR
xor  dh, dh
xor  ch, ch
mov  cl, 12
int  10h
mov  ax, 1010h
mov  bx, REDORANGE
mov  dh, 63
mov  ch, 14
xor  cl, cl
int  10h
mov  si, ArtPaletteRGB
mov  bx, ART_COLOR_BASE
mov  di, ART_COLOR_COUNT
SetPaletteArtPalette:
push si
push bx
push di
mov  ax, 1010h
mov  dh, [si]
mov  ch, [si+1]
mov  cl, [si+2]
int  10h
pop  di
pop  bx
pop  si
add  si, 3
inc  bx
dec  di
jnz  SetPaletteArtPalette
pop  di
pop  si
pop  dx
pop  cx
pop  bx
pop  ax
ret

; DrawText - draws a null-terminated string using the scaled 8x8 ROM font bitmap via FillRect
; Inputs: [bp+4] = x, [bp+6] = y, [bp+8] = scale, [bp+10] = color, [bp+12] = string address, text_spacing, font_seg:font_off.
; Outputs: the string is drawn into draw_segment, one scaled block per font pixel.
; Registers changed: none (all registers are saved and restored).
DrawText:
push bp
mov  bp, sp
sub  sp, 8
push ax
push bx
push cx
push dx
push si
push di
push es

mov  si, [bp+12]
mov  ax, [bp+4]
mov  [bp-2], ax
DrawTextContinueChar:
xor  ax, ax
mov  al, [si]
test al, al
jz   DrawTextDone
inc  si
mov  cl, 3
shl  ax, cl
add  ax, [font_off]
mov  di, ax
mov  ax, [bp+6]
mov  [bp-6], ax
mov  word [bp-8], 8
DrawTextRow:
mov  ax, [bp-2]
mov  [bp-4], ax
mov  es, [font_seg]
mov  al, [es:di]
inc  di
mov  bl, 80h
mov  cx, 8
DrawTextCol:
test al, bl
jz   DrawTextSkipPixel
push word [bp+10]
push word [bp+8]
push word [bp+8]
push word [bp-6]
push word [bp-4]
call FillRect
add  sp, 10
DrawTextSkipPixel:
mov  dx, [bp+8]
add  [bp-4], dx
shr  bl, 1
dec  cx
jnz  DrawTextCol
mov  dx, [bp+8]
add  [bp-6], dx
dec  word [bp-8]
jnz  DrawTextRow
mov  ax, [bp+8]
mov  cl, 3
shl  ax, cl
add  ax, [text_spacing]
add  [bp-2], ax
jmp  DrawTextContinueChar
DrawTextDone:
pop  es
pop  di
pop  si
pop  dx
pop  cx
pop  bx
pop  ax
mov  sp, bp
pop  bp
ret

; DrawTextOutlined - draws a 1-pixel color-0 border around text across 8 neighbor offsets before drawing main text
; Inputs: [bp+4] = x, [bp+6] = y, [bp+8] = scale, [bp+10] = color, [bp+12] = string address.
; Outputs: the string is drawn in color 0 at eight offsets and then once in the requested color.
; Registers changed: none (all registers are saved and restored).
DrawTextOutlined:
push bp
mov  bp, sp
push ax
push cx
push si

mov  si, outline_offsets
mov  cx, 8
DrawTextOutlinedOutline:
push word [bp+12]
xor  ax, ax
push ax
push word [bp+8]
mov  al, [si+1]
xor  ah, ah
test al, 80h
jz   DrawTextOutlinedSignReady1
mov  ah, 0FFh
DrawTextOutlinedSignReady1:
add  ax, [bp+6]
push ax
mov  al, [si]
xor  ah, ah
test al, 80h
jz   DrawTextOutlinedSignReady2
mov  ah, 0FFh
DrawTextOutlinedSignReady2:
add  ax, [bp+4]
push ax
call DrawText
add  sp, 10
add  si, 2
dec  cx
jnz  DrawTextOutlinedOutline
push word [bp+12]
push word [bp+10]
push word [bp+8]
push word [bp+6]
push word [bp+4]
call DrawText
add  sp, 10

pop  si
pop  cx
pop  ax
pop  bp
ret

; DrawTextCentered - calculates total string width and draws text horizontally centered across the 320px screen
; Inputs: [bp+4] = y, [bp+6] = scale, [bp+8] = color, [bp+10] = string address, text_spacing.
; Outputs: the string is drawn horizontally centered on the 320 pixel screen.
; Registers changed: none (all registers are saved and restored).
DrawTextCentered:
push bp
mov  bp, sp
push ax
push bx
push dx
push si

mov  si, [bp+10]
xor  bx, bx
DrawTextCenteredCount:
cmp  byte [si], 0
je   DrawTextCenteredCounted
inc  si
inc  bx
jmp  DrawTextCenteredCount
DrawTextCenteredCounted:
mov  ax, [bp+6]
shl  ax, 1
shl  ax, 1
shl  ax, 1
add  ax, [text_spacing]
mul  bx
sub  ax, [text_spacing]
mov  bx, SCREEN_WIDTH
sub  bx, ax
shr  bx, 1

push word [bp+10]
push word [bp+8]
push word [bp+6]
push word [bp+4]
push bx
call DrawText
add  sp, 10

pop  si
pop  dx
pop  bx
pop  ax
pop  bp
ret

; DrawSpriteScaled - draws a 0FFh-terminated sprite scaled up by an integer factor using FillRect
; Inputs: [bp+4] = x, [bp+6] = y, [bp+8] = shape address, [bp+10] = scale factor.
; Outputs: the sprite is drawn enlarged into draw_segment (no flip, no clipping).
; Registers changed: none (all registers are saved and restored).
DrawSpriteScaled:
push bp
mov  bp, sp
push ax
push bx
push si

mov  si, [bp+8]
mov  bl, [bp+10]
DrawSpriteScaledContinue:
cmp  byte [si], 0FFh
je   DrawSpriteScaledDone
cmp  byte [si+4], 0
je   DrawSpriteScaledSkip
xor  ax, ax
mov  al, [si+4]
push ax
mov  al, [si+3]
mul  bl
push ax
mov  al, [si+2]
mul  bl
push ax
mov  al, [si+1]
mul  bl
add  ax, [bp+6]
push ax
mov  al, [si]
mul  bl
add  ax, [bp+4]
push ax
call FillRect
add  sp, 10
DrawSpriteScaledSkip:
add  si, 5
jmp  DrawSpriteScaledContinue
DrawSpriteScaledDone:
pop  si
pop  bx
pop  ax
pop  bp
ret

; AppendText - appends a null-terminated source string to a destination buffer and returns pointer to its null byte
; Inputs: [bp+4] = destination address, [bp+6] = source string address.
; Outputs: the source string and its zero byte are copied to the destination; AX = address of that zero byte.
; Registers changed: AX returns the new end address; every other register is preserved.
AppendText:
push bp
mov  bp, sp
push si
push di
push es
push ds
pop  es
mov  si, [bp+6]
mov  di, [bp+4]
AppendTextCopy:
mov  al, [si]
inc  si
mov  [es:di], al
inc  di
test al, al
jnz  AppendTextCopy
dec  di
mov  ax, di
pop  es
pop  di
pop  si
pop  bp
ret

; AppendNumber - converts an unsigned 16-bit integer into decimal ASCII characters and appends a null byte
; Inputs: [bp+4] = destination address, [bp+6] = unsigned 16-bit value.
; Outputs: the decimal digits and a zero byte are written to the destination; AX = address of that zero byte.
; Registers changed: AX returns the new end address; every other register is preserved.
AppendNumber:
push bp
mov  bp, sp
push bx
push cx
push dx
push di
mov  di, [bp+4]
mov  ax, [bp+6]
mov  bx, 10
xor  cx, cx
AppendNumberDivide:
xor  dx, dx
div  bx
push dx
inc  cx
test ax, ax
jnz  AppendNumberDivide
AppendNumberEmit:
pop  dx
add  dl, '0'
mov  [di], dl
inc  di
dec  cx
jnz  AppendNumberEmit
mov  byte [di], 0
mov  ax, di
pop  di
pop  dx
pop  cx
pop  bx
pop  bp
ret

font_seg        dw 0F000h
font_off        dw 0FA6Eh
text_spacing    dw 0
screen_key      dw 0FFFFh
line_buffer     times 40 db 0
outline_offsets db -1,-1, 0,-1, 1,-1, -1,0, 1,0, -1,1, 0,1, 1,1
