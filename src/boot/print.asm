BITS 16

;---Initialized data------------------------------------------------------------

newline dw 2
db 13,10 ; \r\n

mbr_message dw 18
db 'TOS: MBR finished.'

printnr_buf dw 3
db 0, 0
db 'h'

;---Code------------------------------------------------------------------------

Real_mode_print:
;*********************************************************************************;
; Prints a string (in real mode)                                                  ;
;---------------------------------------------------------------------------------;
; si: pointer to string (first 16 bits = the number of characters in the string.) ;  
;*********************************************************************************;
push ax
push cx
push si
mov cx, word [si] ; first 16 bits = the number of characters in the string
add si, 2
.string_loop:     ; print all the characters in the string
lodsb
mov ah, 0eh
int 10h
loop .string_loop, cx
pop si
pop cx
pop ax
ret


Real_mode_println:
;***********************************************************;
; Prints a string (in real mode) and a newline (\r\n)       ;
;-----------------------------------------------------------;
; si: pointer to string                                     ;
; (first 16 bits = the number of characters in the string.) ;  
;***********************************************************;
push si
call Real_mode_print
mov si, newline
call Real_mode_print
pop si
ret

Real_mode_printnr:
;***********************************************************;
; Prints value from AL                                      ;
;***********************************************************;
push ax
and al, 0xF0
shr al, 4
add al, '0'
mov [printnr_buf+2], al
pop ax
and al, 0x0F
add al, '0'
mov [printnr_buf+3], al
mov si, printnr_buf
call Real_mode_print
mov si, newline
call Real_mode_print
ret 
