; LEVEL.INC - level map, tiles, map drawing and all sprite artwork.
; Owner: M Rehan - Graphics and Drawing (Modules 3, 4).

%define TILE_SKY 0
%define TILE_GROUND 1
%define TILE_BRICK 2
%define TILE_QUESTION_COIN 3
%define TILE_QUESTION_MUSHROOM 4
%define TILE_EMPTY_BLOCK 5
%define TILE_PIPE_TOP_LEFT 6
%define TILE_PIPE_TOP_RIGHT 7
%define TILE_PIPE_BODY_LEFT 8
%define TILE_PIPE_BODY_RIGHT 9
%define TILE_STAIR 10
%define TILE_COIN 11
%define TILE_FLAG 12

; DrawTile renders a single 16x16 level tile sprite at the given screen coordinates.
; It looks up the tile ID in TileSpriteTable and swaps in FlagTopTile when drawing the top of the flag pole.
; If a valid non-null sprite pointer is found, it calls DrawSprite to plot it.
; Inputs: [bp+4] = x, [bp+6] = y, [bp+8] = tile code.
; Outputs: the sprite of that tile is drawn (sky draws nothing).
; Registers changed: none (all registers are saved and restored).
DrawTile:
push bp
mov  bp, sp
push ax
push bx
push si

mov  ax, [bp+8]
cmp  ax, TILE_FLAG
ja   DrawTileDone
shl  ax, 1
mov  bx, ax
mov  si, [TileSpriteTable+bx]
cmp  word [bp+8], TILE_FLAG
jne  DrawTileDraw
cmp  word [bp+6], 32
jne  DrawTileDraw
mov  si, FlagTopTile

DrawTileDraw:
test si, si
jz   DrawTileDone
xor  ax, ax
push ax
push si
push word [bp+6]
push word [bp+4]
call DrawSprite
add  sp, 8

DrawTileDone:
pop  si
pop  bx
pop  ax
pop  bp
ret

; DrawMap draws the visible section of the level map onto the frame buffer.
; It clamps the camera X position to 640, computes sub-tile horizontal scroll offset, and converts to tile column.
; Then it loops across visible rows and columns, reading from LevelMapWorking and calling DrawTile.
; Inputs: [bp+4] = camera_x, LevelMapWorking.
; Outputs: the 21 visible tile columns of all 12 rows are drawn, shifted by camera_x mod 16.
; Registers changed: none (all registers are saved and restored).
DrawMap:
push bp
mov  bp, sp
sub  sp, 6
push ax
push bx
push cx
push dx
push si
push di

mov  ax, [bp+4]
cmp  ax, 640
jbe  DrawMapCameraOk
mov  ax, 640
DrawMapCameraOk:
mov  bx, ax
and  bx, 15
not  bx
inc  bx
mov  [bp-4], bx
mov  cl, 4
shr  ax, cl
mov  [bp-6], ax
mov  si, LevelMapWorking
add  si, ax
mov  word [bp-2], 12
xor  dx, dx
mov  di, [bp-4]

DrawMapRow:
mov  cx, 21
DrawMapCol:
cmp  si, LevelMapWorking+720
jae  DrawMapEmptyTile
xor  ax, ax
mov  al, [si]
jmp  DrawMapDrawTile
DrawMapEmptyTile:
xor  ax, ax
DrawMapDrawTile:
push ax
push dx
push di
call DrawTile
add  sp, 6
inc  si
add  di, 16
dec  cx
jnz  DrawMapCol
add  si, 39
mov  di, [bp-4]
add  dx, 16
dec  word [bp-2]
jnz  DrawMapRow

pop  di
pop  si
pop  dx
pop  cx
pop  bx
pop  ax
mov  sp, bp
pop  bp
ret

; GetTile returns the tile ID at pixel coordinates (X=[bp+4], Y=[bp+6]) in LevelMapWorking.
; Out-of-bounds positions (X >= 960 or Y >= 192) return 0 (TILE_SKY).
; Otherwise it converts pixel coordinates into grid indices (row * 60 + col) and fetches the byte.
; Inputs: [bp+4] = x, [bp+6] = y (world pixels).
; Outputs: AX = tile code at that position (0 when outside the map).
; Registers changed: AX holds the result; every other register is preserved.
GetTile:
push bp
mov  bp, sp
push bx
push cx
push si

cmp  word [bp+4], 960
jae  GetTileOutside
cmp  word [bp+6], 192
jae  GetTileOutside

mov  ax, [bp+6]
shr  ax, 1
shr  ax, 1
shr  ax, 1
shr  ax, 1
mov  bx, ax
shl  ax, 1
shl  ax, 1
shl  ax, 1
shl  ax, 1
shl  ax, 1
shl  ax, 1
mov  cx, bx
shl  cx, 1
shl  cx, 1
sub  ax, cx

mov  bx, [bp+4]
shr  bx, 1
shr  bx, 1
shr  bx, 1
shr  bx, 1
add  ax, bx
mov  si, LevelMapWorking
add  si, ax
xor  ax, ax
mov  al, [si]
jmp  GetTileDone

GetTileOutside:
xor  ax, ax

GetTileDone:
pop  si
pop  cx
pop  bx
pop  bp
ret

; SetTileAt modifies a tile in LevelMapWorking at the tile cell corresponding to pixel (X=[bp+4], Y=[bp+6]).
; It ignores coordinates outside the map bounds (X >= 960 or Y >= 192).
; For valid coordinates, it computes row * 60 + col and writes the new tile value from [bp+8].
; Inputs: [bp+4] = x, [bp+6] = y (world pixels), [bp+8] = new tile code.
; Outputs: the tile is written into LevelMapWorking (ignored when outside the map).
; Registers changed: none (all registers are saved and restored).
SetTileAt:
push bp
mov  bp, sp
push ax
push bx
push cx
push si

cmp  word [bp+4], 960
jae  SetTileAtDone
cmp  word [bp+6], 192
jae  SetTileAtDone

mov  ax, [bp+6]
mov  cl, 4
shr  ax, cl
mov  bx, ax
mov  cl, 6
shl  ax, cl
mov  cl, 2
shl  bx, cl
sub  ax, bx
mov  bx, [bp+4]
mov  cl, 4
shr  bx, cl
add  ax, bx
mov  si, LevelMapWorking
add  si, ax
mov  ax, [bp+8]
mov  [si], al

SetTileAtDone:
pop  si
pop  cx
pop  bx
pop  ax
pop  bp
ret

; ResetMap restores the mutable working level map back to its original layout.
; It points DS:SI to LevelMapOriginal and ES:DI to LevelMapWorking.
; Then it copies all 720 map bytes using REP MOVSB.
; Inputs: LevelMapOriginal.
; Outputs: LevelMapWorking becomes an exact copy of the original map.
; Registers changed: none (all registers are saved and restored).
ResetMap:
push ax
push cx
push si
push di
push es

mov  ax, ds
mov  es, ax
mov  si, LevelMapOriginal
mov  di, LevelMapWorking
mov  cx, 720
rep  movsb

pop  es
pop  di
pop  si
pop  cx
pop  ax
ret

TileSpriteTable:
dw 0
dw GroundTile
dw BrickTile
dw QuestionCoinTile
dw QuestionMushroomTile
dw EmptyBlockTile
dw PipeTopLeftTile
dw PipeTopRightTile
dw PipeBodyLeftTile
dw PipeBodyRightTile
dw StairTile
dw CoinTileShape
dw FlagPoleTile

LevelMapOriginal:
times 60 db 0
times 60 db 0
times 58 db 0
db 12
times 1 db 0
times 58 db 0
db 12
times 1 db 0
times 2 db 0
db 11
times 2 db 0
db 11
times 4 db 0
db 11
times 4 db 0
db 11
times 2 db 0
db 11
times 4 db 0
db 11
times 20 db 0
db 11
times 13 db 0
db 12
times 1 db 0
times 32 db 0
db 11
times 2 db 0
db 11
times 4 db 0
times 3 db 2
times 6 db 0
db 11
times 8 db 0
db 12
times 1 db 0
times 4 db 0
db 3
times 3 db 0
db 4
times 3 db 0
db 3
times 5 db 0
times 3 db 2
times 5 db 0
db 4
times 3 db 0
db 3
times 25 db 0
db 10
times 1 db 0
db 12
times 1 db 0
times 24 db 0
db 6
db 7
times 29 db 0
times 2 db 10
times 1 db 0
db 12
times 1 db 0
times 6 db 0
db 6
db 7
times 16 db 0
db 8
db 9
times 28 db 0
times 3 db 10
times 1 db 0
db 12
times 1 db 0
times 6 db 0
db 8
db 9
times 16 db 0
db 8
db 9
times 27 db 0
times 4 db 10
times 1 db 0
db 1
times 1 db 0
times 14 db 1
times 3 db 0
times 17 db 1
times 3 db 0
times 23 db 1
times 14 db 1
times 3 db 0
times 17 db 1
times 3 db 0
times 23 db 1

LevelMapWorking:
times 720 db 0

GroundTile:
db 0, 0, 16, 5, 45
db 0, 5, 16, 1, 35
db 0, 6, 16, 1, 51
db 0, 7, 10, 2, 51
db 10, 7, 1, 2, 43
db 11, 7, 5, 2, 51
db 0, 9, 2, 1, 51
db 2, 9, 1, 1, 43
db 3, 9, 13, 1, 51
db 0, 10, 16, 4, 51
db 0, 14, 6, 1, 51
db 6, 14, 1, 1, 43
db 7, 14, 6, 1, 51
db 13, 14, 1, 1, 43
db 14, 14, 2, 1, 51
db 0, 15, 16, 1, 51
db 0FFh

BrickTile:
db 0, 0, 16, 1, 34
db 0, 1, 7, 2, 70
db 7, 1, 2, 6, 34
db 9, 1, 7, 6, 56
db 0, 3, 7, 4, 56
db 0, 7, 16, 2, 34
db 0, 9, 2, 6, 56
db 2, 9, 2, 6, 34
db 4, 9, 6, 6, 56
db 10, 9, 1, 6, 34
db 11, 9, 5, 6, 56
db 0, 15, 16, 1, 34
db 0FFh

