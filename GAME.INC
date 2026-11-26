; GAME.INC - game states, screens flow, reset, win and death sequences.
; Owner: Umer - Game Logic (Modules 1, 14, 15).

; GameStep acts as the main state dispatcher for the game loop.
; It reads the current game_state byte and checks that it does not exceed STATE_EXIT.
; Using it as a word index, it calls the matching routine in GameStateTable and restores AX and BX afterwards.
; Inputs: game_state.
; Outputs: the step routine of the current state is run once.
; Registers changed: none (all registers are saved and restored).
GameStep:
push ax
push bx
xor  ax, ax
mov  al, [game_state]
cmp  al, STATE_EXIT
ja   GameStepInvalid
shl  ax, 1
mov  bx, ax
call word [GameStateTable+bx]
GameStepInvalid:
pop  bx
pop  ax
ret

; TitleStep manages input and screen rendering while on the main title menu.
; Up and Down arrow keys toggle menu_choice with a latch so holding a key does not flip rapidly.
; Pressing Enter starts a new game or exits, while Escape exits the program and updates the screen when needed.
; Inputs: key_states (Up 48h, Down 50h, Enter 1Ch, Esc 01h), menu_choice, latches, screen_key.
; Outputs: menu_choice, game_state and the title picture are updated.
; Registers changed: none (all registers are saved and restored).
TitleStep:
push ax
cmp  byte [key_states+48h], 0
je   TitleStepCheckDown
cmp  byte [menu_latch], 0
jne  TitleStepCheckEnter
xor  byte [menu_choice], 1
mov  byte [menu_latch], 1
jmp  TitleStepCheckEnter
TitleStepCheckDown:
cmp  byte [key_states+50h], 0
je   TitleStepClearMenuLatch
cmp  byte [menu_latch], 0
jne  TitleStepCheckEnter
xor  byte [menu_choice], 1
mov  byte [menu_latch], 1
jmp  TitleStepCheckEnter
TitleStepClearMenuLatch:
mov  byte [menu_latch], 0
TitleStepCheckEnter:
cmp  byte [key_states+1Ch], 0
je   TitleStepCheckEscape
cmp  byte [enter_latch], 0
jne  TitleStepDraw
mov  byte [enter_latch], 1
cmp  byte [menu_choice], 0
jne  TitleStepExit
call NewGame
mov  byte [game_state], STATE_PLAYING
jmp  TitleStepDraw
TitleStepCheckEscape:
mov  byte [enter_latch], 0
cmp  byte [key_states+01h], 0
je   TitleStepClearEscape
cmp  byte [escape_latch], 0
jne  TitleStepDraw
mov  byte [game_state], STATE_EXIT
jmp  TitleStepDraw
TitleStepClearEscape:
mov  byte [escape_latch], 0
jmp  TitleStepDraw
TitleStepExit:
mov  byte [game_state], STATE_EXIT
TitleStepDraw:
mov  ah, STATE_TITLE
mov  al, [menu_choice]
cmp  ax, [screen_key]
je   TitleStepDone
mov  [screen_key], ax
call DrawTitleScreen
TitleStepDone:
pop  ax
ret

; PlayingStep advances active gameplay for a single frame.
; It handles pause and escape keys first, saves Mario's prior position, and updates movement and entities.
; It also checks for pit/time death, flag pole contact, invulnerability cooldown, and draws the frame.
; Debug keys: T sets the clock to 5 seconds and C sets the coin counter to 99 (listed in the README).
; Inputs: key_states, all hero, enemy and timer variables.
; Outputs: one gameplay frame is simulated and drawn; game_state may change to PAUSED, TITLE, DYING or WIN_SEQUENCE.
; Registers changed: none (all registers are saved and restored).
PlayingStep:
push ax
push si
cmp  byte [key_states+19h], 0
je   PlayingStepPauseReleased
cmp  byte [pause_latch], 0
jne  PlayingStepCheckEscape
mov  byte [pause_latch], 1
mov  byte [game_state], STATE_PAUSED
call RenderFrame
mov  byte [screen_row], 11
mov  byte [screen_col], 7
mov  byte [screen_color], 14
mov  si, paused_text
call ScreenPrint
pop  si
pop  ax
ret
PlayingStepPauseReleased:
mov  byte [pause_latch], 0
PlayingStepCheckEscape:
cmp  byte [key_states+01h], 0
je   PlayingStepDebugTime
mov  byte [game_state], STATE_TITLE
mov  byte [menu_choice], 0
mov  byte [escape_latch], 1
pop  si
pop  ax
ret
PlayingStepDebugTime:
cmp  byte [key_states+14h], 0
je   PlayingStepDebugCoins
cmp  word [remaining_seconds], 5
jbe  PlayingStepDebugCoins
mov  word [remaining_seconds], 5
PlayingStepDebugCoins:
cmp  byte [key_states+2Eh], 0
je   PlayingStepUpdate
mov  byte [coin_count], 99
PlayingStepUpdate:
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
mov  [previous_feet_y], ax
mov  ax, [movement_x_int]
mov  [previous_x_int], ax
call UpdateMovement
cmp  byte [game_state], STATE_DYING
je   PlayingStepDrawDying
call UpdateEnemies
call UpdateMushroom
call ResolveEntityContacts
call CollectMapCoins
cmp  byte [game_state], STATE_DYING
je   PlayingStepDrawDying
cmp  word [remaining_seconds], 0
jne  PlayingStepCheckFlag
call BeginDeath
jmp  PlayingStepDrawDying
PlayingStepCheckFlag:
call CheckFlagContact
cmp  byte [invuln_ticks], 0
je   PlayingStepDraw
dec  byte [invuln_ticks]
PlayingStepDraw:
call RenderFrame
pop  si
pop  ax
ret
PlayingStepDrawDying:
call DrawDeathScene
pop  si
pop  ax
ret

; PausedStep handles controls while gameplay is paused.
; Pressing Escape resets latches and sends the player back to the title menu.
; Pressing the pause key again with a latch releases the pause and returns to active play.
; Inputs: key_states (P 19h, Esc 01h), pause_latch, escape_latch.
; Outputs: game_state returns to PLAYING (P) or TITLE (Esc).
; Registers changed: none (all registers are saved and restored).
PausedStep:
cmp  byte [key_states+01h], 0
je   PausedStepCheckPause
cmp  byte [escape_latch], 0
jne  PausedStepDone
mov  byte [escape_latch], 1
mov  byte [menu_choice], 0
mov  byte [pause_latch], 0
mov  byte [game_state], STATE_TITLE
ret
PausedStepCheckPause:
cmp  byte [key_states+19h], 0
je   PausedStepPauseRelease
cmp  byte [pause_latch], 0
jne  PausedStepShow
mov  byte [pause_latch], 1
mov  byte [game_state], STATE_PLAYING
ret
PausedStepPauseRelease:
mov  byte [pause_latch], 0
PausedStepShow:
PausedStepDone:
ret

; DyingStep plays out Mario's death animation after losing a life.
; It hops Mario upward for the first few frames and then lets him fall while redrawing the scene.
; Once the timer finishes, it decrements lives and moves to STATE_LIFE_LOST or STATE_GAME_OVER.
; Inputs: death_frames, lives, movement_y_fp.
; Outputs: the death animation advances; game_state becomes LIFE_LOST or GAME_OVER when it ends.
; Registers changed: none (all registers are saved and restored).
DyingStep:
inc  byte [death_frames]
cmp  byte [death_frames], 8
ja   DyingStepFall
sub  word [movement_y_fp], 512
jmp  DyingStepDraw
DyingStepFall:
cmp  byte [movement_y_fp+1], 208
jae  DyingStepDraw
add  word [movement_y_fp], 1024
DyingStepDraw:
call DrawDeathScene
cmp  byte [death_frames], 27
jb   DyingStepDone
cmp  byte [lives], 0
je   DyingStepGameOver
dec  byte [lives]
cmp  byte [lives], 0
je   DyingStepGameOver
mov  byte [life_lost_frames], 0
mov  byte [game_state], STATE_LIFE_LOST
DyingStepDone:
ret
DyingStepGameOver:
mov  byte [game_state], STATE_GAME_OVER
ret

