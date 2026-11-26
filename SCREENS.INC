; SCREENS.INC - title, life lost, level complete, game over and dying screens.
; Owner: Member 3 - Graphics and Drawing (Module 14 (drawing part)).

; ScreenPrint writes a null-terminated string at screen_row and screen_col via BIOS.
; It positions the cursor with INT 10h AH=02h first.
; Then it loops through each character using teletype output AH=0Eh until hitting zero.
; Inputs: SI = string address, screen_row, screen_col, screen_color.
; Outputs: the string is printed with BIOS teletype output directly on the screen.
ScreenPrint:
push ax
push bx
push dx
push si
mov  ah, 02h
xor  bh, bh
mov  dh, [screen_row]
mov  dl, [screen_col]
int  10h
ScreenPrintContinue:
mov  al, [si]
inc  si
test al, al
jz   ScreenPrintDone
mov  ah, 0Eh
xor  bh, bh
mov  bl, [screen_color]
int  10h
jmp  ScreenPrintContinue
ScreenPrintDone:
pop  si
pop  dx
pop  bx
pop  ax
ret

; DrawDeathScene renders the death frame and displays the life-lost message.
; It invalidates screen_key, clears the frame, draws Mario, and copies it to video memory.
; Then it sets the BIOS text position and prints dying_text.
; Inputs: Mario's variables.
; Outputs: one dying frame (Mario on a blue background and the dying text) is shown.
; Registers changed: none (all registers are saved and restored).
DrawDeathScene:
push ax
push si
mov  word [screen_key], 0FFFFh
mov  ax, 1
push ax
call ClearFrame
add  sp, 2
call DrawMario
call CopyFrame
mov  byte [screen_row], 10
mov  byte [screen_col], 13
mov  byte [screen_color], 12
mov  si, dying_text
call ScreenPrint
pop  si
pop  ax
ret

; DrawTitleScreen draws the title menu background, sprites, logo, and options.
; It builds brick borders, ground/shadow rectangles, Mario and Goomba sprites, and centered titles.
; It also highlights the selected menu item with an arrow and copies the completed frame.
; Inputs: menu_choice.
; Outputs: the complete title screen is drawn and copied to the screen.
; Registers changed: none (all registers are saved and restored).
DrawTitleScreen:
push ax
push bx
push cx

mov ax, COLOR_SKY
push ax
call ClearFrame
add sp, 2

xor  bx, bx
DrawTitleScreenBrickRows:
mov  cl, 4
mov  ax, TILE_BRICK
push ax
xor  ax, ax
push ax
mov  ax, bx
shl  ax, cl
push ax
call DrawTile
add  sp, 6
mov  ax, TILE_BRICK
push ax
mov  ax, 176
push ax
mov  ax, bx
shl  ax, cl
push ax
call DrawTile
add  sp, 6
inc  bx
cmp  bx, 20
jb   DrawTitleScreenBrickRows

mov  bx, 1
DrawTitleScreenBrickColumns:
mov  cl, 4
mov  ax, TILE_BRICK
push ax
mov  ax, bx
shl  ax, cl
push ax
xor  ax, ax
push ax
call DrawTile
add  sp, 6
mov  ax, TILE_BRICK
push ax
mov  ax, bx
shl  ax, cl
push ax
mov  ax, 304
push ax
call DrawTile
add  sp, 6
inc  bx
cmp  bx, 11
jb   DrawTitleScreenBrickColumns

mov ax, 10
push ax
mov ax, 4
push ax
mov ax, 288
push ax
mov ax, 160
push ax
mov ax, 16
push ax
call FillRect
add sp, 10
mov ax, 6
push ax
mov ax, 12
push ax
mov ax, 288
push ax
mov ax, 164
push ax
mov ax, 16
push ax
call FillRect
add sp, 10

xor  ax, ax
push ax
mov  ax, MarioStandShape
push ax
mov  ax, 144
push ax
mov  ax, 40
push ax
call DrawSprite
add  sp, 8
xor  ax, ax
push ax
mov  ax, GoombaShape
push ax
mov  ax, 144
push ax
mov  ax, 250
push ax
call DrawSprite
add  sp, 8

mov ax, title_logo_text
push ax
mov ax, REDORANGE
push ax
mov ax, 2
push ax
mov ax, 42
push ax
mov ax, 72
push ax
call DrawTextOutlined
add sp, 10
mov  word [text_spacing], 8
mov ax, title_engine_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 72
push ax
call DrawTextCentered
add sp, 8
mov  word [text_spacing], 0

