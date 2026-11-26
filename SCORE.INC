; SCORE.INC - score, coins, lives and number-to-text conversion.
; Owner: Member 3 - Game Logic (Module 13).

; AddCoin increments the player's coin count and handles 100-coin rollovers.
; If coin_count reaches 100, it wraps back to 0 and awards an extra life.
; Lives are capped at 99 so the counter stays within two digits.
; Inputs: coin_count, lives.
; Outputs: coin_count + 1; at 100 coins it returns to 0 and lives + 1 (maximum 99).
; Registers changed: none (all registers are saved and restored).
AddCoin:
cmp  byte [coin_count], 99
jae  AddCoinBonusLife
inc  byte [coin_count]
cmp  byte [coin_count], 100
jb   AddCoinDone
AddCoinBonusLife:
mov  byte [coin_count], 0
cmp  byte [lives], 99
jae  AddCoinDone
inc  byte [lives]
AddCoinDone:
ret

; AddScore adds a 16-bit point value to the 32-bit score in score_high:score_low.
; It carries overflow from score_low into score_high when addition wraps.
; Finally, it clamps the score at the maximum allowed value (15:423Fh).
; Inputs: [bp+4] = points to add.
; Outputs: the 32-bit score (score_high:score_low) is increased with ADD/ADC and clamped to 999999.
; Registers changed: none (all registers are saved and restored).
AddScore:
push bp
mov  bp, sp
push ax
push dx
mov  ax, [bp+4]
add  [score_low], ax
jnc  AddScoreScoreNoCarry
inc  word [score_high]
AddScoreScoreNoCarry:
cmp  word [score_high], 15
ja   AddScoreClamp
jb   AddScoreDone
cmp  word [score_low], 423Fh
jbe  AddScoreDone
AddScoreClamp:
mov  word [score_high], 15
mov  word [score_low], 423Fh
AddScoreDone:
call UpdateHighScore
pop  dx
pop  ax
pop  bp
ret

; BuildScoreString prepares the current 32-bit score for display.
; It pushes score_high and score_low onto the stack.
; Then it delegates formatting to BuildScoreFrom and cleans up the stack.
; Inputs: score_low, score_high.
; Outputs: number_buffer holds the 6 digit score text.
; Registers changed: none (all registers are saved and restored).
BuildScoreString:
push word [score_high]
push word [score_low]
call BuildScoreFrom
add  sp, 4
ret

; BuildScoreFrom converts a 32-bit number on the stack into a 6-digit decimal string.
; It repeatedly divides score_work_hi:score_work_low by 10 to extract digits from right to left.
; Each remainder is converted to ASCII and stored in number_buffer with a null terminator.
; Inputs: [bp+4] = low word, [bp+6] = high word of a 32-bit value.
; Outputs: number_buffer holds the 6 digit zero padded decimal text.
; Registers changed: none (all registers are saved and restored).
BuildScoreFrom:
push bp
mov  bp, sp
push ax
push bx
push cx
push dx
push di
mov  ax, [bp+4]
mov  [score_work_low], ax
mov  ax, [bp+6]
mov  [score_work_hi], ax
mov  di, number_buffer+5
mov  byte [number_buffer+6], 0
mov  cx, 6
BuildScoreFromAgain:
mov  ax, [score_work_hi]
xor  dx, dx
mov  bx, 10
div  bx
mov  [score_work_hi], ax
mov  ax, [score_work_low]
div  bx
mov  [score_work_low], ax
add  dl, '0'
mov  [di], dl
dec  di
dec  cx
jnz  BuildScoreFromAgain
pop  di
pop  dx
pop  cx
pop  bx
pop  ax
pop  bp
ret

; BuildCoinString formats the coin counter into small_number_buffer.
; It loads coin_count into AX and requests a 2-digit width.
; Then it calls NumberToFixedString to fill the buffer.
; Inputs: coin_count.
; Outputs: small_number_buffer holds the 2 digit coin text.
; Registers changed: none (all registers are saved and restored).
BuildCoinString:
push ax
push bx
push di
xor  ax, ax
mov  al, [coin_count]
mov  di, small_number_buffer
mov  bx, 2
call NumberToFixedString
pop  di
pop  bx
pop  ax
ret

; NumberToFixedString formats an unsigned 16-bit value into a fixed-width decimal string.
; It places a null terminator at the end and fills digits backward from the right.
; Repeated division by 10 turns each remainder into an ASCII character at DI.
; Inputs: AX = number, DI = destination, BX = number of digits.
; Outputs: the zero padded decimal digits and a zero byte are written at DI.
; Registers changed: none (all registers are saved and restored).
NumberToFixedString:
push ax
push bx
push cx
push dx
push di
mov  cx, bx
add  di, bx
mov  byte [di], 0
dec  di
NumberToFixedStringAgain:
xor  dx, dx
mov  bx, 10
div  bx
add  dl, '0'
mov  [di], dl
dec  di
dec  cx
jnz  NumberToFixedStringAgain
pop  di
pop  dx
pop  cx
pop  bx
pop  ax
ret

lives db 3
coin_count db 0
score_low dw 0
score_high dw 0
score_work_hi dw 0
score_work_low dw 0
number_buffer times 7 db 0
small_number_buffer times 4 db 0
