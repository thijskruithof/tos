[bits 16]
[org 0x7c00]

bootstage0_entry:

; Some BIOS may load us at 0x0000:0x7C00 while others at 0x07C0:0x0000. 
; We do a far jump to accommodate for this issue (CS is reloaded to 0x0000).
jmp 0x0000:.setup_segments

.setup_segments:

; Next, we set all segment registers to zero.
xor ax, ax
mov ss, ax
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
; We set up a temporary stack so that it starts growing below bootstage0_entry 
; (i.e. the stack base will be located at 0:0x7c00).        
mov sp, bootstage0_entry   
; Clear direction flag (go forward in memory when using instructions like lodsb).
cld                                 

; Loading stage 1 from disk into RAM (places it at 'bootstage1_start')
call disk_read_bootstage1

; Print "MBR stage finished." message.
mov si, mbr_message
call Real_mode_println

.halt: hlt   ; Infinite loop.
jmp .halt    ; (It prevents us from going off in memory and executing junk).


; Includes
%include "boot/disk.asm"
%include "boot/print.asm"


; ; BIOS sets boot drive in 'dl'; store for later use
; mov [BOOT_DRIVE], dl

; ; setup stack
; mov bp, 0x9000
; mov sp, bp

; call load_kernel
; call switch_to_32bit

; ; infinite loop
; jmp $       

; %include "disk.asm"
; %include "gdt.asm"
; %include "switch-to-32bit.asm"

; [bits 16]
; load_kernel:
;     mov bx, KERNEL_OFFSET ; bx -> destination
;     mov dh, 2             ; dh -> num sectors
;     mov dl, [BOOT_DRIVE]  ; dl -> disk
;     call disk_load
;     ret

; [bits 32]
; BEGIN_32BIT:
;     call KERNEL_OFFSET ; give control to the kernel
;     jmp $ ; loop in case kernel returns

; ; boot drive variable
; BOOT_DRIVE db 0

; padding
times 510 - ($-$$) db 0

; Magic number: the last two bytes of the boot sector should have the 0xAA55 signature.
; Otherwise, we'll get an error message from BIOS that it didn't find a bootable disk.
; This signature is represented in binary as 1010101001010101. The alternating bit 
; pattern was thought to be a protection against certain (drive or controller) failures.
dw 0xaa55
