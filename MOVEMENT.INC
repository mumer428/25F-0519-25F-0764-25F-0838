; MOVEMENT.INC - Mario physics, tile collision and the camera (8.8 fixed point).
; Owner: Nafia- Movement and Physics (Modules 6, 9, 10).

WALK_ACCEL       equ 0057h
GROUND_FRICTION  equ 002Bh
WALK_MAX_SPEED   equ 0279h
RUN_MAX_SPEED    equ 041Fh
GRAVITY_AMOUNT   equ 0080h
JUMP_START_SPEED equ 0F7CDh
JUMP_CUTOFF      equ 0FD00h
MAX_FALL_SPEED   equ 00500h
CAMERA_MAX_X     equ 640
CAMERA_FOLLOW_X  equ 160
MOVEMENT_X_MAX   equ 944

; UpdateMovement processes horizontal input, acceleration, friction, and jumping for Mario.
; It updates fixed-point horizontal position, clamps level bounds, and resolves side collisions.
; Finally, it checks ground support, triggers jumps on Space, and updates vertical physics and camera.
; Inputs: key_states (Left 4Bh, Right 4Dh, Shift 2Ah/36h, Space 39h) and all movement_* variables.
; Outputs: movement_x_int/x_frac, movement_y_fp, movement_vx, movement_vy, movement_grounded, movement_facing_left and camera_x are updated.
; Registers changed: none (all registers are saved and restored).
UpdateMovement:
push ax
push bx
push cx
push dx

mov  al, [key_states+4Bh]
mov  ah, [key_states+4Dh]
cmp  al, ah
je   UpdateMovementApplyFriction
test al, al
jnz  UpdateMovementMoveLeft

mov  byte [movement_facing_left], 0
mov  bx, WALK_MAX_SPEED
cmp  byte [key_states+2Ah], 0
jne  UpdateMovementRightRunLimit
cmp  byte [key_states+36h], 0
je   UpdateMovementRightLimitReady
UpdateMovementRightRunLimit:
mov  bx, RUN_MAX_SPEED
UpdateMovementRightLimitReady:
add  word [movement_vx], WALK_ACCEL
mov  ax, [movement_vx]
test ax, 8000h
jnz  UpdateMovementIntegrateHorizontal
cmp  ax, bx
jle  UpdateMovementIntegrateHorizontal
mov  [movement_vx], bx
jmp  UpdateMovementIntegrateHorizontal

UpdateMovementMoveLeft:
mov  byte [movement_facing_left], 1
mov  bx, WALK_MAX_SPEED
cmp  byte [key_states+2Ah], 0
jne  UpdateMovementLeftRunLimit
cmp  byte [key_states+36h], 0
je   UpdateMovementLeftLimitReady
UpdateMovementLeftRunLimit:
mov  bx, RUN_MAX_SPEED
UpdateMovementLeftLimitReady:
sub  word [movement_vx], WALK_ACCEL
mov  ax, [movement_vx]
test ax, 8000h
jz   UpdateMovementIntegrateHorizontal
not  bx
inc  bx
cmp  ax, bx
jge  UpdateMovementIntegrateHorizontal
mov  [movement_vx], bx
jmp  UpdateMovementIntegrateHorizontal

UpdateMovementApplyFriction:
mov  ax, [movement_vx]
test ax, ax
jz   UpdateMovementIntegrateHorizontal
test ax, 8000h
jnz  UpdateMovementFrictionNegative
cmp  ax, GROUND_FRICTION
jbe  UpdateMovementStopHorizontal
sub  ax, GROUND_FRICTION
jmp  UpdateMovementStoreHorizontal
UpdateMovementFrictionNegative:
cmp  ax, -GROUND_FRICTION
jge  UpdateMovementStopHorizontal
add  ax, GROUND_FRICTION
jmp  UpdateMovementStoreHorizontal
UpdateMovementStopHorizontal:
xor  ax, ax
UpdateMovementStoreHorizontal:
mov  [movement_vx], ax

UpdateMovementIntegrateHorizontal:
mov  ax, [movement_vx]
add  byte [movement_x_frac], al
jnc  UpdateMovementNoFractionCarry
inc  word [movement_x_int]
UpdateMovementNoFractionCarry:
mov  al, ah
xor  ah, ah
test al, 80h
jz   UpdateMovementSignReady1
mov  ah, 0FFh
UpdateMovementSignReady1:
add  word [movement_x_int], ax

cmp  word [movement_x_int], MOVEMENT_X_MAX
jbe  UpdateMovementHorizontalBoundsOk
test word [movement_vx], 8000h
jnz  UpdateMovementHorizontalClampLeft
mov  word [movement_x_int], MOVEMENT_X_MAX
mov  byte [movement_x_frac], 0
mov  word [movement_vx], 0
jmp  UpdateMovementHorizontalBoundsOk
UpdateMovementHorizontalClampLeft:
mov  word [movement_x_int], 0
mov  byte [movement_x_frac], 0
mov  word [movement_vx], 0
UpdateMovementHorizontalBoundsOk:
call ResolveHorizontalCollision

cmp  byte [movement_grounded], 0
je   UpdateMovementProcessJump
call CheckGroundSupport

UpdateMovementProcessJump:
cmp  byte [key_states+39h], 0
je   UpdateMovementSpaceReleased
cmp  byte [movement_space_previous], 0
jne  UpdateMovementSpaceHeld
cmp  byte [movement_grounded], 0
je   UpdateMovementSpaceHeld
mov  word [movement_vy], JUMP_START_SPEED
mov  byte [movement_grounded], 0
UpdateMovementSpaceHeld:
mov  byte [movement_space_previous], 1
jmp  UpdateMovementCheckAir
UpdateMovementSpaceReleased:
mov  byte [movement_space_previous], 0

UpdateMovementCheckAir:
cmp  byte [movement_grounded], 0
jne  UpdateMovementUpdateCamera
call UpdateVertical

UpdateMovementUpdateCamera:
call UpdateCamera

pop  dx
pop  cx
pop  bx
pop  ax
ret

; ResolveHorizontalCollision prevents Mario from walking into solid level tiles.
; When moving right, it probes upper and lower points along the right edge and snaps to the tile face.
; When moving left, it probes along the left edge similarly and clears horizontal velocity on contact.
; Inputs: movement_x_int, movement_vx, movement_y_fp, movement_height.
; Outputs: Mario is snapped against a solid wall and movement_vx is cleared when he hits one.
; Registers changed: none (all registers are saved and restored).
ResolveHorizontalCollision:
push ax
push bx
push cx
push dx

mov  ax, [movement_vx]
test ax, ax
jnz  ResolveHorizontalCollisionHasVelocity
jmp  ResolveHorizontalCollisionDone
ResolveHorizontalCollisionHasVelocity:
test ax, 8000h
jz   ResolveHorizontalCollisionMovingRight
jmp  ResolveHorizontalCollisionMovingLeft

ResolveHorizontalCollisionMovingRight:
mov  ax, [movement_x_int]
add  ax, 15
mov  dx, ax
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, 2
push ax
push dx
call IsSolidAt
add  sp, 4
test ax, ax
jnz  ResolveHorizontalCollisionHitRight

mov  ax, [movement_x_int]
add  ax, 15
mov  dx, ax
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
sub  ax, 2
push ax
push dx
call IsSolidAt
add  sp, 4
test ax, ax
jnz  ResolveHorizontalCollisionRightLowerHit
jmp  ResolveHorizontalCollisionDone

ResolveHorizontalCollisionRightLowerHit:
ResolveHorizontalCollisionHitRight:
mov  ax, [movement_x_int]
add  ax, 15
and  ax, 0FFF0h
sub  ax, 16
mov  [movement_x_int], ax
mov  byte [movement_x_frac], 0
mov  word [movement_vx], 0
jmp  ResolveHorizontalCollisionDone

ResolveHorizontalCollisionMovingLeft:
mov  ax, [movement_x_int]
mov  dx, ax
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, 2
push ax
push dx
call IsSolidAt
add  sp, 4
test ax, ax
jnz  ResolveHorizontalCollisionHitLeft

mov  ax, [movement_x_int]
mov  dx, ax
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
sub  ax, 2
push ax
push dx
call IsSolidAt
add  sp, 4
test ax, ax
jnz  ResolveHorizontalCollisionLeftLowerHit
jmp  ResolveHorizontalCollisionDone

ResolveHorizontalCollisionLeftLowerHit:
ResolveHorizontalCollisionHitLeft:
mov  ax, [movement_x_int]
and  ax, 0FFF0h
add  ax, 16
mov  [movement_x_int], ax
mov  byte [movement_x_frac], 0
mov  word [movement_vx], 0

ResolveHorizontalCollisionDone:
pop  dx
pop  cx
pop  bx
pop  ax
ret

