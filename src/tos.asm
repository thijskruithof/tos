; Main assembly file

; Stage 0 first (resides in the MBR)
bootstage0_start:
; times 90 db 0 ; BPB (BIOS Parameter Block) will go here
%include "boot/bootstage0.asm"
bootstage0_end:

; Followed by stage 1 (which is loaded from disk by stage 0)
bootstage1_start:
%include "boot/bootstage1.asm"
align 512, db 0
bootstage1_end:

kernel_start:
;     %include "kernel/kernel.asm"
align 512, db 0
kernel_end:

