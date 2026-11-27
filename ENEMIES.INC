; ENEMIES.INC - Goomba, Koopa and shell behaviour.
; Owner: Nafia - Movement and Physics (Module 11).

; ResetEnemies initializes all enemy slots in enemy_table.
; It sets up Goombas and Koopas with their starting positions, directions, walk states, and active flags.
; Timers and animation phases are cleared so every enemy starts cleanly.
; Inputs: none.
; Outputs: all six records of enemy_table hold their starting values.
; Registers changed: none (all registers are saved and restored).
ResetEnemies:
mov  byte [enemy_table+0*ENEMY_SIZE+E_TYPE], E_GOOMBA
mov  byte [enemy_table+0*ENEMY_SIZE+E_STATE], E_WALK
mov  word [enemy_table+0*ENEMY_SIZE+E_X], 160
mov  word [enemy_table+0*ENEMY_SIZE+E_Y], 144
mov  byte [enemy_table+0*ENEMY_SIZE+E_DIR], 1
mov  byte [enemy_table+0*ENEMY_SIZE+E_TIMER], 0
mov  byte [enemy_table+0*ENEMY_SIZE+E_ACTIVE], 1
mov  byte [enemy_table+0*ENEMY_SIZE+E_PHASE], 0

mov  byte [enemy_table+1*ENEMY_SIZE+E_TYPE], E_GOOMBA
mov  byte [enemy_table+1*ENEMY_SIZE+E_STATE], E_WALK
mov  word [enemy_table+1*ENEMY_SIZE+E_X], 280
mov  word [enemy_table+1*ENEMY_SIZE+E_Y], 144
mov  byte [enemy_table+1*ENEMY_SIZE+E_DIR], 0
mov  byte [enemy_table+1*ENEMY_SIZE+E_TIMER], 0
mov  byte [enemy_table+1*ENEMY_SIZE+E_ACTIVE], 1
mov  byte [enemy_table+1*ENEMY_SIZE+E_PHASE], 0

mov  byte [enemy_table+2*ENEMY_SIZE+E_TYPE], E_KOOPA
mov  byte [enemy_table+2*ENEMY_SIZE+E_STATE], E_WALK
mov  word [enemy_table+2*ENEMY_SIZE+E_X], 360
mov  word [enemy_table+2*ENEMY_SIZE+E_Y], 144
mov  byte [enemy_table+2*ENEMY_SIZE+E_DIR], 0
mov  byte [enemy_table+2*ENEMY_SIZE+E_TIMER], 0
mov  byte [enemy_table+2*ENEMY_SIZE+E_ACTIVE], 1
mov  byte [enemy_table+2*ENEMY_SIZE+E_PHASE], 0

mov  byte [enemy_table+3*ENEMY_SIZE+E_TYPE], E_GOOMBA
mov  byte [enemy_table+3*ENEMY_SIZE+E_STATE], E_WALK
mov  word [enemy_table+3*ENEMY_SIZE+E_X], 520
mov  word [enemy_table+3*ENEMY_SIZE+E_Y], 144
mov  byte [enemy_table+3*ENEMY_SIZE+E_DIR], 1
mov  byte [enemy_table+3*ENEMY_SIZE+E_TIMER], 0
mov  byte [enemy_table+3*ENEMY_SIZE+E_ACTIVE], 1
mov  byte [enemy_table+3*ENEMY_SIZE+E_PHASE], 0

mov  byte [enemy_table+4*ENEMY_SIZE+E_TYPE], E_KOOPA
mov  byte [enemy_table+4*ENEMY_SIZE+E_STATE], E_WALK
mov  word [enemy_table+4*ENEMY_SIZE+E_X], 680
mov  word [enemy_table+4*ENEMY_SIZE+E_Y], 144
mov  byte [enemy_table+4*ENEMY_SIZE+E_DIR], 0
mov  byte [enemy_table+4*ENEMY_SIZE+E_TIMER], 0
mov  byte [enemy_table+4*ENEMY_SIZE+E_ACTIVE], 1
mov  byte [enemy_table+4*ENEMY_SIZE+E_PHASE], 0

mov  byte [enemy_table+5*ENEMY_SIZE+E_TYPE], E_GOOMBA
mov  byte [enemy_table+5*ENEMY_SIZE+E_STATE], E_WALK
mov  word [enemy_table+5*ENEMY_SIZE+E_X], 840
mov  word [enemy_table+5*ENEMY_SIZE+E_Y], 144
mov  byte [enemy_table+5*ENEMY_SIZE+E_DIR], 0
mov  byte [enemy_table+5*ENEMY_SIZE+E_TIMER], 0
mov  byte [enemy_table+5*ENEMY_SIZE+E_ACTIVE], 1
mov  byte [enemy_table+5*ENEMY_SIZE+E_PHASE], 0
ret

; UpdateEnemies steps enemy timers, movement, collisions, and sliding shell knockouts.
; It expires flattened Goombas, removes offscreen enemies, slides shells, and turns walkers at walls, ledges, or other enemies.
; After updating positions, sliding shells check for hits against other active enemies and award points.
; Inputs: enemy_table, camera_x.
; Outputs: enemies walk, fall, turn, slide or expire; sliding shells remove the enemies they touch (+500 each).
; Registers changed: none (all registers are saved and restored).
UpdateEnemies:
push ax
push bx
push cx
push dx
push si
push di

mov  si, enemy_table
mov  cx, ENEMY_COUNT
UpdateEnemiesAgain:
cmp  byte [si+E_ACTIVE], 0
jne  UpdateEnemiesActiveEntity
jmp  near UpdateEnemiesContinue
UpdateEnemiesActiveEntity:
cmp  byte [si+E_STATE], E_FLAT
jne  UpdateEnemiesNotFlat
dec  byte [si+E_TIMER]
jz   UpdateEnemiesFlatExpired
jmp  near UpdateEnemiesContinue
UpdateEnemiesFlatExpired:
mov  byte [si+E_ACTIVE], 0
jmp  UpdateEnemiesContinue
UpdateEnemiesNotFlat:
cmp  byte [si+E_STATE], E_SHELL
jne  UpdateEnemiesNotStationaryShell
jmp  near UpdateEnemiesContinue
UpdateEnemiesNotStationaryShell:
mov  ax, [si+E_X]
cmp  ax, [camera_x]
jae  UpdateEnemiesNotBehind
mov  bx, [camera_x]
sub  bx, ax
cmp  bx, 32
jbe  UpdateEnemiesNotBehind
jmp  near UpdateEnemiesRemove
UpdateEnemiesNotBehind:
mov  ax, [si+E_X]
sub  ax, [camera_x]
cmp  ax, 336
jbe  UpdateEnemiesWithinView
jmp  near UpdateEnemiesContinue
UpdateEnemiesWithinView:
cmp  byte [si+E_STATE], E_SLIDE
jne  UpdateEnemiesWalk
cmp  byte [si+E_DIR], 0
jne  UpdateEnemiesSlideRight
sub  word [si+E_X], 5
jmp  UpdateEnemiesSlideWall
UpdateEnemiesSlideRight:
add  word [si+E_X], 5
UpdateEnemiesSlideWall:
cmp  byte [si+E_TIMER], 0
je   UpdateEnemiesSlideMoveCheck
dec  byte [si+E_TIMER]
UpdateEnemiesSlideMoveCheck:
mov  ax, [si+E_X]
add  ax, 8
xor  dx, dx
mov  dl, [si+E_Y]
add  dx, 8
push dx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jnz  UpdateEnemiesShellBlocked
jmp  near UpdateEnemiesContinue
UpdateEnemiesShellBlocked:
xor  byte [si+E_DIR], 1
jmp  near UpdateEnemiesContinue
UpdateEnemiesWalk:
inc  byte [si+E_PHASE]
test byte [si+E_PHASE], 2
jz   UpdateEnemiesWalkStep
jmp  near UpdateEnemiesContinue
UpdateEnemiesWalkStep:
cmp  byte [si+E_DIR], 0
jne  UpdateEnemiesWalkRight
sub  word [si+E_X], 1
jmp  near UpdateEnemiesWalkCheck
UpdateEnemiesWalkRight:
add  word [si+E_X], 1
UpdateEnemiesWalkCheck:
mov  ax, [si+E_X]
cmp  byte [si+E_DIR], 0
je   UpdateEnemiesWallLeft
add  ax, 15
jmp  UpdateEnemiesCheckWall
UpdateEnemiesWallLeft:
dec  ax
UpdateEnemiesCheckWall:
xor  dx, dx
mov  dl, [si+E_Y]
add  dx, 8
push dx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jnz  UpdateEnemiesTurn
push si
call EnemyBlockedByEnemy
add  sp, 2
test ax, ax
jnz  UpdateEnemiesTurn
cmp  byte [si+E_TYPE], E_KOOPA
jne  UpdateEnemiesGoombaFloor
mov  ax, [si+E_X]
cmp  byte [si+E_DIR], 0
je   UpdateEnemiesLedgeLeft
add  ax, 15
jmp  UpdateEnemiesLedgeCheck
UpdateEnemiesLedgeLeft:
dec  ax
UpdateEnemiesLedgeCheck:
xor  dx, dx
mov  dl, [si+E_Y]
add  dx, 17
push dx
push ax
call GetTile
add  sp, 4
test ax, ax
jnz  UpdateEnemiesContinue
UpdateEnemiesTurn:
xor  byte [si+E_DIR], 1
jmp  UpdateEnemiesContinue
UpdateEnemiesGoombaFloor:
mov  ax, [si+E_X]
add  ax, 8
xor  dx, dx
mov  dl, [si+E_Y]
add  dx, 17
push dx
push ax
call GetTile
add  sp, 4
test ax, ax
jnz  UpdateEnemiesContinue
add  byte [si+E_Y], 1
cmp  byte [si+E_Y], 192
jae  UpdateEnemiesRemove
jmp  UpdateEnemiesContinue
UpdateEnemiesRemove:
mov  byte [si+E_ACTIVE], 0
UpdateEnemiesContinue:
add  si, ENEMY_SIZE
dec  cx
jz   UpdateEnemiesEnemyUpdatesDone
jmp  UpdateEnemiesAgain

UpdateEnemiesEnemyUpdatesDone:
mov  si, enemy_table
mov  cx, ENEMY_COUNT
UpdateEnemiesShellAgain:
cmp  byte [si+E_ACTIVE], 0
je   UpdateEnemiesShellContinue
cmp  byte [si+E_STATE], E_SLIDE
jne  UpdateEnemiesShellContinue
push si
mov  di, enemy_table
mov  bx, ENEMY_COUNT
UpdateEnemiesHitAgain:
cmp  byte [di+E_ACTIVE], 0
je   UpdateEnemiesHitContinue
cmp  si, di
je   UpdateEnemiesHitContinue
cmp  byte [di+E_STATE], E_FLAT
je   UpdateEnemiesHitContinue
mov  ax, [si+E_X]
add  ax, 16
cmp  ax, [di+E_X]
jbe  UpdateEnemiesHitContinue
mov  ax, [di+E_X]
add  ax, 16
cmp  ax, [si+E_X]
jbe  UpdateEnemiesHitContinue
mov  ax, [si+E_Y]
add  ax, 16
cmp  ax, [di+E_Y]
jbe  UpdateEnemiesHitContinue
mov  ax, [di+E_Y]
add  ax, 16
cmp  ax, [si+E_Y]
jbe  UpdateEnemiesHitContinue
mov  byte [di+E_ACTIVE], 0
mov  ax, 500
push ax
call AddScore
add  sp, 2
UpdateEnemiesHitContinue:
add  di, ENEMY_SIZE
dec  bx
jnz  UpdateEnemiesHitAgain
pop  si
UpdateEnemiesShellContinue:
add  si, ENEMY_SIZE
dec  cx
jnz  UpdateEnemiesShellAgain
pop  di
pop  si
pop  dx
pop  cx
pop  bx
pop  ax
ret

; EnemyBlockedByEnemy checks whether another walking enemy or resting shell stands directly in front of an enemy.
; It looks only in the enemy's facing direction, within 16 pixels horizontally and less than 16 pixels vertically.
; Inputs: [bp+4] = address of the enemy record being tested.
; Outputs: AX = 1 if blocked, AX = 0 otherwise.
; Registers changed: AX holds the result; every other register is preserved.
EnemyBlockedByEnemy:
push bp
mov  bp, sp
push bx
push cx
push si
push di

mov  si, [bp+4]
mov  di, enemy_table
mov  cx, ENEMY_COUNT
xor  bx, bx
EnemyBlockedByEnemyAgain:
cmp  si, di
je   EnemyBlockedByEnemyContinue
cmp  byte [di+E_ACTIVE], 0
je   EnemyBlockedByEnemyContinue
cmp  byte [di+E_STATE], E_WALK
je   EnemyBlockedByEnemyRow
cmp  byte [di+E_STATE], E_SHELL
jne  EnemyBlockedByEnemyContinue
EnemyBlockedByEnemyRow:
mov  ax, [si+E_Y]
sub  ax, [di+E_Y]
add  ax, 15
cmp  ax, 30
ja   EnemyBlockedByEnemyContinue
cmp  byte [si+E_DIR], 0
je   EnemyBlockedByEnemyLeft
mov  ax, [di+E_X]
sub  ax, [si+E_X]
jmp  EnemyBlockedByEnemyDistance
EnemyBlockedByEnemyLeft:
mov  ax, [si+E_X]
sub  ax, [di+E_X]
EnemyBlockedByEnemyDistance:
jbe  EnemyBlockedByEnemyContinue
cmp  ax, 16
ja   EnemyBlockedByEnemyContinue
mov  bx, 1
jmp  EnemyBlockedByEnemyDone
EnemyBlockedByEnemyContinue:
add  di, ENEMY_SIZE
dec  cx
jnz  EnemyBlockedByEnemyAgain
EnemyBlockedByEnemyDone:
mov  ax, bx

pop  di
pop  si
pop  cx
pop  bx
pop  bp
ret

enemy_table times ENEMY_COUNT*ENEMY_SIZE db 0
