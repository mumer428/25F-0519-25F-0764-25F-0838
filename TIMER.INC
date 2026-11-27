; TIMER.INC - timer interrupt, tick counter and the 300 second clock.
; Owner: Nafia - Movement and Physics (Module 8).

; InstallTimer hooks BIOS timer interrupt 1Ch so the game can drive tick and clock updates.
; It fetches the existing INT 1Ch vector via DOS INT 21h (AX=351Ch) and saves its segment and offset.
; Then it registers TimerISR as the custom handler using DOS INT 21h (AX=251Ch).
; Inputs: none.
; Outputs: old_timer_offset/old_timer_segment hold the old vector and INT 1Ch points to TimerISR.
; Registers changed: none (all registers are saved and restored).
InstallTimer:
push ax
push bx
push dx
push ds
push es

push cs
pop  ds
mov  ax, 351Ch
int  21h
mov  [old_timer_offset], bx
mov  [old_timer_segment], es

mov  dx, TimerISR
mov  ax, 251Ch
int  21h

pop  es
pop  ds
pop  dx
pop  bx
pop  ax
ret

; RestoreTimer puts the original BIOS timer interrupt 1Ch handler back before exit.
; It loads the saved offset and segment into DS:DX from old_timer_offset and old_timer_segment.
; Finally, it calls DOS INT 21h (AX=251Ch) to restore standard timer behavior.
; Inputs: old_timer_offset, old_timer_segment.
; Outputs: INT 1Ch points to the original timer handler again.
; Registers changed: none (all registers are saved and restored).
RestoreTimer:
push ax
push dx
push ds

push cs
pop  ds
mov  dx, [old_timer_offset]
mov  ax, [old_timer_segment]
mov  ds, ax
mov  ax, 251Ch
int  21h

pop  ds
pop  dx
pop  ax
ret

; TimerISR runs on every ~18.2 Hz BIOS timer tick to advance counters and countdown time.
; It increments timer_ticks, and during STATE_PLAYING accumulates subticks to decrement remaining_seconds.
; Before returning with IRET, it chains to the previous timer interrupt handler via a far call.
; Inputs: game_state, clock_subticks, remaining_seconds.
; Outputs: timer_ticks is increased every tick; while playing, clock_subticks and remaining_seconds count the 300 second clock.
; Registers changed: none (all registers are saved and restored).
TimerISR:
push ax
push ds
push cs
pop  ds

inc  word [timer_ticks]

cmp  byte [game_state], STATE_PLAYING
jne  TimerISRChain
add  word [clock_subticks], 549
cmp  word [clock_subticks], 10000
jb   TimerISRChain
sub  word [clock_subticks], 10000
cmp  word [remaining_seconds], 0
je   TimerISRChain
dec  word [remaining_seconds]

TimerISRChain:
pushf
call far [cs:old_timer_offset]

pop  ds
pop  ax
iret

; ResetGameTimer restarts the level clock at 300 seconds.
; It clears timer_ticks and clock_subticks, sets remaining_seconds, and syncs last_tick so no extra frame runs.
; Inputs: none.
; Outputs: timer_ticks, clock_subticks = 0, remaining_seconds = 300 and last_tick = timer_ticks.
; Registers changed: none (all registers are saved and restored).
ResetGameTimer:
push ax
mov  word [timer_ticks], 0
mov  word [clock_subticks], 0
mov  word [remaining_seconds], 300
mov  ax, [timer_ticks]
mov  [last_tick], ax
pop  ax
ret

old_timer_offset dw 0
old_timer_segment dw 0
timer_ticks      dw 0
clock_subticks   dw 0
remaining_seconds dw 300
last_tick dw 0
