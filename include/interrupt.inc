INTERRUPT_COUNT			equ		256
INTERRUPT_8259A_OFFSET		equ		32

; IRQ save process stack frame procedure
%macro IRQ_SAVE_PROC_FRAME 0

        ; Save user process stack frame
        pushad
        push ds
        push es
        push fs
        push gs

        ; Set ds, es, fs to the descriptor of ring 0
        mov ax, ss
        mov ds, ax
        mov es, ax
        mov fs, ax

%endmacro