QuestionCoinTile:
db 0, 0, 16, 1, 43
db 0, 1, 1, 10, 43
db 1, 1, 14, 1, 71
db 15, 1, 1, 14, 43
db 1, 2, 1, 2, 71
db 2, 2, 1, 2, 43
db 3, 2, 3, 2, 71
db 6, 2, 3, 2, 69
db 9, 2, 4, 2, 71
db 13, 2, 1, 2, 43
db 14, 2, 1, 2, 71
db 1, 4, 4, 2, 71
db 5, 4, 2, 1, 69
db 7, 4, 1, 1, 71
db 8, 4, 2, 1, 69
db 10, 4, 5, 5, 71
db 5, 5, 1, 1, 69
db 6, 5, 3, 1, 71
db 9, 5, 1, 4, 69
db 1, 6, 8, 3, 71
db 1, 9, 5, 2, 71
db 6, 9, 3, 1, 69
db 9, 9, 6, 1, 71
db 6, 10, 1, 1, 69
db 7, 10, 8, 1, 71
db 0, 11, 1, 1, 44
db 1, 11, 14, 1, 71
db 0, 12, 1, 3, 43
db 1, 12, 1, 2, 71
db 2, 12, 1, 2, 43
db 3, 12, 3, 2, 71
db 6, 12, 1, 2, 69
db 7, 12, 6, 2, 71
db 13, 12, 1, 2, 43
db 14, 12, 1, 2, 71
db 1, 14, 14, 1, 71
db 0, 15, 16, 1, 43
db 0FFh

QuestionMushroomTile:
db 0, 0, 16, 1, 43
db 0, 1, 1, 10, 43
db 1, 1, 14, 1, 71
db 15, 1, 1, 14, 43
db 1, 2, 1, 2, 71
db 2, 2, 1, 2, 43
db 3, 2, 3, 2, 71
db 6, 2, 3, 2, 69
db 9, 2, 4, 2, 71
db 13, 2, 1, 2, 43
db 14, 2, 1, 2, 71
db 1, 4, 4, 2, 71
db 5, 4, 2, 1, 69
db 7, 4, 1, 1, 71
db 8, 4, 2, 1, 69
db 10, 4, 5, 5, 71
db 5, 5, 1, 1, 69
db 6, 5, 3, 1, 71
db 9, 5, 1, 4, 69
db 1, 6, 8, 3, 71
db 1, 9, 5, 2, 71
db 6, 9, 3, 1, 69
db 9, 9, 6, 1, 71
db 6, 10, 1, 1, 69
db 7, 10, 8, 1, 71
db 0, 11, 1, 1, 44
db 1, 11, 14, 1, 71
db 0, 12, 1, 3, 43
db 1, 12, 1, 2, 71
db 2, 12, 1, 2, 43
db 3, 12, 3, 2, 71
db 6, 12, 1, 2, 69
db 7, 12, 6, 2, 71
db 13, 12, 1, 2, 43
db 14, 12, 1, 2, 71
db 1, 14, 14, 1, 71
db 0, 15, 16, 1, 43
db 0FFh

EmptyBlockTile:
db 0, 0, 16, 1, 48
db 0, 1, 1, 14, 48
db 1, 1, 14, 3, 51
db 15, 1, 1, 14, 48
db 1, 4, 2, 1, 51
db 3, 4, 2, 1, 48
db 5, 4, 6, 1, 51
db 11, 4, 2, 1, 48
db 13, 4, 2, 1, 51
db 1, 5, 14, 7, 51
db 1, 12, 2, 2, 51
db 3, 12, 2, 2, 48
db 5, 12, 8, 2, 51
db 13, 12, 1, 2, 48
db 14, 12, 1, 2, 51
db 1, 14, 14, 1, 51
db 0, 15, 16, 1, 48
db 0FFh

PipeTopLeftTile:
db 0, 0, 8, 1, 38
db 8, 0, 1, 1, 47
db 9, 0, 7, 1, 38
db 0, 1, 1, 14, 38
db 1, 1, 1, 14, 47
db 2, 1, 2, 14, 50
db 4, 1, 5, 14, 47
db 9, 1, 1, 14, 38
db 10, 1, 2, 14, 47
db 12, 1, 2, 14, 42
db 14, 1, 1, 14, 47
db 15, 1, 1, 14, 38
db 0, 15, 8, 1, 38
db 8, 15, 1, 1, 47
db 9, 15, 7, 1, 38
db 0FFh

PipeTopRightTile:
db 0, 0, 8, 1, 38
db 8, 0, 1, 1, 47
db 9, 0, 7, 1, 38
db 0, 1, 1, 14, 38
db 1, 1, 1, 14, 47
db 2, 1, 2, 14, 50
db 4, 1, 5, 14, 47
db 9, 1, 1, 14, 38
db 10, 1, 2, 14, 47
db 12, 1, 2, 14, 42
db 14, 1, 1, 14, 47
db 15, 1, 1, 14, 38
db 0, 15, 8, 1, 38
db 8, 15, 1, 1, 47
db 9, 15, 7, 1, 38
db 0FFh

PipeBodyLeftTile:
db 0, 0, 8, 1, 38
db 8, 0, 1, 1, 47
db 9, 0, 7, 1, 38
db 0, 1, 1, 14, 38
db 1, 1, 1, 14, 47
db 2, 1, 2, 14, 50
db 4, 1, 5, 14, 47
db 9, 1, 1, 14, 38
db 10, 1, 2, 14, 47
db 12, 1, 2, 14, 42
db 14, 1, 1, 14, 47
db 15, 1, 1, 14, 38
db 0, 15, 8, 1, 38
db 8, 15, 1, 1, 47
db 9, 15, 7, 1, 38
db 0FFh

PipeBodyRightTile:
db 0, 0, 8, 1, 38
db 8, 0, 1, 1, 47
db 9, 0, 7, 1, 38
db 0, 1, 1, 14, 38
db 1, 1, 1, 14, 47
db 2, 1, 2, 14, 50
db 4, 1, 5, 14, 47
db 9, 1, 1, 14, 38
db 10, 1, 2, 14, 47
db 12, 1, 2, 14, 42
db 14, 1, 1, 14, 47
db 15, 1, 1, 14, 38
db 0, 15, 8, 1, 38
db 8, 15, 1, 1, 47
db 9, 15, 7, 1, 38
db 0FFh

StairTile:
db 0, 0, 15, 1, 59
db 15, 0, 1, 15, 44
db 0, 1, 1, 14, 59
db 1, 1, 14, 14, 55
db 0, 15, 16, 1, 44
db 0FFh

CoinTileShape:
db 5, 3, 6, 1, 66
db 4, 4, 8, 1, 66
db 3, 5, 2, 4, 66
db 5, 5, 6, 4, 61
db 11, 5, 2, 4, 66
db 6, 5, 2, 4, 63
db 4, 9, 8, 1, 66
db 5, 10, 6, 1, 66
db 0FFh

FlagTopTile:
db 6, 0, 2, 1, 39
db 5, 1, 1, 1, 39
db 6, 1, 2, 1, 53
db 8, 1, 1, 1, 39
db 4, 2, 1, 2, 39
db 5, 2, 4, 2, 53
db 9, 2, 1, 2, 39
db 5, 4, 1, 1, 39
db 6, 4, 2, 1, 53
db 8, 4, 1, 1, 39
db 6, 5, 2, 2, 39
db 4, 7, 4, 1, 39
db 2, 8, 2, 1, 39
db 4, 8, 2, 1, 72
db 6, 8, 2, 3, 39
db 0, 9, 2, 1, 39
db 2, 9, 4, 1, 72
db 2, 10, 2, 1, 39
db 4, 10, 2, 1, 72
db 4, 11, 4, 1, 39
db 6, 12, 2, 3, 39
db 0FFh

FlagPoleTile:
db 6, 0, 2, 16, 39
db 0FFh

MarioStandShape:
db 4, 1, 4, 1, 62
db 3, 2, 8, 1, 62
db 3, 3, 3, 1, 49
db 6, 3, 1, 1, 58
db 7, 3, 1, 2, 49
db 8, 3, 1, 1, 58
db 2, 4, 1, 2, 49
db 3, 4, 1, 2, 58
db 4, 4, 1, 1, 49
db 5, 4, 2, 1, 58
db 8, 4, 2, 1, 58
db 4, 5, 2, 1, 49
db 6, 5, 2, 1, 58
db 8, 5, 1, 1, 49
db 9, 5, 3, 1, 58
db 2, 6, 2, 1, 49
db 4, 6, 3, 1, 58
db 7, 6, 4, 1, 49
db 3, 7, 7, 1, 58
db 2, 8, 2, 1, 62
db 4, 8, 1, 2, 40
db 5, 8, 3, 1, 62
db 1, 9, 3, 1, 62
db 5, 9, 2, 1, 62
db 7, 9, 1, 1, 40
db 8, 9, 3, 1, 62
db 0, 10, 3, 1, 62
db 3, 10, 1, 1, 40
db 4, 10, 1, 1, 60
db 5, 10, 2, 1, 40
db 7, 10, 1, 1, 60
db 8, 10, 1, 1, 40
db 9, 10, 3, 1, 62
db 0, 11, 3, 1, 58
db 3, 11, 7, 1, 40
db 10, 11, 2, 2, 58
db 0, 12, 2, 1, 58
db 2, 12, 8, 1, 40
db 2, 13, 3, 1, 40
db 7, 13, 3, 1, 40
db 1, 14, 3, 1, 54
db 8, 14, 3, 1, 54
db 0, 15, 4, 1, 54
db 8, 15, 4, 1, 54
db 0FFh

MarioRunFrameA:
db 5, 0, 4, 1, 62
db 4, 1, 8, 1, 62
db 4, 2, 3, 1, 49
db 7, 2, 1, 1, 58
db 8, 2, 1, 2, 49
db 9, 2, 1, 1, 58
db 3, 3, 1, 2, 49
db 4, 3, 1, 2, 58
db 5, 3, 1, 1, 49
db 6, 3, 2, 1, 58
db 9, 3, 2, 1, 58
db 5, 4, 2, 1, 49
db 7, 4, 2, 1, 58
db 9, 4, 1, 1, 49
db 10, 4, 3, 1, 58
db 3, 5, 2, 1, 49
db 5, 5, 3, 1, 58
db 8, 5, 4, 1, 49
db 4, 6, 7, 1, 58
db 3, 7, 2, 1, 62
db 5, 7, 1, 2, 40
db 6, 7, 3, 1, 62
db 2, 8, 3, 1, 62
db 6, 8, 2, 1, 62
db 8, 8, 1, 1, 40
db 9, 8, 3, 1, 62
db 1, 9, 3, 1, 62
db 4, 9, 1, 1, 40
db 5, 9, 1, 1, 60
db 6, 9, 2, 1, 40
db 8, 9, 1, 1, 60
db 9, 9, 1, 1, 40
db 10, 9, 3, 1, 62
db 1, 10, 3, 1, 58
db 4, 10, 7, 1, 40
db 11, 10, 2, 2, 58
db 1, 11, 2, 1, 58
db 3, 11, 8, 1, 40
db 3, 12, 3, 1, 40
db 8, 12, 3, 1, 40
db 2, 13, 3, 1, 40
db 9, 13, 1, 1, 40
db 10, 13, 4, 1, 54
db 2, 14, 3, 1, 54
db 10, 14, 5, 1, 54
db 1, 15, 4, 1, 54
db 0FFh

MarioRunFrameB:
db 5, 0, 4, 1, 62
db 4, 1, 8, 1, 62
db 4, 2, 3, 1, 49
db 7, 2, 1, 1, 58
db 8, 2, 1, 2, 49
db 9, 2, 1, 1, 58
db 3, 3, 1, 2, 49
db 4, 3, 1, 2, 58
db 5, 3, 1, 1, 49
db 6, 3, 2, 1, 58
db 9, 3, 2, 1, 58
db 5, 4, 2, 1, 49
db 7, 4, 2, 1, 58
db 9, 4, 1, 1, 49
db 10, 4, 3, 1, 58
db 3, 5, 2, 1, 49
db 5, 5, 3, 1, 58
db 8, 5, 4, 1, 49
db 4, 6, 7, 1, 58
db 3, 7, 2, 1, 62
db 5, 7, 1, 2, 40
db 6, 7, 3, 1, 62
db 2, 8, 3, 1, 62
db 6, 8, 2, 1, 62
db 8, 8, 1, 1, 40
db 9, 8, 3, 1, 62
db 1, 9, 3, 1, 62
db 4, 9, 1, 1, 40
db 5, 9, 1, 1, 60
db 6, 9, 2, 1, 40
db 8, 9, 1, 1, 60
db 9, 9, 1, 1, 40
db 10, 9, 3, 1, 62
db 1, 10, 3, 1, 58
db 4, 10, 7, 1, 40
db 11, 10, 2, 2, 58
db 1, 11, 2, 1, 58
db 3, 11, 8, 1, 40
db 5, 12, 3, 1, 40
db 10, 12, 3, 1, 40
db 2, 13, 4, 1, 54
db 6, 13, 1, 1, 40
db 11, 13, 3, 1, 40
db 1, 14, 5, 1, 54
db 11, 14, 3, 1, 54
db 11, 15, 4, 1, 54
db 0FFh

MarioJumpShape:
db 5, 0, 4, 1, 62
db 4, 1, 8, 1, 62
db 4, 2, 3, 1, 49
db 7, 2, 1, 1, 58
db 8, 2, 1, 2, 49
db 9, 2, 1, 1, 58
db 3, 3, 1, 2, 49
db 4, 3, 1, 2, 58
db 5, 3, 1, 1, 49
db 6, 3, 2, 1, 58
db 9, 3, 2, 1, 58
db 5, 4, 2, 1, 49
db 7, 4, 2, 1, 58
db 9, 4, 1, 1, 49
db 10, 4, 3, 1, 58
db 3, 5, 2, 1, 49
db 5, 5, 3, 1, 58
db 8, 5, 4, 1, 49
db 4, 6, 7, 1, 58
db 3, 7, 2, 1, 62
db 5, 7, 1, 2, 40
db 6, 7, 3, 1, 62
db 2, 8, 3, 1, 62
db 6, 8, 2, 1, 62
db 8, 8, 1, 1, 40
db 9, 8, 3, 1, 62
db 1, 9, 3, 1, 62
db 4, 9, 1, 1, 40
db 5, 9, 1, 1, 60
db 6, 9, 2, 1, 40
db 8, 9, 1, 1, 60
db 9, 9, 1, 1, 40
db 10, 9, 3, 1, 62
db 1, 10, 3, 1, 58
db 4, 10, 7, 1, 40
db 11, 10, 2, 2, 58
db 1, 11, 2, 1, 58
db 3, 11, 3, 1, 40
db 8, 11, 3, 1, 40
db 3, 12, 2, 2, 40
db 9, 12, 2, 2, 40
db 2, 14, 3, 1, 54
db 9, 14, 3, 1, 54
db 1, 15, 3, 1, 54
db 10, 15, 3, 1, 54
db 0FFh

SuperMarioShape:
db 6, 0, 4, 2, 62
db 5, 2, 8, 2, 62
db 5, 4, 3, 2, 49
db 8, 4, 1, 2, 58
db 9, 4, 1, 4, 49
db 10, 4, 1, 2, 58
db 4, 6, 1, 4, 49
db 5, 6, 1, 4, 58
db 6, 6, 1, 2, 49
db 7, 6, 2, 2, 58
db 10, 6, 2, 2, 58
db 6, 8, 2, 2, 49
db 8, 8, 2, 2, 58
db 10, 8, 1, 2, 49
db 11, 8, 3, 2, 58
db 4, 10, 2, 2, 49
db 6, 10, 3, 2, 58
db 9, 10, 4, 2, 49
db 5, 12, 7, 2, 58
db 4, 14, 2, 2, 62
db 6, 14, 1, 4, 40
db 7, 14, 3, 2, 62
db 3, 16, 3, 2, 62
db 7, 16, 2, 2, 62
db 9, 16, 1, 2, 40
db 10, 16, 3, 2, 62
db 2, 18, 3, 2, 62
db 5, 18, 1, 2, 40
db 6, 18, 1, 2, 60
db 7, 18, 2, 2, 40
db 9, 18, 1, 2, 60
db 10, 18, 1, 2, 40
db 11, 18, 3, 2, 62
db 2, 20, 3, 2, 58
db 5, 20, 7, 2, 40
db 12, 20, 2, 5, 58
db 2, 22, 2, 3, 58
db 4, 22, 8, 2, 40
db 4, 24, 3, 2, 40
db 9, 24, 3, 2, 40
db 3, 26, 4, 1, 40
db 9, 26, 4, 1, 40
db 3, 27, 3, 1, 40
db 10, 27, 3, 1, 40
db 2, 28, 4, 2, 40
db 10, 28, 4, 2, 40
db 2, 30, 4, 1, 54
db 10, 30, 4, 1, 54
db 1, 31, 5, 1, 54
db 10, 31, 5, 1, 54
db 0FFh
SuperMarioRunFrameA:
db 6, 0, 4, 2, 62
db 5, 2, 8, 2, 62
db 5, 4, 3, 2, 49
db 8, 4, 1, 2, 58
db 9, 4, 1, 4, 49
db 10, 4, 1, 2, 58
db 4, 6, 1, 4, 49
db 5, 6, 1, 4, 58
db 6, 6, 1, 2, 49
db 7, 6, 2, 2, 58
db 10, 6, 2, 2, 58
db 6, 8, 2, 2, 49
db 8, 8, 2, 2, 58
db 10, 8, 1, 2, 49
db 11, 8, 3, 2, 58
db 4, 10, 2, 2, 49
db 6, 10, 3, 2, 58
db 9, 10, 4, 2, 49
db 5, 12, 7, 2, 58
db 4, 14, 2, 2, 62
db 6, 14, 1, 4, 40
db 7, 14, 3, 2, 62
db 3, 16, 3, 2, 62
db 7, 16, 2, 2, 62
db 9, 16, 1, 2, 40
db 10, 16, 3, 2, 62
db 2, 18, 3, 2, 62
db 5, 18, 1, 2, 40
db 6, 18, 1, 2, 60
db 7, 18, 2, 2, 40
db 9, 18, 1, 2, 60
db 10, 18, 1, 2, 40
db 11, 18, 3, 2, 62
db 2, 20, 3, 2, 58
db 5, 20, 7, 2, 40
db 12, 20, 2, 5, 58
db 2, 22, 2, 3, 58
db 4, 22, 8, 2, 40
db 2, 24, 3, 2, 40
db 7, 24, 3, 2, 40
db 1, 26, 3, 2, 40
db 8, 26, 1, 2, 40
db 9, 26, 4, 2, 54
db 1, 28, 3, 2, 54
db 9, 28, 5, 2, 54
db 0, 30, 4, 2, 54
db 0FFh
SuperMarioRunFrameB:
db 6, 0, 4, 2, 62
db 5, 2, 8, 2, 62
db 5, 4, 3, 2, 49
db 8, 4, 1, 2, 58
db 9, 4, 1, 4, 49
db 10, 4, 1, 2, 58
db 4, 6, 1, 4, 49
db 5, 6, 1, 4, 58
db 6, 6, 1, 2, 49
db 7, 6, 2, 2, 58
db 10, 6, 2, 2, 58
db 6, 8, 2, 2, 49
db 8, 8, 2, 2, 58
db 10, 8, 1, 2, 49
db 11, 8, 3, 2, 58
db 4, 10, 2, 2, 49
db 6, 10, 3, 2, 58
db 9, 10, 4, 2, 49
db 5, 12, 7, 2, 58
db 4, 14, 2, 2, 62
db 6, 14, 1, 4, 40
db 7, 14, 3, 2, 62
db 3, 16, 3, 2, 62
db 7, 16, 2, 2, 62
db 9, 16, 1, 2, 40
db 10, 16, 3, 2, 62
db 2, 18, 3, 2, 62
db 5, 18, 1, 2, 40
db 6, 18, 1, 2, 60
db 7, 18, 2, 2, 40
db 9, 18, 1, 2, 60
db 10, 18, 1, 2, 40
db 11, 18, 3, 2, 62
db 2, 20, 3, 2, 58
db 5, 20, 7, 2, 40
db 12, 20, 2, 5, 58
db 2, 22, 2, 3, 58
db 4, 22, 8, 2, 40
db 4, 24, 3, 2, 40
db 9, 24, 3, 2, 40
db 1, 26, 4, 2, 54
db 5, 26, 1, 2, 40
db 10, 26, 3, 2, 40
db 0, 28, 5, 2, 54
db 10, 28, 3, 2, 54
db 10, 30, 4, 2, 54
db 0FFh
SuperMarioJumpShape:
db 6, 0, 4, 2, 62
db 5, 2, 8, 2, 62
db 5, 4, 3, 2, 49
db 8, 4, 1, 2, 58
db 9, 4, 1, 4, 49
db 10, 4, 1, 2, 58
db 4, 6, 1, 4, 49
db 5, 6, 1, 4, 58
db 6, 6, 1, 2, 49
db 7, 6, 2, 2, 58
db 10, 6, 2, 2, 58
db 6, 8, 2, 2, 49
db 8, 8, 2, 2, 58
db 10, 8, 1, 2, 49
db 11, 8, 3, 2, 58
db 4, 10, 2, 2, 49
db 6, 10, 3, 2, 58
db 9, 10, 4, 2, 49
db 5, 12, 7, 2, 58
db 4, 14, 2, 2, 62
db 6, 14, 1, 4, 40
db 7, 14, 3, 2, 62
db 3, 16, 3, 2, 62
db 7, 16, 2, 2, 62
db 9, 16, 1, 2, 40
db 10, 16, 3, 2, 62
db 2, 18, 3, 2, 62
db 5, 18, 1, 2, 40
db 6, 18, 1, 2, 60
db 7, 18, 2, 2, 40
db 9, 18, 1, 2, 60
db 10, 18, 1, 2, 40
db 11, 18, 3, 2, 62
db 2, 20, 3, 2, 58
db 5, 20, 7, 2, 40
db 12, 20, 2, 5, 58
db 0, 22, 2, 2, 58
db 2, 22, 3, 2, 40
db 7, 22, 3, 2, 40
db 2, 24, 2, 4, 40
db 8, 24, 2, 4, 40
db 1, 28, 3, 2, 54
db 8, 28, 3, 2, 54
db 0, 30, 3, 2, 54
db 9, 30, 3, 2, 54
db 0FFh

GoombaShape:
db 5, 0, 3, 1, 36
db 3, 1, 2, 1, 36
db 5, 1, 3, 1, 46
db 8, 1, 2, 1, 36
db 2, 2, 1, 1, 36
db 3, 2, 7, 1, 46
db 10, 2, 1, 1, 36
db 1, 3, 1, 1, 36
db 2, 3, 1, 1, 46
db 3, 3, 2, 1, 65
db 5, 3, 2, 3, 46
db 7, 3, 2, 1, 65
db 9, 3, 2, 1, 46
db 11, 3, 1, 1, 36
db 1, 4, 2, 1, 46
db 3, 4, 1, 1, 65
db 4, 4, 1, 1, 36
db 7, 4, 1, 1, 36
db 8, 4, 1, 1, 65
db 9, 4, 3, 1, 46
db 0, 5, 3, 1, 46
db 3, 5, 2, 1, 65
db 7, 5, 2, 1, 65
db 9, 5, 4, 1, 46
db 0, 6, 13, 1, 46
db 1, 7, 11, 1, 46
db 3, 8, 7, 1, 57
db 2, 9, 2, 1, 57
db 4, 9, 5, 1, 65
db 9, 9, 2, 1, 57
db 2, 10, 2, 1, 36
db 4, 10, 5, 1, 57
db 9, 10, 2, 1, 36
db 1, 11, 4, 2, 36
db 8, 11, 4, 2, 36
db 2, 13, 2, 1, 36
db 9, 13, 2, 1, 36
db 0FFh

GoombaFlatShape:
db 5, 2, 1, 1, 36
db 6, 2, 4, 1, 46
db 10, 2, 1, 1, 36
db 3, 3, 1, 1, 36
db 4, 3, 8, 1, 46
db 12, 3, 1, 1, 36
db 2, 4, 3, 1, 46
db 5, 4, 1, 1, 65
db 6, 4, 1, 1, 36
db 7, 4, 2, 1, 46
db 9, 4, 1, 1, 36
db 10, 4, 1, 1, 65
db 11, 4, 3, 1, 46
db 4, 5, 9, 1, 57
db 4, 6, 3, 1, 36
db 10, 6, 3, 1, 36
db 0FFh

