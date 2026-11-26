; CONSTANTS.INC - shared named constants (game states, flag pole zone, enemy record layout, sky color).
; Owner: Member 3 - Game Logic (Module 1).

STATE_TITLE          equ 0
STATE_PLAYING        equ 1
STATE_PAUSED         equ 2
STATE_DYING           equ 3
STATE_LIFE_LOST       equ 4
STATE_WIN_SEQUENCE    equ 5
STATE_LEVEL_COMPLETE  equ 6
STATE_GAME_OVER       equ 7
STATE_EXIT            equ 8
FLAG_POLE_LEFT_X      equ 934
FLAG_POLE_RIGHT_X     equ 936
FLAG_POLE_TOP_Y       equ 32
FLAG_POLE_BOTTOM_Y    equ 144

ENEMY_COUNT equ 6
ENEMY_SIZE  equ 10
E_TYPE      equ 0
E_STATE     equ 1
E_X         equ 2
E_Y         equ 4
E_DIR       equ 6
E_TIMER     equ 7
E_ACTIVE    equ 8
E_PHASE     equ 9
E_GOOMBA    equ 1
E_KOOPA     equ 2
E_WALK      equ 1
E_FLAT      equ 2
E_SHELL     equ 3
E_SLIDE     equ 4

COLOR_SKY equ 9