; CheckGroundSupport verifies whether solid ground is still beneath Mario while he is grounded.
; It tests two points just below Mario's feet and clears movement_grounded if neither is solid.
; If supported, it snaps Y neatly to the tile top, zeroes vertical speed, and resets stomp_chain.
; Inputs: movement_x_int, movement_y_fp, movement_height.
; Outputs: movement_grounded stays 1 and Y is snapped to the tile top, or movement_grounded becomes 0 when the floor is gone.
; Registers changed: none (all registers are saved and restored).
CheckGroundSupport:
push ax
push bx
push cx

xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
mov  bx, ax
mov  ax, [movement_x_int]
add  ax, 2
push bx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jnz  CheckGroundSupportSupported

xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
mov  bx, ax
mov  ax, [movement_x_int]
add  ax, 13
push bx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jnz  CheckGroundSupportSupported

mov  byte [movement_grounded], 0
jmp  CheckGroundSupportDone

CheckGroundSupportSupported:
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
and  ax, 0FFF0h
sub  ax, [movement_height]
mov  cl, 8
shl  ax, cl
mov  [movement_y_fp], ax
mov  word [movement_vy], 0
mov  byte [movement_grounded], 1
mov  byte [stomp_chain], 0

CheckGroundSupportDone:
pop  cx
pop  bx
pop  ax
ret

; UpdateVertical applies gravity, jump cut-off, vertical integration, and ceiling/floor collisions.
; When rising and Space is released early, upward velocity is clamped at JUMP_CUTOFF.
; It triggers blocks on ceiling hits, handles landing on floors, and calls BeginDeath on falling into a pit.
; Inputs: key_states (Space), movement_vy, movement_y_fp, movement_x_int, movement_height.
; Outputs: gravity, variable jump height, ceiling hits, landing (grounded, stomp_chain = 0) and pit death are applied.
; Registers changed: none (all registers are saved and restored).
UpdateVertical:
push ax
push bx
push cx
push dx

cmp  byte [key_states+39h], 0
jne  UpdateVerticalAddGravity
mov  ax, [movement_vy]
test ax, 8000h
jz   UpdateVerticalAddGravity
cmp  ax, JUMP_CUTOFF
jge  UpdateVerticalAddGravity
mov  word [movement_vy], JUMP_CUTOFF

UpdateVerticalAddGravity:
add  word [movement_vy], GRAVITY_AMOUNT
cmp  word [movement_vy], MAX_FALL_SPEED
jle  UpdateVerticalIntegrateY
mov  word [movement_vy], MAX_FALL_SPEED

UpdateVerticalIntegrateY:
mov  ax, [movement_y_fp]
add  ax, [movement_vy]
test word [movement_vy], 8000h
jz   UpdateVerticalStoreY
cmp  ah, 240
jb   UpdateVerticalStoreY
xor  ax, ax
mov  word [movement_vy], 0
UpdateVerticalStoreY:
mov  [movement_y_fp], ax
test word [movement_vy], 8000h
jnz  UpdateVerticalResolveCeiling
jmp  UpdateVerticalResolveFloor

UpdateVerticalResolveCeiling:
xor  ax, ax
mov  al, [movement_y_fp+1]
mov  dx, ax
mov  ax, [movement_x_int]
add  ax, 2
push dx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jnz  UpdateVerticalHitCeiling

xor  ax, ax
mov  al, [movement_y_fp+1]
mov  dx, ax
mov  ax, [movement_x_int]
add  ax, 13
push dx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jnz  UpdateVerticalHitCeiling
jmp  UpdateVerticalDone

UpdateVerticalHitCeiling:
call TriggerQuestionBlock
xor  ax, ax
mov  al, [movement_y_fp+1]
and  ax, 0FFF0h
add  ax, 16
mov  cl, 8
shl  ax, cl
mov  [movement_y_fp], ax
mov  word [movement_vy], 0
jmp  UpdateVerticalDone

UpdateVerticalResolveFloor:
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
mov  dx, ax
mov  ax, [movement_x_int]
add  ax, 2
push dx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jnz  UpdateVerticalHitFloor

xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
mov  dx, ax
mov  ax, [movement_x_int]
add  ax, 13
push dx
push ax
call IsSolidAt
add  sp, 4
test ax, ax
jnz  UpdateVerticalHitFloor

cmp  byte [movement_y_fp+1], 192
jae  UpdateVerticalPitReached
jmp  UpdateVerticalDone
UpdateVerticalPitReached:
call BeginDeath
jmp  UpdateVerticalDone

UpdateVerticalHitFloor:
xor  ax, ax
mov  al, [movement_y_fp+1]
add  ax, [movement_height]
and  ax, 0FFF0h
sub  ax, [movement_height]
mov  cl, 8
shl  ax, cl
mov  [movement_y_fp], ax
mov  word [movement_vy], 0
mov  byte [movement_grounded], 1
mov  byte [stomp_chain], 0

UpdateVerticalDone:
pop  dx
pop  cx
pop  bx
pop  ax
ret

; TriggerQuestionBlock is a thin wrapper invoked when Mario's head hits a tile from below.
; It calls HandleBlockHit to process question blocks, power-ups, or breakable bricks.
; Keeping this helper makes ceiling collision handling clean and modular.
; Inputs: Mario's head position (through HandleBlockHit).
; Outputs: the block above Mario's head is processed.
; Registers changed: none (all registers are saved and restored).
TriggerQuestionBlock:
call HandleBlockHit
ret

; IsSolidAt checks whether the level tile at pixel position (X=[bp+4], Y=[bp+6]) is solid.
; It fetches the tile ID via GetTile and tests if it falls within the solid range 1..10.
; Returns AX = 1 if solid, or AX = 0 if non-solid.
; Inputs: [bp+4] = x, [bp+6] = y (world pixels).
; Outputs: AX = 1 if the tile is solid (codes 1 to 10), otherwise AX = 0.
; Registers changed: AX holds the result; every other register is preserved.
IsSolidAt:
push bp
mov  bp, sp
push bx

push word [bp+6]
push word [bp+4]
call GetTile
add  sp, 4

cmp  ax, 1
jb   IsSolidAtNotSolid
cmp  ax, 10
ja   IsSolidAtNotSolid
mov  ax, 1
jmp  IsSolidAtDone
IsSolidAtNotSolid:
xor  ax, ax
IsSolidAtDone:
pop  bx
pop  bp
ret

; UpdateCamera prevents Mario from moving left past camera_x and scrolls the view forward.
; If Mario passes CAMERA_FOLLOW_X relative to the camera, camera_x advances up to CAMERA_MAX_X.
; It only moves camera_x forward so scrolling remains strictly one-way.
; Inputs: movement_x_int, camera_x.
; Outputs: camera_x only moves forward (never past 640) and Mario is stopped at the left screen edge.
; Registers changed: none (all registers are saved and restored).
UpdateCamera:
push ax
push bx

mov  ax, [movement_x_int]
cmp  ax, [camera_x]
jae  UpdateCameraCheckFollow
mov  ax, [camera_x]
mov  [movement_x_int], ax
mov  byte [movement_x_frac], 0
mov  word [movement_vx], 0

UpdateCameraCheckFollow:
mov  ax, [movement_x_int]
sub  ax, [camera_x]
cmp  ax, CAMERA_FOLLOW_X
jbe  UpdateCameraDone
mov  ax, [movement_x_int]
sub  ax, CAMERA_FOLLOW_X
cmp  ax, CAMERA_MAX_X
jbe  UpdateCameraCandidateReady
mov  ax, CAMERA_MAX_X
UpdateCameraCandidateReady:
cmp  ax, [camera_x]
jbe  UpdateCameraDone
mov  [camera_x], ax

UpdateCameraDone:
pop  bx
pop  ax
ret

; ResetMovement sets Mario's position, velocities, and camera back to level start defaults.
; It places Mario at X=32, Y=144 (in fixed-point), grounded on the floor, facing right.
; It also clears horizontal/vertical speeds, jump latch, and camera_x.
; Inputs: none.
; Outputs: Mario's position, speeds, flags and camera_x are set to their starting values.
; Registers changed: none (all registers are saved and restored).
ResetMovement:
mov  word [movement_x_int], 32
mov  byte [movement_x_frac], 0
mov  word [movement_y_fp], 24*256
mov  word [movement_vx], 0
mov  word [movement_vy], 0
mov  byte [movement_grounded], 1
mov  byte [movement_space_previous], 0
mov  byte [movement_facing_left], 0
mov  word [camera_x], 0
ret

movement_x_int          dw 32
movement_x_frac         db 0
movement_y_fp           dw 24*256
movement_vx             dw 0
movement_vy             dw 0
movement_grounded       db 1
movement_space_previous db 0
movement_facing_left    db 0
movement_frame          db 0
camera_x                dw 0
movement_height dw 16
hero_form db 0
invuln_ticks db 0
stomp_chain db 0