KoopaShape:
db 5, 1, 4, 1, 64
db 4, 2, 6, 1, 64
db 4, 3, 1, 1, 64
db 5, 3, 1, 1, 68
db 6, 3, 1, 1, 37
db 7, 3, 3, 1, 64
db 4, 4, 6, 1, 64
db 5, 5, 4, 1, 64
db 3, 6, 7, 1, 41
db 1, 7, 3, 1, 41
db 4, 7, 2, 1, 53
db 6, 7, 1, 2, 41
db 7, 7, 2, 1, 53
db 9, 7, 4, 1, 41
db 0, 8, 3, 1, 41
db 3, 8, 3, 1, 53
db 7, 8, 3, 1, 53
db 10, 8, 4, 2, 41
db 0, 9, 4, 1, 41
db 4, 9, 2, 1, 53
db 6, 9, 2, 1, 41
db 8, 9, 2, 1, 53
db 0, 10, 14, 1, 41
db 0, 11, 3, 1, 68
db 3, 11, 8, 1, 41
db 11, 11, 3, 1, 68
db 2, 12, 10, 1, 68
db 2, 13, 2, 1, 64
db 5, 13, 3, 2, 64
db 10, 13, 2, 1, 64
db 1, 14, 3, 1, 64
db 10, 14, 3, 1, 64
db 0FFh

ShellShape:
db 5, 0, 7, 1, 41
db 3, 1, 2, 1, 41
db 5, 1, 2, 1, 53
db 7, 1, 2, 2, 41
db 9, 1, 2, 1, 53
db 11, 1, 2, 1, 41
db 2, 2, 2, 1, 41
db 4, 2, 3, 1, 53
db 9, 2, 3, 1, 53
db 12, 2, 2, 2, 41
db 2, 3, 3, 1, 41
db 5, 3, 2, 1, 53
db 7, 3, 3, 1, 41
db 10, 3, 2, 1, 53
db 2, 4, 12, 1, 41
db 2, 5, 2, 1, 68
db 4, 5, 8, 1, 41
db 12, 5, 2, 1, 68
db 1, 6, 14, 2, 68
db 1, 8, 2, 1, 68
db 3, 8, 1, 1, 37
db 4, 8, 8, 1, 68
db 12, 8, 1, 1, 37
db 13, 8, 2, 1, 68
db 2, 9, 12, 1, 68
db 3, 10, 10, 1, 68
db 0FFh

MushroomShape:
db 5, 0, 4, 1, 52
db 3, 1, 2, 1, 52
db 5, 1, 4, 1, 65
db 9, 1, 2, 1, 52
db 2, 2, 1, 1, 52
db 3, 2, 3, 1, 65
db 6, 2, 2, 1, 52
db 8, 2, 3, 1, 65
db 11, 2, 1, 1, 52
db 1, 3, 2, 1, 52
db 3, 3, 2, 1, 65
db 5, 3, 4, 1, 52
db 9, 3, 2, 1, 65
db 11, 3, 2, 1, 52
db 1, 4, 12, 1, 52
db 0, 5, 2, 1, 52
db 2, 5, 1, 1, 65
db 3, 5, 8, 1, 52
db 11, 5, 1, 1, 65
db 12, 5, 2, 1, 52
db 0, 6, 1, 2, 52
db 1, 6, 3, 2, 65
db 4, 6, 6, 2, 52
db 10, 6, 3, 2, 65
db 13, 6, 1, 2, 52
db 0, 8, 2, 1, 52
db 2, 8, 1, 1, 65
db 3, 8, 8, 1, 52
db 11, 8, 1, 1, 65
db 12, 8, 2, 1, 52
db 1, 9, 12, 1, 67
db 2, 10, 2, 2, 67
db 4, 10, 1, 2, 37
db 5, 10, 3, 2, 67
db 8, 10, 1, 2, 37
db 9, 10, 3, 2, 67
db 3, 12, 8, 1, 67
db 0FFh


HeartShape:
db 1, 0, 2, 2, 12
db 5, 0, 2, 2, 12
db 0, 1, 8, 2, 12
db 1, 3, 6, 1, 12
db 2, 4, 4, 1, 12
db 3, 5, 2, 1, 12
db 0FFh
HeartEmptyShape:
db 1, 0, 2, 2, 8
db 5, 0, 2, 2, 8
db 0, 1, 8, 2, 8
db 1, 3, 6, 1, 8
db 2, 4, 4, 1, 8
db 3, 5, 2, 1, 8
db 0FFh

ART_COLOR_BASE equ 34
ART_COLOR_COUNT equ 39
ArtPaletteRGB:
db 0, 0, 0, 1, 14, 0, 4, 3, 3, 4, 4, 3
db 4, 11, 1, 5, 18, 6, 8, 17, 61, 9, 27, 11
db 13, 27, 7, 14, 7, 0, 14, 5, 1, 14, 37, 13
db 15, 9, 4, 15, 33, 7, 18, 11, 7, 26, 16, 7
db 28, 50, 18, 29, 19, 8, 31, 8, 6, 31, 52, 34
db 45, 24, 7, 46, 31, 17, 47, 17, 2, 47, 47, 33
db 56, 40, 27, 57, 42, 27, 58, 63, 33, 59, 48, 14
db 59, 10, 10, 59, 55, 40, 59, 56, 23, 60, 60, 60
db 60, 24, 7, 60, 53, 41, 60, 59, 58, 61, 63, 60
db 62, 34, 21, 62, 43, 35, 62, 62, 62


