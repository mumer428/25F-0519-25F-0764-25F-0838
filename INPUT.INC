; INPUT.INC - keyboard interrupt handler and the key-state table.
; Owner: Nafia - Movement and Physics (Module 7).

; InstallKeyboard hooks BIOS interrupt 09h so the game can track real-time key presses and releases.
; It queries the original INT 09h vector via DOS INT 21h (AX=3509h) and saves its segment and offset.
; Then it installs KeyboardISR as the active handler using DOS INT 21h (AX=2509h).
; Inputs: none.
; Outputs: old_int09_offset/old_int09_segment hold the old vector and INT 09h points to KeyboardISR.
; Registers changed: none (all registers are saved and restored).
InstallKeyboard:
push ax
push bx
push dx
push ds
push es

push cs
pop  ds
mov  ax, 3509h
int  21h
mov  [old_int09_offset], bx
mov  [old_int09_segment], es

mov  dx, KeyboardISR
mov  ax, 2509h
int  21h

pop  es
pop  ds
pop  dx
pop  bx
pop  ax
ret

; RestoreKeyboard puts the original BIOS keyboard interrupt handler back before exiting.
; It loads the saved offset and segment into DS:DX from old_int09_offset and old_int09_segment.
; Finally, it calls DOS INT 21h (AX=2509h) to restore normal keyboard handling.
; Inputs: old_int09_offset, old_int09_segment.
; Outputs: INT 09h points to the original keyboard handler again.
; Registers changed: none (all registers are saved and restored).
RestoreKeyboard:
push ax
push dx
push ds

push cs
pop  ds
mov  dx, [old_int09_offset]
mov  ax, [old_int09_segment]
mov  ds, ax
mov  ax, 2509h
int  21h

pop  ds
pop  dx
pop  ax
ret

; KeyboardISR is the custom interrupt handler triggered whenever a hardware key event occurs.
; It reads the scan code from port 60h, skips multi-byte prefixes (0E0h/0E1h), and updates key_states.
; Bit 7 distinguishes press vs. release, and port 20h is sent EOI (20h) before returning with IRET.
; Inputs: the scan code read from port 60h.
; Outputs: key_states entry set to 1 on press and 0 on release (E0h/E1h prefixes ignored); EOI sent to port 20h.
; Registers changed: none (all registers are saved and restored).
KeyboardISR:
push ax
push bx
push ds
push cs
pop  ds

in   al, 60h
cmp  al, 0E0h
je   KeyboardISRAcknowledge
cmp  al, 0E1h
je   KeyboardISRAcknowledge
xor  bx, bx
mov  bl, al
test bl, 80h
jnz  KeyboardISRKeyRelease

mov  byte [key_states+bx], 1
jmp  KeyboardISRAcknowledge

KeyboardISRKeyRelease:
and  bl, 7Fh
mov  byte [key_states+bx], 0
jmp  KeyboardISRAcknowledge

KeyboardISRAcknowledge:
mov  al, 20h
out  20h, al
pop  ds
pop  bx
pop  ax
iret

old_int09_offset dw 0
old_int09_segment dw 0
key_states times 128 db 0

