; VIDEO.INC - switching between VGA mode 13h and text mode.
; Owner: Member 1 - Graphics and Drawing (Module 2).

; VideoMode13 switches the display into standard 320x200 256-color VGA graphics mode.
; It saves AX on the stack and loads 0013h into AX for the BIOS video service.
; Calling INT 10h activates mode 13h, and AX is restored before returning.
; Inputs: none.
; Outputs: the screen is switched to 320x200, 256 colors (BIOS mode 13h).
; Registers changed: none (all registers are saved and restored).
VideoMode13:
push ax
mov  ax, 0013h
int  10h
pop  ax
ret

; VideoTextMode switches the display back to standard 80x25 color text mode.
; It preserves AX, sets AX to 0003h, and invokes BIOS video interrupt 10h.
; Finally, it restores AX and returns control to the caller.
; Inputs: none.
; Outputs: the screen is switched back to 80x25 text mode (BIOS mode 03h).
; Registers changed: none (all registers are saved and restored).
VideoTextMode:
push ax
mov  ax, 0003h
int  10h
pop  ax
ret