; LifeLostStep shows the life-lost screen between respawns.
; It draws the screen once when entering the state and counts down life_lost_frames.
; After 36 frames, it resets the level and resumes active play.
; Inputs: life_lost_frames, screen_key.
; Outputs: the life lost screen is shown; ResetLevel and PLAYING follow after 36 frames.
; Registers changed: none (all registers are saved and restored).
LifeLostStep:
push ax
mov  ah, STATE_LIFE_LOST
xor  al, al
cmp  ax, [screen_key]
je   LifeLostStepCount
mov  [screen_key], ax
call DrawLifeLostScreen
LifeLostStepCount:
inc  byte [life_lost_frames]
cmp  byte [life_lost_frames], 36
jb   LifeLostStepDone
call ResetLevel
mov  byte [game_state], STATE_PLAYING
LifeLostStepDone:
pop  ax
ret

; WinSequenceStep runs the end-of-level sequence after touching the flag pole.
; It slides Mario horizontally toward win_target_x, then down the pole to its base, while updating the camera and rendering frames.
; Mario disappears after 18 frames (DrawMario hides him), standing in for the walk to the castle.
; When the sequence finishes, it calculates the time bonus, adds it to the score, and completes the level.
; Inputs: win_frames, win_target_x, movement variables, remaining_seconds, score.
; Outputs: Mario slides to the pole and down it; after 36 frames win_bonus is added to the score and game_state = LEVEL_COMPLETE.
; Registers changed: none (all registers are saved and restored).
WinSequenceStep:
push ax
push bx
push dx
inc  byte [win_frames]
mov  byte [movement_grounded], 1
mov  ax, [movement_x_int]
cmp  ax, [win_target_x]
je   WinSequenceStepAtFlag
ja   WinSequenceStepSlideLeft
inc  word [movement_x_int]
mov  byte [movement_facing_left], 0
mov  word [movement_vx], 0100h
jmp  WinSequenceStepSlideDown
WinSequenceStepSlideLeft:
dec  word [movement_x_int]
mov  byte [movement_facing_left], 1
mov  word [movement_vx], 0FF00h
jmp  WinSequenceStepSlideDown
WinSequenceStepAtFlag:
mov  word [movement_vx], 0
WinSequenceStepSlideDown:
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
cmp  ax, FLAG_POLE_BOTTOM_Y
jae  WinSequenceStepRender
add  word [movement_y_fp], 1536
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
cmp  ax, FLAG_POLE_BOTTOM_Y
jbe  WinSequenceStepRender
mov  ax, FLAG_POLE_BOTTOM_Y
sub  ax, [movement_height]
mov  ah, al
xor  al, al
mov  [movement_y_fp], ax
WinSequenceStepRender:
call UpdateCamera
call RenderFrame
cmp  byte [win_frames], 36
jb   WinSequenceStepDone
mov  ax, [remaining_seconds]
mov  bx, 50
mul  bx
mov  [win_bonus], ax
mov  bx, [score_low]
mov  [win_score_low], bx
mov  bx, [score_high]
mov  [win_score_high], bx
push ax
call AddScore
add  sp, 2
mov  byte [game_state], STATE_LEVEL_COMPLETE
WinSequenceStepDone:
pop  dx
pop  bx
pop  ax
ret

; LevelCompleteStep displays the level complete summary screen.
; It draws the screen once and waits for Enter to be released and pressed again.
; Pressing Enter starts a fresh game from the beginning.
; Inputs: key_states (Enter 1Ch), enter_latch, screen_key.
; Outputs: the summary screen is shown; Enter calls NewGame (back to the title).
; Registers changed: none (all registers are saved and restored).
LevelCompleteStep:
push ax
mov  ah, STATE_LEVEL_COMPLETE
xor  al, al
cmp  ax, [screen_key]
je   LevelCompleteStepInput
mov  [screen_key], ax
call DrawLevelCompleteScreen
LevelCompleteStepInput:
cmp  byte [key_states+1Ch], 0
jne  LevelCompleteStepPress
mov  byte [enter_latch], 0
pop  ax
ret
LevelCompleteStepPress:
cmp  byte [enter_latch], 0
jne  LevelCompleteStepDone
mov  byte [enter_latch], 1
call NewGame
LevelCompleteStepDone:
pop  ax
ret

