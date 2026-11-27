; ITEMS.INC - question blocks, bricks, map coins and the mushroom.
; Owner: Nafia - Movement and Physics (Module 12).

; UpdateMushroom moves an active power-up mushroom and handles falling.
; When airborne, it accelerates downward up to a cap, snaps to tile tops, or deactivates in pits.
; When grounded, it walks left or right, reverses on solid walls, and deactivates if far behind the camera.
; Inputs: mushroom_* variables, camera_x.
; Outputs: the mushroom walks, falls or is deactivated.
; Registers changed: none (all registers are saved and restored).
UpdateMushroom:
push ax
push dx
cmp  byte [mushroom_active], 0
jne  UpdateMushroomActive
jmp  UpdateMushroomDone
UpdateMushroomActive:
call MushroomOnGround
test ax, ax
jnz  UpdateMushroomWalk

mov  ax, [mushroom_fall]
cmp  ax, 4
jae  UpdateMushroomFallSpeedReady
inc  ax
mov  [mushroom_fall], ax
UpdateMushroomFallSpeedReady:
add  [mushroom_y], ax
call MushroomOnGround
test ax, ax
jz   UpdateMushroomCheckPit
mov  ax, [mushroom_y]
add  ax, 16
and  ax, 0FFF0h
sub  ax, 16
mov  [mushroom_y], ax
mov  word [mushroom_fall], 0
jmp  UpdateMushroomDone
UpdateMushroomCheckPit:
cmp  word [mushroom_y], 192
jb   UpdateMushroomDone
mov  byte [mushroom_active], 0
jmp  UpdateMushroomDone

UpdateMushroomWalk:
mov  word [mushroom_fall], 0
cmp  byte [mushroom_dir], 0
jne  UpdateMushroomRight
mov  ax, [mushroom_x]
dec  ax
jmp  UpdateMushroomWallProbe
UpdateMushroomRight:
mov  ax, [mushroom_x]
add  ax, 16
UpdateMushroomWallProbe:
mov  dx, [mushroom_y]
add  dx, 8
push dx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jz   UpdateMushroomMove
xor  byte [mushroom_dir], 1
jmp  UpdateMushroomLeftBehind
UpdateMushroomMove:
cmp  byte [mushroom_dir], 0
jne  UpdateMushroomMoveRight
dec  word [mushroom_x]
jmp  UpdateMushroomLeftBehind
UpdateMushroomMoveRight:
inc  word [mushroom_x]
UpdateMushroomLeftBehind:
mov  ax, [mushroom_x]
add  ax, 32
cmp  ax, [camera_x]
jae  UpdateMushroomDone
mov  byte [mushroom_active], 0
UpdateMushroomDone:
pop  dx
pop  ax
ret

; MushroomOnGround checks whether the mushroom has solid ground directly beneath it.
; It probes just below the bottom edge at two horizontal points (x+2 and x+13).
; Returns a non-zero value in AX if either probe finds a solid tile.
; Inputs: mushroom_x, mushroom_y.
; Outputs: AX = 1 if a solid tile is under either foot of the mushroom, otherwise AX = 0.
; Registers changed: AX holds the result; every other register is preserved.
MushroomOnGround:
push dx
mov  dx, [mushroom_y]
add  dx, 16
mov  ax, [mushroom_x]
add  ax, 2
push dx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jnz  MushroomOnGroundDone
mov  dx, [mushroom_y]
add  dx, 16
mov  ax, [mushroom_x]
add  ax, 13
push dx
push ax
call IsSolidAt
add  sp, 4
MushroomOnGroundDone:
pop  dx
ret

; CollectMapCoins checks all level tiles covered by Mario's current bounding box.
; Any TILE_COIN tile found is replaced with TILE_SKY in the working map.
; For each collected coin, it awards 200 points and increments the coin counter.
; Inputs: movement_x_int, movement_y_fp, movement_height, LevelMapWorking.
; Outputs: every touched coin tile becomes sky, +200 points and coin_count + 1.
; Registers changed: none (all registers are saved and restored).
CollectMapCoins:
push ax
push bx
push cx
push dx

xor  ax, ax
mov  al, [movement_y_fp+1]
mov  bx, ax
add  ax, 2
mov  cl, 4
shr  ax, cl
mov  [coin_row], ax
mov  ax, bx
add  ax, [movement_height]
sub  ax, 2
mov  cl, 4
shr  ax, cl
mov  [coin_end_row], ax
mov  ax, [movement_x_int]
add  ax, 2
mov  cl, 4
shr  ax, cl
mov  [coin_start_col], ax
mov  [coin_col], ax
mov  ax, [movement_x_int]
add  ax, 13
mov  cl, 4
shr  ax, cl
mov  [coin_end_col], ax

CollectMapCoinsRowAgain:
mov  ax, [coin_row]
cmp  ax, [coin_end_row]
ja   CollectMapCoinsDone
CollectMapCoinsColAgain:
mov  ax, [coin_col]
cmp  ax, [coin_end_col]
ja   CollectMapCoinsContinueRow
mov  cl, 4
mov  ax, [coin_row]
shl  ax, cl
add  ax, 8
push ax
mov  ax, [coin_col]
shl  ax, cl
add  ax, 8
push ax
call GetTile
add  sp, 4
cmp  ax, TILE_COIN
jne  CollectMapCoinsContinueCol
mov  cl, 4
mov  ax, TILE_SKY
push ax
mov  ax, [coin_row]
shl  ax, cl
add  ax, 8
push ax
mov  ax, [coin_col]
shl  ax, cl
add  ax, 8
push ax
call SetTileAt
add  sp, 6
mov  ax, 200
push ax
call AddScore
add  sp, 2
call AddCoin
CollectMapCoinsContinueCol:
inc  word [coin_col]
jmp  CollectMapCoinsColAgain
CollectMapCoinsContinueRow:
mov  ax, [coin_start_col]
mov  [coin_col], ax
inc  word [coin_row]
jmp  CollectMapCoinsRowAgain
CollectMapCoinsDone:
pop  dx
pop  cx
pop  bx
pop  ax
ret

; HandleBlockHit handles what happens when Mario bumps a block from below.
; Question coin blocks turn empty and award coins/points, while bricks break only when Mario is big.
; Mushroom blocks turn empty and spawn a walking power-up mushroom above the hit tile.
; Inputs: movement_x_int, movement_y_fp (Mario's head), hero_form.
; Outputs: coin blocks, mushroom blocks and bricks react; the working map changes and the mushroom may appear.
; Registers changed: none (all registers are saved and restored).
HandleBlockHit:
push ax
push bx
push dx
push si

xor  ax, ax
mov  al, [movement_y_fp+1]
mov  dx, ax
mov  ax, [movement_x_int]
add  ax, 8
push dx
push ax
call GetTile
add  sp, 4
mov  [hit_tile], al
cmp  al, TILE_QUESTION_COIN
je   HandleBlockHitCoinBlock
cmp  al, TILE_QUESTION_MUSHROOM
je   HandleBlockHitMushroomBlock
cmp  al, TILE_BRICK
je   HandleBlockHitBrickBlock
jmp  near HandleBlockHitDone
HandleBlockHitBrickBlock:
cmp  byte [hero_form], 0
jne  HandleBlockHitBreakBrick
jmp  near HandleBlockHitDone
HandleBlockHitBreakBrick:
mov  ax, TILE_SKY
jmp  HandleBlockHitChangeTile
HandleBlockHitCoinBlock:
mov  ax, TILE_EMPTY_BLOCK
push ax
xor  ax, ax
mov  al, [movement_y_fp+1]
push ax
mov  ax, [movement_x_int]
add  ax, 8
push ax
call SetTileAt
add  sp, 6
mov  ax, 200
push ax
call AddScore
add  sp, 2
call AddCoin
jmp  HandleBlockHitDone
HandleBlockHitMushroomBlock:
mov  ax, TILE_EMPTY_BLOCK
HandleBlockHitChangeTile:
push ax
xor  ax, ax
mov  al, [movement_y_fp+1]
push ax
mov  ax, [movement_x_int]
add  ax, 8
push ax
call SetTileAt
add  sp, 6
cmp  byte [hit_tile], TILE_BRICK
je   HandleBlockHitBrickPoints
mov  ax, [movement_x_int]
add  ax, 8
and  ax, 0FFF0h
mov  [mushroom_x], ax
xor  ax, ax
mov  al, [movement_y_fp+1]
and  ax, 0FFF0h
sub  ax, 16
mov  [mushroom_y], ax
mov  word [mushroom_fall], 0
mov  byte [mushroom_active], 1
mov  byte [mushroom_dir], 1
jmp  HandleBlockHitDone
HandleBlockHitBrickPoints:
mov  ax, 50
push ax
call AddScore
add  sp, 2
HandleBlockHitDone:
pop  si
pop  dx
pop  bx
pop  ax
ret

mushroom_active db 0
mushroom_dir db 1
mushroom_x dw 0
mushroom_y dw 0
mushroom_fall dw 0
coin_row dw 0
coin_end_row dw 0
coin_col dw 0
coin_end_col dw 0
coin_start_col dw 0
hit_tile db 0
