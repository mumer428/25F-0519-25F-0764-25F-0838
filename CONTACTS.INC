; CONTACTS.INC - Mario against enemies and the flag: stomp, kick, damage, flag points.
; Owner: Member 3 - Game Logic (Modules 10 (entity part), 11 (damage and stomp rules)).

; ResolveEntityContacts handles collisions between Mario, enemies, and power-up mushrooms.
; Overlapping Mario with enemies supports stomping, kicking shells, stopping sliding shells, or taking damage.
; Overlapping an active mushroom consumes it, awards 1000 points, and grows small Mario into Super Mario.
; Inputs: enemy_table, mushroom_*, Mario's position, speed and previous_feet_y.
; Outputs: stomps, kicks, damage, mushroom pickup and scoring are applied.
; Registers changed: none (all registers are saved and restored).
ResolveEntityContacts:
push ax
push bx
push cx
push dx
push si

mov  si, enemy_table
mov  cx, ENEMY_COUNT
ResolveEntityContactsEnemyAgain:
cmp  byte [si+E_ACTIVE], 0
jne  ResolveEntityContactsActiveContact
jmp  near ResolveEntityContactsContinue
ResolveEntityContactsActiveContact:
mov  ax, [movement_x_int]
push ax
xor  ax, ax
mov  al, [movement_y_fp+1]
push ax
mov  ax, 16
push ax
mov  ax, [movement_height]
push ax
mov  ax, [si+E_X]
push ax
xor  ax, ax
mov  al, [si+E_Y]
push ax
mov  ax, 16
push ax
push ax
call RectOverlap
add  sp, 16
test ax, ax
jnz  ResolveEntityContactsOverlapping
jmp  near ResolveEntityContactsContinue
ResolveEntityContactsOverlapping:
cmp  byte [si+E_STATE], E_FLAT
jne  ResolveEntityContactsNotFlatContact
jmp  near ResolveEntityContactsContinue
ResolveEntityContactsNotFlatContact:
cmp  byte [si+E_STATE], E_SHELL
je   ResolveEntityContactsKickShell
cmp  byte [si+E_STATE], E_SLIDE
jne  ResolveEntityContactsCheckStomp
cmp  byte [si+E_TIMER], 0
je   ResolveEntityContactsCheckStomp
jmp  near ResolveEntityContactsContinue
ResolveEntityContactsCheckStomp:
cmp  word [movement_vy], 0
jle  ResolveEntityContactsHurtOrShell
mov  ax, [previous_feet_y]
mov  bx, [si+E_Y]
add  bx, 8
cmp  ax, bx
jae  ResolveEntityContactsHurtOrShell
cmp  byte [si+E_STATE], E_SLIDE
je   ResolveEntityContactsStopShell
cmp  byte [si+E_TYPE], E_KOOPA
je   ResolveEntityContactsMakeShell
mov  byte [si+E_STATE], E_FLAT
mov  byte [si+E_TIMER], 18
jmp  ResolveEntityContactsStompScore
ResolveEntityContactsMakeShell:
mov  byte [si+E_STATE], E_SHELL
ResolveEntityContactsStompScore:
call AwardStomp
call StompBounce
jmp  ResolveEntityContactsContinue
ResolveEntityContactsStopShell:
mov  byte [si+E_STATE], E_SHELL
mov  byte [si+E_TIMER], 0
call AwardStomp
call StompBounce
jmp  ResolveEntityContactsContinue
ResolveEntityContactsKickShell:
mov  byte [si+E_STATE], E_SLIDE
mov  byte [si+E_TIMER], 8
mov  ax, 400
push ax
call AddScore
add  sp, 2
mov  ax, [movement_x_int]
cmp  ax, [si+E_X]
jae  ResolveEntityContactsKickLeft
mov  byte [si+E_DIR], 1
jmp  ResolveEntityContactsContinue
ResolveEntityContactsKickLeft:
mov  byte [si+E_DIR], 0
jmp  ResolveEntityContactsContinue
ResolveEntityContactsHurtOrShell:
cmp  byte [si+E_STATE], E_SLIDE
jne  ResolveEntityContactsDamage
cmp  byte [si+E_TIMER], 0
jne  ResolveEntityContactsContinue
ResolveEntityContactsDamage:
call HurtMario
ResolveEntityContactsContinue:
add  si, ENEMY_SIZE
dec  cx
jz   ResolveEntityContactsEnemyContactsDone
jmp  ResolveEntityContactsEnemyAgain

ResolveEntityContactsEnemyContactsDone:
cmp  byte [mushroom_active], 0
je   ResolveEntityContactsDone
mov  ax, [movement_x_int]
add  ax, 16
cmp  ax, [mushroom_x]
jbe  ResolveEntityContactsDone
mov  ax, [mushroom_x]
add  ax, 16
cmp  ax, [movement_x_int]
jbe  ResolveEntityContactsDone
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
cmp  ax, [mushroom_y]
jbe  ResolveEntityContactsDone
mov  ax, [mushroom_y]
add  ax, 16
xor  bx, bx
mov  bl, [movement_y_fp+1]
cmp  ax, bx
jbe  ResolveEntityContactsDone
mov  byte [mushroom_active], 0
mov  ax, 1000
push ax
call AddScore
add  sp, 2
cmp  byte [hero_form], 0
jne  ResolveEntityContactsDone
mov  byte [hero_form], 1
mov  word [movement_height], 32
sub  word [movement_y_fp], 4096
ResolveEntityContactsDone:
pop  si
pop  dx
pop  cx
pop  bx
pop  ax
ret

; RectOverlap tests whether two axis-aligned boxes on the stack intersect.
; It compares horizontal and vertical spans to rule out gaps between the two rectangles.
; Returns AX = 1 if they overlap, or AX = 0 otherwise.
; Inputs: [bp+4] = B width, [bp+6] = B height, [bp+8] = B x, [bp+10] = B y, [bp+12] = A width, [bp+14] = A height, [bp+16] = A x, [bp+18] = A y.
; Outputs: AX = 1 if the rectangles overlap, otherwise AX = 0.
; Registers changed: AX holds the result; every other register is preserved.
RectOverlap:
push bp
mov  bp, sp
push bx
mov  ax, [bp+18]
mov  bx, [bp+10]
add  bx, [bp+6]
cmp  ax, bx
jae  RectOverlapNoOverlap
mov  ax, [bp+18]
add  ax, [bp+14]
cmp  ax, [bp+10]
jbe  RectOverlapNoOverlap
mov  ax, [bp+16]
mov  bx, [bp+8]
add  bx, [bp+4]
cmp  ax, bx
jae  RectOverlapNoOverlap
mov  ax, [bp+16]
add  ax, [bp+12]
cmp  ax, [bp+8]
jbe  RectOverlapNoOverlap
mov  ax, 1
jmp  RectOverlapDone
RectOverlapNoOverlap:
xor  ax, ax
RectOverlapDone:
pop  bx
pop  bp
ret

; StompBounce launches Mario upward after he stomps an enemy.
; It gives a normal bounce, or a higher one while the Space key is held, and marks Mario as airborne.
; Inputs: key_states (Space, scan code 39h).
; Outputs: movement_vy is set (higher while Space is held) and movement_grounded = 0.
; Registers changed: none (all registers are saved and restored).
StompBounce:
mov  word [movement_vy], 0FB00h
cmp  byte [key_states+39h], 0
je   StompBounceDone
mov  word [movement_vy], 0F900h
StompBounceDone:
mov  byte [movement_grounded], 0
ret

; HurtMario applies damage when Mario is not currently invulnerable.
; If Mario is big, it shrinks him back to small form, shifts Y down, and grants invulnerability ticks.
; If Mario is already small, it triggers BeginDeath.
; Inputs: invuln_ticks, hero_form.
; Outputs: Super Mario shrinks and blinks for 36 frames, small Mario dies (nothing while invulnerable).
; Registers changed: none (all registers are saved and restored).
HurtMario:
cmp  byte [invuln_ticks], 0
jne  HurtMarioDone
cmp  byte [hero_form], 0
je   HurtMarioDeath
mov  byte [hero_form], 0
mov  word [movement_height], 16
add  word [movement_y_fp], 4096
mov  byte [invuln_ticks], 36
ret
HurtMarioDeath:
call BeginDeath
HurtMarioDone:
ret

; AwardStomp rewards consecutive enemy stomps during a single jump/air sequence.
; It advances stomp_chain and adds points from stomp_scores up to 10 consecutive stomps.
; Beyond that streak, it grants an extra life up to the cap of 99 lives.
; Inputs: stomp_chain, stomp_scores, lives.
; Outputs: stomp_chain is increased and its points (or an extra life after 10 stomps) are awarded.
; Registers changed: none (all registers are saved and restored).
AwardStomp:
push ax
push bx
cmp  byte [stomp_chain], 10
jae  AwardStompExtraLife
inc  byte [stomp_chain]
xor  bx, bx
mov  bl, [stomp_chain]
dec  bx
shl  bx, 1
mov  ax, [stomp_scores+bx]
push ax
call AddScore
add  sp, 2
pop  bx
pop  ax
ret
AwardStompExtraLife:

cmp  byte [lives], 99
jae  AwardStompDone
inc  byte [lives]
AwardStompDone:
pop  bx
pop  ax
ret

; CheckFlagContact checks if Mario crosses the flag pole from either side.
; It verifies vertical overlap with the pole and switches to STATE_WIN_SEQUENCE on contact.
; Depending on how high Mario reaches on the pole, it awards bonus points.
; Inputs: previous_x_int, movement_x_int, movement_y_fp, movement_height, previous_feet_y.
; Outputs: on contact game_state = WIN_SEQUENCE, win_target_x is set and the height bonus is added to the score.
; Registers changed: none (all registers are saved and restored).
CheckFlagContact:
push ax
push bx
push dx
mov  ax, [previous_x_int]
cmp  ax, [movement_x_int]
jne  CheckFlagContactDifferentX
jmp  CheckFlagContactDone
CheckFlagContactDifferentX:
ja   CheckFlagContactMovingLeft

add  ax, 16
cmp  ax, FLAG_POLE_LEFT_X
jbe  CheckFlagContactRightPreviousOk
jmp  CheckFlagContactDone
CheckFlagContactRightPreviousOk:
mov  ax, [movement_x_int]
add  ax, 16
cmp  ax, FLAG_POLE_LEFT_X
ja   CheckFlagContactRightContact
jmp  CheckFlagContactDone
CheckFlagContactRightContact:
jmp  CheckFlagContactVerticalCheck

CheckFlagContactMovingLeft:
cmp  ax, FLAG_POLE_RIGHT_X
jae  CheckFlagContactLeftPreviousOk
jmp  CheckFlagContactDone
CheckFlagContactLeftPreviousOk:
mov  ax, [movement_x_int]
cmp  ax, FLAG_POLE_RIGHT_X
jb   CheckFlagContactLeftContact
jmp  CheckFlagContactDone
CheckFlagContactLeftContact:

CheckFlagContactVerticalCheck:
xor  ax, ax
mov  al, [movement_y_fp+1]
cmp  ax, FLAG_POLE_BOTTOM_Y
jb   CheckFlagContactVerticalInsideBottom
jmp  CheckFlagContactDone
CheckFlagContactVerticalInsideBottom:
mov  bx, ax
add  bx, [movement_height]
cmp  bx, FLAG_POLE_TOP_Y
ja   CheckFlagContactBodyBelowTop
jmp  CheckFlagContactDone
CheckFlagContactBodyBelowTop:
cmp  word [previous_feet_y], FLAG_POLE_TOP_Y
ja   CheckFlagContactNotEnteringFromAbove
jmp  CheckFlagContactDone
CheckFlagContactNotEnteringFromAbove:

mov  byte [game_state], STATE_WIN_SEQUENCE
mov  byte [win_frames], 0
mov  ax, [movement_x_int]
add  ax, 8
and  ax, 0FFF0h
sub  ax, 8
mov  [win_target_x], ax
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
shr  ax, 1
shr  ax, 1
shr  ax, 1
shr  ax, 1
cmp  ax, 3
jbe  CheckFlagContactTopScore
cmp  ax, 4
jbe  CheckFlagContactScore2000
cmp  ax, 5
jbe  CheckFlagContactScore800
cmp  ax, 7
jbe  CheckFlagContactScore400
mov  ax, 100
jmp  CheckFlagContactAward
CheckFlagContactTopScore:
mov  ax, 5000
jmp  CheckFlagContactAward
CheckFlagContactScore2000:
mov  ax, 2000
jmp  CheckFlagContactAward
CheckFlagContactScore800:
mov  ax, 800
jmp  CheckFlagContactAward
CheckFlagContactScore400:
mov  ax, 400
CheckFlagContactAward:
push ax
call AddScore
add  sp, 2
CheckFlagContactDone:
pop  dx
pop  bx
pop  ax
ret

stomp_scores dw 100,200,400,500,800,1000,2000,4000,5000,8000
