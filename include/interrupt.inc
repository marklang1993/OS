INTERRUPT_COUNT			equ		256
INTERRUPT_8259A_OFFSET		equ		32

; I8259A save process stack frame procedure
%macro I8259A_SAVE_PROC_FRAME 0

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

; I8269A interrupt handler procedure
%macro I8259A_INT_HANDLER 1

        ; Save user process stack frame
        I8259A_SAVE_PROC_FRAME

        ; Notify 8295A - ready for next interrupt
        mov al, PORT_8259A_MAIN_0
        out DATA_8259A_OCW2, al         ; Send EOI, activate 8259A

        ; Check interrupt re-enter - whether need to switch stack or not
        cmp dword [int_global_reenter], 0
        jne i8259a_keep_esp_ %+ %1
        inc dword [int_global_reenter]

        ; Switch stack to kernel stack
        mov [current_user_process], esp ; Save pointer to current user process frame
        mov esp, [kernel_esp]

i8259a_keep_esp_ %+ %1:
        ; Check recursively calling of a specific handler
        lea ebx, [int_reenter_times + (INTERRUPT_8259A_OFFSET + %1) * 4]
        cmp dword [ebx], 0
        jne i8259a_goto_dispatch_ %+ %1	; This interrupt can be dispatched

	; This interrupt cannot be dispatched: insufficient re-enter times
	push %1					; Pass 8259A interrupt index
	call i8259a_failed_interrupt_dispatch	; Run 8259A re-enter failed interrupt dispatch function
	add esp, 4				; Clean stack

	; Go to end processing
	jmp process_restart_reenter

i8259a_goto_dispatch_ %+ %1:
	dec dword [ebx]                 ; Decrease remained re-entry times

	; Turn on interrupt
	sti

        ; Run interrupt dispatch function
        push %1				; Pass 8259A interrupt index
        call i8259a_interrupt_dispatch  ; Run 8259A interrupt dispatch function
        add esp, 4                      ; Clean stack
	
	; Turn off interrupt
	cli

        ; Load new user process frame
	mov eax, [current_user_process]

        ; Increase remained re-entry times
        lea ebx, [int_reenter_times + (INTERRUPT_8259A_OFFSET + %1) * 4]
        inc dword [ebx]

        ; Decrease global re-enter times 
        dec dword [int_global_reenter]

        jmp process_restart

%endmacro

