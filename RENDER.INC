; RENDER.INC - building one gameplay frame: actors, Mario, HUD and hearts.
; Owner: M Rehan - Graphics and Drawing (Modules 4, 5, 13 (drawing part)).

; RenderFrame builds one complete gameplay picture in the off-screen buffer and shows it.
; It clears the frame to sky, draws the map, actors, Mario, HUD and hearts, and copies the frame to video memory once.
; Inputs: camera_x and all game variables.
; Outputs: a complete gameplay frame is on the screen (screen_key is invalidated).
; Registers changed: none (all registers are saved and restored).
RenderFrame:
push ax

mov  word [screen_key], 0FFFFh

mov  ax, COLOR_SKY
push ax
call ClearFrame
add  sp, 2

mov  ax, [camera_x]
push ax
call DrawMap
add  sp, 2

call RenderGameActors
call DrawMario

xor  ax, ax
push ax
mov  ax, 8
push ax
mov  ax, 320
push ax
mov  ax, 192
push ax
xor  ax, ax
push ax
call FillRect
add  sp, 10

call DrawHUD
call DrawHealthHearts
call CopyFrame

pop  ax
ret

; RenderGameActors draws active enemies and the power-up mushroom within the camera view.
; For each visible enemy, it chooses the walking, flat, shell, or Koopa sprite and respects facing direction.
; If a mushroom is active, it also draws MushroomShape offset by camera_x.
; Inputs: enemy_table, mushroom_*, camera_x.
; Outputs: visible enemies and the mushroom are drawn into the frame buffer.
; Registers changed: none (all registers are saved and restored).
RenderGameActors:
push ax
push bx
push cx
push dx
push si
push di
mov  si, enemy_table
mov  cx, ENEMY_COUNT
RenderGameActorsAgain:
cmp  byte [si+E_ACTIVE], 0
je   RenderGameActorsContinue
mov  ax, [si+E_X]
sub  ax, [camera_x]
cmp  ax, -15
jl   RenderGameActorsContinue
cmp  ax, 320
jge  RenderGameActorsContinue
cmp  byte [si+E_STATE], E_FLAT
je   RenderGameActorsFlat
cmp  byte [si+E_STATE], E_SHELL
je   RenderGameActorsShell
cmp  byte [si+E_STATE], E_SLIDE
je   RenderGameActorsShell
cmp  byte [si+E_TYPE], E_KOOPA
je   RenderGameActorsKoopa
mov  ax, GoombaShape
jmp  RenderGameActorsDrawEnemy
RenderGameActorsKoopa:
mov  ax, KoopaShape
jmp  RenderGameActorsDrawEnemy
RenderGameActorsFlat:
mov  ax, GoombaFlatShape
jmp  RenderGameActorsDrawEnemy
RenderGameActorsShell:
mov  ax, ShellShape
RenderGameActorsDrawEnemy:
mov  bx, ax
xor  ax, ax
mov  al, [si+E_DIR]
push ax
mov  ax, bx
push ax
xor  ax, ax
mov  al, [si+E_Y]
push ax
mov  ax, [si+E_X]
sub  ax, [camera_x]
push ax
call DrawSprite
add  sp, 8
RenderGameActorsContinue:
add  si, ENEMY_SIZE
dec  cx
jnz  RenderGameActorsAgain
cmp  byte [mushroom_active], 0
je   RenderGameActorsDone
xor  ax, ax
push ax
mov  ax, MushroomShape
push ax
mov  ax, [mushroom_y]
push ax
mov  ax, [mushroom_x]
sub  ax, [camera_x]
push ax
call DrawSprite
add  sp, 8
RenderGameActorsDone:
pop  di
pop  si
pop  dx
pop  cx
pop  bx
pop  ax
ret

; RenderMovement chooses and draws small Mario's sprite based on grounded and velocity state.
; It picks MarioJumpShape when airborne, MarioStandShape when stationary, or run frames when moving.
; Then it calls DrawSprite at Mario's screen coordinates adjusted by camera_x.
; Inputs: movement_x_int, movement_y_fp, camera_x, movement_grounded, movement_vx, movement_frame, movement_facing_left.
; Outputs: the small Mario stand, run, or jump shape is drawn.
; Registers changed: none (all registers are saved and restored).
RenderMovement:
push ax

xor  ax, ax
mov  al, [movement_facing_left]
push ax
cmp  byte [movement_grounded], 0
jne  RenderMovementOnGround
mov  ax, MarioJumpShape
jmp  RenderMovementPushSprite
RenderMovementOnGround:
cmp  word [movement_vx], 0
je   RenderMovementStanding
cmp  byte [movement_frame], 0
jne  RenderMovementFrameB
mov  ax, MarioRunFrameA
jmp  RenderMovementPushSprite
RenderMovementFrameB:
mov  ax, MarioRunFrameB
jmp  RenderMovementPushSprite
RenderMovementStanding:
mov  ax, MarioStandShape
RenderMovementPushSprite:
push ax
xor  ax, ax
mov  al, [movement_y_fp+1]
push ax
mov  ax, [movement_x_int]
sub  ax, [camera_x]
push ax
call DrawSprite
add  sp, 8

pop  ax
ret