mov ax, 15
push ax
mov ax, 52
push ax
mov ax, 128
push ax
mov ax, 100
push ax
mov ax, 96
push ax
call FillRect
add sp, 10
mov ax, 0
push ax
mov ax, 48
push ax
mov ax, 124
push ax
mov ax, 102
push ax
mov ax, 98
push ax
call FillRect
add sp, 10
cmp  byte [menu_choice], 0
jne  DrawTitleScreenExitSelected
mov ax, arrow_text
push ax
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 112
push ax
mov ax, 108
push ax
call DrawText
add sp, 10
mov ax, start_game_text
push ax
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 112
push ax
mov ax, 124
push ax
call DrawText
add sp, 10
mov ax, exit_game_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 132
push ax
mov ax, 124
push ax
call DrawText
add sp, 10
jmp  DrawTitleScreenMenuDone
DrawTitleScreenExitSelected:
mov ax, start_game_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 112
push ax
mov ax, 124
push ax
call DrawText
add sp, 10
mov ax, arrow_text
push ax
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 132
push ax
mov ax, 108
push ax
call DrawText
add sp, 10
mov ax, exit_game_text
push ax
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 132
push ax
mov ax, 124
push ax
call DrawText
add sp, 10
DrawTitleScreenMenuDone:
mov ax, team_text
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 166
push ax
call DrawTextCentered
add sp, 8

call CopyFrame
pop  cx
pop  bx
pop  ax
ret

; DrawLifeLostScreen displays how many lives remain before respawning.
; It clears the screen, prints the world heading, and draws a scaled Mario sprite.
; Then it formats "x <lives>" beside Mario, prints the remaining lives label, and copies the frame.
; Inputs: lives.
; Outputs: the life lost screen is drawn and copied to the screen.
; Registers changed: none (all registers are saved and restored).
DrawLifeLostScreen:
push ax
push bx

mov ax, 0
push ax
call ClearFrame
add sp, 2
mov ax, world_text
push ax
mov ax, 15
push ax
mov ax, 2
push ax
mov ax, 36
push ax
call DrawTextCentered
add sp, 8

mov  ax, 2
push ax
mov  ax, MarioStandShape
push ax
mov  ax, 80
push ax
mov  ax, 60
push ax
call DrawSpriteScaled
add  sp, 8

mov  ax, line_buffer
mov bx, times_text
push bx
push ax
call AppendText
add sp, 4
xor  bx, bx
mov  bl, [lives]
push bx
push ax
call AppendNumber
add sp, 4
mov ax, line_buffer
push ax
mov ax, 15
push ax
mov ax, 2
push ax
mov ax, 88
push ax
mov ax, 160
push ax
call DrawText
add sp, 10

mov ax, lives_left_text
push ax
mov ax, 14
push ax
mov ax, 2
push ax
mov ax, 148
push ax
call DrawTextCentered
add sp, 8

call CopyFrame
pop  bx
pop  ax
ret

; DrawLevelCompleteScreen renders the end-of-level score and bonus breakdown.
; It draws a framed navy panel and displays score, coins, time left, time bonus, and final score.
; At the bottom, it shows a prompt to press Enter to continue and copies the frame to screen.
; Inputs: win_score_low/high, coin_count, remaining_seconds, win_bonus, score.
; Outputs: the level complete summary is drawn and copied to the screen.
; Registers changed: none (all registers are saved and restored).
DrawLevelCompleteScreen:
push ax
push bx

mov ax, NAVY_COLOR
push ax
call ClearFrame
add sp, 2
mov ax, 14
push ax
mov ax, 168
push ax
mov ax, 288
push ax
mov ax, 16
push ax
mov ax, 16
push ax
call FillRect
add sp, 10
mov ax, NAVY_COLOR
push ax
mov ax, 164
push ax
mov ax, 284
push ax
mov ax, 18
push ax
mov ax, 18
push ax
call FillRect
add sp, 10
mov ax, level_complete_text
push ax
mov ax, 14
push ax
mov ax, 2
push ax
mov ax, 36
push ax
call DrawTextCentered
add sp, 8

mov ax, lbl_score
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 70
push ax
mov ax, 56
push ax
call DrawText
add sp, 10
push word [win_score_high]
push word [win_score_low]
call BuildScoreFrom
add  sp, 4
mov ax, number_buffer
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 70
push ax
mov ax, 160
push ax
call DrawText
add sp, 10

mov ax, lbl_coins
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 88
push ax
mov ax, 56
push ax
call DrawText
add sp, 10
mov  ax, line_buffer
mov bx, times_one_text
push bx
push ax
call AppendText
add sp, 4
xor  bx, bx
mov  bl, [coin_count]
push bx
push ax
call AppendNumber
add sp, 4
mov ax, line_buffer
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 88
push ax
mov ax, 160
push ax
call DrawText
add sp, 10

mov ax, lbl_time_left
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 106
push ax
mov ax, 56
push ax
call DrawText
add sp, 10
mov  ax, line_buffer
mov  bx, [remaining_seconds]
push bx
push ax
call AppendNumber
add sp, 4
mov bx, times_fifty_text
push bx
push ax
call AppendText
add sp, 4
mov  bx, [win_bonus]
push bx
push ax
call AppendNumber
add sp, 4
mov ax, line_buffer
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 106
push ax
mov ax, 160
push ax
call DrawText
add sp, 10

mov ax, lbl_time_bonus
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 124
push ax
mov ax, 56
push ax
call DrawText
add sp, 10
mov  ax, line_buffer
mov bx, plus_text
push bx
push ax
call AppendText
add sp, 4
mov  bx, [win_bonus]
push bx
push ax
call AppendNumber
add sp, 4
mov ax, line_buffer
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 124
push ax
mov ax, 160
push ax
call DrawText
add sp, 10

mov ax, lbl_final_score
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 142
push ax
mov ax, 56
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
mov ax, 142
push ax
mov ax, 160
push ax
call DrawText
add sp, 10

mov ax, press_enter_continue
push ax
mov ax, 10
push ax
mov ax, 1
push ax
mov ax, 168
push ax
call DrawTextCentered
add sp, 8

call CopyFrame
pop  bx
pop  ax
ret

; DrawGameOverScreen renders the game over summary screen.
; It clears the frame, draws a bordered box, and centers the GAME OVER heading.
; Then it builds and displays final score, coins, remaining time, and the Enter prompt before copying.
; Inputs: score, coin_count, remaining_seconds.
; Outputs: the game over screen is drawn and copied to the screen.
; Registers changed: none (all registers are saved and restored).
DrawGameOverScreen:
push ax
push bx
push di

mov ax, 0
push ax
call ClearFrame
add sp, 2
mov ax, REDORANGE
push ax
mov ax, 150
push ax
mov ax, 260
push ax
mov ax, 26
push ax
mov ax, 30
push ax
call FillRect
add sp, 10
mov ax, 0
push ax
mov ax, 146
push ax
mov ax, 256
push ax
mov ax, 28
push ax
mov ax, 32
push ax
call FillRect
add sp, 10

mov  word [text_spacing], 8
mov ax, game_over_text
push ax
mov ax, REDORANGE
push ax
mov ax, 2
push ax
mov ax, 62
push ax
call DrawTextCentered
add sp, 8
mov  word [text_spacing], 0

call BuildScoreString
mov  ax, line_buffer
mov bx, final_score_text
push bx
push ax
call AppendText
add sp, 4
mov bx, number_buffer
push bx
push ax
call AppendText
add sp, 4
mov ax, line_buffer
push ax
mov ax, 15
push ax
mov ax, 1
push ax
mov ax, 99
push ax
call DrawTextCentered
add sp, 8

mov  ax, line_buffer
mov bx, coins_label_text
push bx
push ax
call AppendText
add sp, 4
xor  bx, bx
mov  bl, [coin_count]
push bx
push ax
call AppendNumber
add sp, 4
mov bx, time_label_text
push bx
push ax
call AppendText
add sp, 4
push ax
mov  ax, [remaining_seconds]
mov  di, small_number_buffer
mov  bx, 3
call NumberToFixedString
pop  ax
mov bx, small_number_buffer
push bx
push ax
call AppendText
add sp, 4
mov ax, line_buffer
push ax
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 119
push ax
call DrawTextCentered
add sp, 8

call DrawHighScore

mov ax, press_enter_title
push ax
mov ax, 10
push ax
mov ax, 1
push ax
mov ax, 149
push ax
call DrawTextCentered
add sp, 8

call CopyFrame
pop  di
pop  bx
pop  ax
ret

screen_row db 0
screen_col db 0
screen_color db 15
paused_text db 'PAUSED - PRESS P TO RESUME',0
dying_text db 'MARIO LOST A LIFE',0
title_logo_text      db 'SUPER MARIO',0
title_engine_text    db 'ENGINE',0
start_game_text      db 'START GAME',0
exit_game_text       db 'EXIT',0
arrow_text           db '>',0
team_text            db ' Umer, Rehan, Nafia',0
world_text           db 'WORLD 1-1',0
times_text           db 'x  ',0
lives_left_text      db 'LIVES LEFT',0
level_complete_text  db 'LEVEL COMPLETE!',0
lbl_score            db 'SCORE',0
lbl_coins            db 'COINS',0
lbl_time_left        db 'TIME LEFT',0
lbl_time_bonus       db 'TIME BONUS',0
lbl_final_score      db 'FINAL SCORE',0
times_one_text       db 'x ',0
times_fifty_text     db '  x 50 = ',0
plus_text            db '+ ',0
press_enter_continue db 'Press ENTER to continue',0
game_over_text       db 'GAME OVER',0
final_score_text     db 'Final Score: ',0
coins_label_text     db 'Coins: ',0
time_label_text      db '     Time: ',0
press_enter_title    db 'Press ENTER for the title',0
