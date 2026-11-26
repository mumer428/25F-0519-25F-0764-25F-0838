LoadHighScore:
push ax
push bx
push cx
push dx
mov word [high_score_low], 0
mov word [high_score_high], 0
mov dx, high_score_filename
mov ax, 3D00h
int 21h
jc LoadHighScoreDone
mov [high_score_handle], ax
mov bx, ax
mov cx, 4
mov dx, high_score_low
mov ah, 3Fh
int 21h
cmp ax, 4
je LoadHighScoreClose
mov word [high_score_low], 0
mov word [high_score_high], 0
LoadHighScoreClose:
mov bx, [high_score_handle]
mov ah, 3Eh
int 21h
mov ax, [high_score_high]
cmp ax, 15
jb LoadHighScoreDone
ja LoadHighScoreMax
mov ax, [high_score_low]
cmp ax, 423Fh
jbe LoadHighScoreDone
LoadHighScoreMax:
mov word [high_score_high], 15
mov word [high_score_low], 423Fh
LoadHighScoreDone:
pop dx
pop cx
pop bx
pop ax
ret

UpdateHighScore:
push ax
mov ax, [score_high]
cmp ax, [high_score_high]
ja UpdateHighScoreStore
jb UpdateHighScoreDone
mov ax, [score_low]
cmp ax, [high_score_low]
jbe UpdateHighScoreDone
UpdateHighScoreStore:
mov ax, [score_low]
mov [high_score_low], ax
mov ax, [score_high]
mov [high_score_high], ax
UpdateHighScoreDone:
pop ax
ret

SaveHighScore:
push ax
push bx
push cx
push dx
mov dx, high_score_filename
mov cx, 0
mov ah, 3Ch
int 21h
jc SaveHighScoreDone
mov [high_score_handle], ax
mov bx, ax
mov cx, 4
mov dx, high_score_low
mov ah, 40h
int 21h
mov bx, [high_score_handle]
mov ah, 3Eh
int 21h
SaveHighScoreDone:
pop dx
pop cx
pop bx
pop ax
ret

BuildHighScoreString:
push word [high_score_high]
push word [high_score_low]
call BuildScoreFrom
add sp, 4
ret

DrawHighScore:
push ax
push bx
call BuildHighScoreString
mov ax, line_buffer
mov bx, high_score_label
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
mov ax, 14
push ax
mov ax, 1
push ax
mov ax, 134
push ax
call DrawTextCentered
add sp, 8
pop bx
pop ax
ret

high_score_low dw 0
high_score_high dw 0
high_score_handle dw 0
high_score_filename db 'HIGHSCOR.DAT', 0
high_score_label db 'HIGH SCORE: ', 0