; DrawMario renders Mario on screen at his current camera-relative position.
; When invuln_ticks is active, it alternates visibility every other frame as a blink effect.
; It draws the Super Mario stand, run, or jump shape when big, or calls RenderMovement for small Mario.
; During the win sequence it hides Mario after 18 frames, standing in for the walk to the castle.
; Inputs: hero_form, invuln_ticks, movement variables, game_state, win_frames.
; Outputs: Mario is drawn (or skipped on blink frames and at the end of the win sequence).
; Registers changed: none (all registers are saved and restored).
DrawMario:
push ax
cmp  byte [game_state], STATE_WIN_SEQUENCE
jne  DrawMarioCheckBlink
cmp  byte [win_frames], 18
jae  DrawMarioDone
DrawMarioCheckBlink:
cmp  byte [invuln_ticks], 0
je   DrawMarioVisible
test byte [movement_frame], 1
jnz  DrawMarioDone
DrawMarioVisible:
cmp  byte [hero_form], 0
je   DrawMarioSmall
xor  ax, ax
mov  al, [movement_facing_left]
push ax
cmp  byte [movement_grounded], 0
jne  DrawMarioSuperOnGround
mov  ax, SuperMarioJumpShape
jmp  DrawMarioSuperPushSprite
DrawMarioSuperOnGround:
cmp  word [movement_vx], 0
je   DrawMarioSuperStanding
cmp  byte [movement_frame], 0
jne  DrawMarioSuperFrameB
mov  ax, SuperMarioRunFrameA
jmp  DrawMarioSuperPushSprite
DrawMarioSuperFrameB:
mov  ax, SuperMarioRunFrameB
jmp  DrawMarioSuperPushSprite
DrawMarioSuperStanding:
mov  ax, SuperMarioShape
DrawMarioSuperPushSprite:
push ax
xor  ax, ax
mov  al, [movement_y_fp+1]
push ax
mov  ax, [movement_x_int]
sub  ax, [camera_x]
push ax
call DrawSprite
add  sp, 8
pop  ax
ret
DrawMarioSmall:
call RenderMovement
DrawMarioDone:
pop  ax
ret

; DrawHUD draws the top status bar and all HUD text labels and values.
; It fills the header band with navy and adds a thin divider bar underneath.
; Then it renders score, coins, world, remaining time, lives, and health labels and numbers.
; Inputs: score, coin_count, remaining_seconds, lives.
; Outputs: the navy status bar with all labels and values is drawn into the frame buffer.
; Registers changed: none (all registers are saved and restored).
DrawHUD:
push ax
push bx
push di
mov ax, NAVY_COLOR
push ax
mov ax, 16
push ax
mov ax, 320
push ax
mov ax, 0
push ax
mov ax, 0
push ax
call FillRect
add sp, 10
mov ax, 8
push ax
mov ax, 1
push ax
mov ax, 320
push ax
mov ax, 16
push ax
mov ax, 0
push ax
call FillRect
add sp, 10

mov ax, hud_score_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 0
push ax
mov ax, 4
push ax
call DrawText
add sp, 10
call BuildScoreString
mov ax, number_buffer
push ax
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 0
push ax
mov ax, 58
push ax
call DrawText
add sp, 10
mov ax, hud_coin_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 0
push ax
mov ax, 120
push ax
call DrawText
add sp, 10
call BuildCoinString
mov ax, small_number_buffer
push ax
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 0
push ax
mov ax, 165
push ax
call DrawText
add sp, 10
mov ax, hud_world_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 0
push ax
mov ax, 220
push ax
call DrawText
add sp, 10
mov ax, hud_time_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 8
push ax
mov ax, 228
push ax
call DrawText
add sp, 10
mov  ax, [remaining_seconds]
mov  di, small_number_buffer
mov  bx, 3
call NumberToFixedString
mov ax, small_number_buffer
push ax
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 8
push ax
mov ax, 278
push ax
call DrawText
add sp, 10
mov ax, hud_lives_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 8
push ax
mov ax, 4
push ax
call DrawText
add sp, 10
xor  ax, ax
mov  al, [lives]
mov  di, small_number_buffer
mov  bx, 2
call NumberToFixedString
mov ax, small_number_buffer
push ax
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 8
push ax
mov ax, 58
push ax
call DrawText
add sp, 10
mov ax, hud_health_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 8
push ax
mov ax, 104
push ax
call DrawText
add sp, 10
pop  di
pop  bx
pop  ax
ret

; DrawHealthHearts draws Mario's health as two heart icons in the HUD row.
; Small Mario (HP 1) shows one filled heart and one empty heart, while Super Mario (HP 2) shows two filled hearts.
; Each heart sprite is spaced evenly along the HUD row.
; Inputs: hero_form.
; Outputs: two heart icons are drawn: one filled for small Mario, two filled for Super Mario.
; Registers changed: none (all registers are saved and restored).
DrawHealthHearts:
push ax
push bx
push cx
push dx
mov  cx, 1
cmp  byte [hero_form], 0
je   DrawHealthHeartsCountReady
mov  cx, 2
DrawHealthHeartsCountReady:
xor  bx, bx
DrawHealthHeartsHeartAgain:
cmp  bx, 2
jae  DrawHealthHeartsDone
xor  ax, ax
push ax
mov  ax, HeartShape
cmp  bx, cx
jb   DrawHealthHeartsPushShape
mov  ax, HeartEmptyShape
DrawHealthHeartsPushShape:
push ax
mov  ax, 9
push ax
mov  ax, 168
mov  dx, bx
shl  dx, 1
shl  dx, 1
shl  dx, 1
shl  dx, 1
sub  dx, bx
add  ax, dx
push ax
call DrawSprite
add  sp, 8
inc  bx
jmp  DrawHealthHeartsHeartAgain
DrawHealthHeartsDone:
pop  dx
pop  cx
pop  bx
pop  ax
ret

hud_score_text db 'SCORE:',0
hud_coin_text db 'COIN:',0
hud_world_text db 'WORLD:1-1',0
hud_time_text db 'TIME:',0
hud_lives_text db 'LIVES:',0
hud_health_text db 'HEALTH:',0