; GameOverStep displays the game over screen once all lives are lost.
; It draws the screen once and tracks Enter key press/release using enter_latch.
; Pressing Enter triggers NewGame to let the player restart.
; Inputs: key_states (Enter 1Ch), enter_latch, screen_key.
; Outputs: the game over screen is shown; Enter calls NewGame (back to the title).
; Registers changed: none (all registers are saved and restored).
GameOverStep:
push ax
mov  ah, STATE_GAME_OVER
xor  al, al
cmp  ax, [screen_key]
je   GameOverStepInput
mov  [screen_key], ax
call DrawGameOverScreen
GameOverStepInput:
cmp  byte [key_states+1Ch], 0
jne  GameOverStepPress
mov  byte [enter_latch], 0
pop  ax
ret
GameOverStepPress:
cmp  byte [enter_latch], 0
jne  GameOverStepDone
mov  byte [enter_latch], 1
call NewGame
GameOverStepDone:
pop  ax
ret

; ResetLevel resets the current stage state for a fresh attempt.
; It restores the map, Mario's movement, enemies, mushroom state, hero size, and invulnerability/stomp counters.
; Finally, it resets the level timer back to its starting value.
; Inputs: none.
; Outputs: map, Mario, enemies, mushroom, hero form, invulnerability, stomp chain and clock are reset (score, coins and lives are kept).
; Registers changed: none (all registers are saved and restored).
ResetLevel:
call ResetMap
call ResetMovement
call ResetEnemies
mov  byte [mushroom_active], 0
mov  byte [mushroom_dir], 1
mov  word [mushroom_fall], 0
mov  byte [hero_form], 0
mov  word [movement_height], 16
mov  byte [invuln_ticks], 0
mov  byte [stomp_chain], 0
call ResetGameTimer
ret

; NewGame initializes overall run counters for a brand-new playthrough.
; It clears score, coins, hero form, and input latches, while setting lives to 3 and state to title.
; Then it calls ResetLevel to set up the map, actors, and timer.
; Inputs: none.
; Outputs: score, coins, lives, hero form, menu and latches are reset, game_state = TITLE, then ResetLevel runs.
; Registers changed: none (all registers are saved and restored).
NewGame:
mov  word [score_low], 0
mov  word [score_high], 0
mov  byte [coin_count], 0
mov  byte [lives], 3
mov  byte [hero_form], 0
mov  word [movement_height], 16
mov  byte [menu_choice], 0
mov  byte [menu_latch], 0
mov  byte [pause_latch], 0
mov  byte [escape_latch], 0
mov  byte [game_state], STATE_TITLE
call ResetLevel
ret

; BeginDeath puts the game into the dying state once.
; It sets game_state to STATE_DYING and resets death_frames to start the animation.
; Mario's horizontal and vertical velocities are zeroed and grounded is cleared.
; Inputs: game_state.
; Outputs: game_state = STATE_DYING, death_frames = 0, Mario's speeds are cleared (ignored when already dying).
; Registers changed: none (all registers are saved and restored).
BeginDeath:
cmp  byte [game_state], STATE_DYING
je   BeginDeathDone
mov  byte [game_state], STATE_DYING
mov  byte [death_frames], 0
mov  word [movement_vx], 0
mov  word [movement_vy], 0
mov  byte [movement_grounded], 0
BeginDeathDone:
ret

; GameExitStep serves as the exit handler in GameStateTable.
; It immediately returns so the outer loop can finish and exit cleanly.
; Inputs: none.
; Outputs: none.
; Registers changed: none (all registers are saved and restored).
GameExitStep:
ret

GameStateTable:
dw TitleStep
dw PlayingStep
dw PausedStep
dw DyingStep
dw LifeLostStep
dw WinSequenceStep
dw LevelCompleteStep
dw GameOverStep
dw GameExitStep

game_state db STATE_TITLE
menu_choice db 0
menu_latch db 0
enter_latch db 0
pause_latch db 0
escape_latch db 0
previous_feet_y dw 0
previous_x_int dw 0
death_frames db 0
life_lost_frames db 0
win_frames db 0
win_target_x dw 0
win_bonus dw 0
win_score_low dw 0
win_score_high dw 0
