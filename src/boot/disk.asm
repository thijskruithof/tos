[bits 16]

;---Initialized data------------------------------------------------------------

disk_error_message dw 11
db 'Disk error!'


;---Code------------------------------------------------------------------------

; Read the bootstage1 data from disk and stores it in memory, at bootstage1_start
; parameters:
; dl : current disk ID (already set by BIOS at startup)
disk_read_bootstage1:

; https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive
mov ah, 0x02  ; "Read sectors from drive"
mov al, 1 + ((kernel_end-bootstage1_start)/512) ; number of sectors to read
mov ch, 0                   ; Starting cylinder
mov cl, 1                   ; Starting sector (1-based)
mov dh, 0                   ; Starting head
mov bx, bootstage1_start    ; Destination buffer
int 0x13
jc .print_error
ret 

.print_error:
; Print error code
mov al, ah
call Real_mode_printnr
; print "Disk Error!"
mov si, disk_error_message
call Real_mode_println

.halt: hlt
jmp .halt; Infinite loop. We cannot recover from disk error.
