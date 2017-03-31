INTERRUPT_COUNT			equ		256
INTERRUPT_8259A_OFFSET		equ		32

; Interrupt : save process stack frame procedure
%macro SAVE_PROC_FRAME 0

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
        SAVE_PROC_FRAME

        ; Notify 8295A - ready for next interrupt
        mov al, PORT_8259A_MAIN_0
        out DATA_8259A_OCW2, al         ; Send EOI, activate 8259A

        ; Check interrupt re-enter - whether need to switch stack or not
        inc dword [int_global_reenter]
        cmp dword [int_global_reenter], 1
        jne i8259a_goto_dispatch_ %+ %1

        ; Switch stack to kernel stack
        mov [current_user_process], esp ; Save pointer to current user process frame
        mov esp, [kernel_esp]

i8259a_goto_dispatch_ %+ %1:

	; Turn on interrupt
	sti

        ; Run interrupt dispatch function
        push dword %1			; Pass 8259A interrupt index
        call i8259a_interrupt_dispatch  ; Run 8259A interrupt dispatch function
        add esp, 4                      ; Clean stack

	; Turn off interrupt
	cli

        ; Load new user process frame
	mov eax, [current_user_process]

        ; Decrease global re-enter times 
        dec dword [int_global_reenter]

        jmp process_restart

%endmacro


; System call interrupt handler procedure
%macro SYSCALL_INT_HANDLER 0

        ; Save user process stack frame
        SAVE_PROC_FRAME

        ; Check interrupt re-enter - whether need to switch stack or not
        inc dword [int_global_reenter]
        cmp dword [int_global_reenter], 1
        jne syscall_goto_dispatch_ %+ %1

        ; Switch stack to kernel stack
        mov [current_user_process], esp ; Save pointer to current user process frame
        mov esp, [kernel_esp]

syscall_goto_dispatch_ %+ %1:

	; Turn on interrupt
	sti

        ; Run interrupt dispatch function
        push ebx			; Pass base address of arguments
        call sys_call_dispatch		; Run syscall interrupt dispatch function 
        add esp, 4                      ; Clean stack
	
	; Turn off interrupt
	cli

        ; Load new user process frame
	mov eax, [current_user_process]

        ; Decrease global re-enter times
        dec dword [int_global_reenter]

        jmp process_restart

%endmacro

