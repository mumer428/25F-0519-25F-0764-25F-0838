; mario.asm - program entry, memory setup, main loop and clean shutdown.
; Owner: Umer - Game Logic (Module 1, Module 15 exit).

CPU 8086
BITS 16
ORG 100h

jmp start

%include "CONSTANT.INC"
%include "VIDEO.INC"
%include "DRAW.INC"
%include "LEVEL.INC"
%include "RENDER.INC"
%include "SCREENS.INC"
%include "INPUT.INC"
%include "TIMER.INC"
%include "MOVEMENT.INC"
%include "ITEMS.INC"
%include "ENEMIES.INC"
%include "GAME.INC"
%include "CONTACTS.INC"
%include "SCORE.INC"
%include "HiSCORE.INC"

start:
push cs
pop  ds
mov  ax, cs
mov  ss, ax
mov  sp, StackTop
mov  ax, cs
mov  es, ax
mov  bx, StackTop
add  bx, 15
mov  cl, 4
shr  bx, cl
mov  ah, 4Ah
int  21h
jc   startAllocationFailure

mov  bx, 1000h
mov  ah, 48h
int  21h
jc   startAllocationFailure
mov  [frame_segment], ax
mov  [draw_segment], ax

call LoadHighScore
call NewGame
call InstallKeyboard
call InstallTimer
call VideoMode13
call SetPalette
call InitFont
call GameStep

startMainAgain:
cmp  byte [game_state], STATE_EXIT
je   startShutdown
mov  ax, [timer_ticks]
cmp  ax, [last_tick]
je   startMainAgain
mov  [last_tick], ax
xor  byte [movement_frame], 1
call GameStep
jmp  startMainAgain

startShutdown:
call RestoreTimer
call RestoreKeyboard
call UpdateHighScore
call SaveHighScore
call VideoTextMode
mov  ax, [frame_segment]
mov  es, ax
mov  ah, 49h
int  21h
mov  ax, 4C00h
int  21h

startAllocationFailure:
mov  ax, 4C01h
int  21h

frame_segment dw 0
draw_segment  dw 0
stack_space   times 1024 db 0
StackTop:
