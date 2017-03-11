%include "interrupt.inc"
%include "pm.inc"
%include "proc.inc"
%include "drivers/i8259a.inc"

global sti
global cli

global int_entry_division_fault
global int_entry_debug_exception
global int_entry_non_maskable_interrupt
global int_entry_break_point
global int_entry_overflow
global int_entry_out_bound
global int_entry_undefined_opcode
global int_entry_no_math_coprocessor
global int_entry_double_fault
global int_entry_reserved_coprocessor_seg_overbound
global int_entry_invalid_tss
global int_entry_segment_not_exist
global int_entry_stack_segment_fault
global int_entry_general_protect_fault
global int_entry_page_fault
global int_entry_reserved_intel
global int_entry_x87fpu_fault
global int_entry_alignment_check_fault
global int_entry_machine_check_abort
global int_entry_simd_float_fault

global int_handler_default 
global int_handler_clock
global int_handler_keyboard

; kernel.c
extern tss
extern kernel_esp

; kernel.asm
extern process_restart
extern process_restart_reenter

; interrupt.c
extern interrupt_handler
extern int_global_reenter
extern int_reenter_times

; proc.c
extern current_user_process

; drivers/i8259a.c
extern i8259a_interrupt_dispatch


[section .text]

; # void sti(void)
sti:
	sti			; Enable interrupt
	ret

; # void cli(void)
cli:
	cli			; Disable interrupt
	ret


; NOTE: These are INTERRUPT HANDLERS, not for common call
; Order of pushing stack after interrupt: eflags, cs, eip, (error code) -- THEN push interrupt vector number
; NOTE: If no error code is pushed, then manually push 0xffffffff as default value

; Dispatch interrupt & Post-processing
interrupt_dispatch:
	call interrupt_handler
	jmp $			; Stop here
	
	pop eax			; Clean stack
	pop eax

	iretd

; 0 - #DE Fault
int_entry_division_fault:
	push dword 0ffffffffh
	push dword 0
	jmp interrupt_dispatch

; 1 - #DB Fault/Trap
int_entry_debug_exception:
	push dword 0ffffffffh
	push dword 1
	jmp interrupt_dispatch

; 2 - #-- Interrupt
int_entry_non_maskable_interrupt:
	push dword 0ffffffffh
	push dword 2
	jmp interrupt_dispatch

; 3 - #BP Trap
int_entry_break_point:
	push dword 0ffffffffh
	push dword 3
	jmp interrupt_dispatch

; 4 - #OF Trap
int_entry_overflow:
	push dword 0ffffffffh
	push dword 4
	jmp interrupt_dispatch

; 5 - #BR Fault
int_entry_out_bound:
	push dword 0ffffffffh
	push dword 5
	jmp interrupt_dispatch

; 6 - #UD Fault
int_entry_undefined_opcode:
	push dword 0ffffffffh
	push dword 6
	jmp interrupt_dispatch

; 7 - #NM Fault
int_entry_no_math_coprocessor:
	push dword 0ffffffffh
	push dword 7
	jmp interrupt_dispatch

; 8 - #DF Abort - Error code = 0
int_entry_double_fault:
	push dword 8
	jmp interrupt_dispatch

; 9 - #-- Fault
int_entry_reserved_coprocessor_seg_overbound:
	push dword 0ffffffffh
	push dword 9
	jmp interrupt_dispatch

; 10 - #TS Fault - Error code
int_entry_invalid_tss:
	push dword 10
	jmp interrupt_dispatch

; 11 - #NP Fault - Error code 
int_entry_segment_not_exist:
	push dword 11
	jmp interrupt_dispatch

; 12 - #SS Fault - Error code
int_entry_stack_segment_fault:
	push dword 12
	jmp interrupt_dispatch

; 13 - #GP Fault - Error code
int_entry_general_protect_fault:
	push dword 13
	jmp interrupt_dispatch

; 14 - #PF Fault - Error code
int_entry_page_fault:
	push dword 14
	jmp interrupt_dispatch

; 15 - #-- Intel Reserved - Not Used
int_entry_reserved_intel:
	push dword 0ffffffffh
	push dword 15
	jmp interrupt_dispatch

; 16 - #MF Fault
int_entry_x87fpu_fault:
	push dword 0ffffffffh
	push dword 16
	jmp interrupt_dispatch

; 17 - #AC Fault - Error code = 0
int_entry_alignment_check_fault:
	push dword 17
	jmp interrupt_dispatch

; 18 - #MC Abort
int_entry_machine_check_abort:
	push dword 0ffffffffh
	push dword 18
	jmp interrupt_dispatch

; 19 - #XF Fault
int_entry_simd_float_fault:
	push dword 0ffffffffh
	push dword 19
	jmp interrupt_dispatch


; # void int_handler_default(void)
; Handle non-defined interrupt
int_handler_default:
	; Display "D" for interrupt	
	mov ah, 0fh
	mov al, 'D'
	mov [gs:((80 * 2 + 6) * 2)], ax
	
	iretd

; # void int_handler_clock(void)
; Handle clock interrupt
int_handler_clock:
	I8259A_INT_HANDLER INDEX_8259A_CLOCK

; # void int_handler_keyboard(void)
; Handle keyboard interrupt
int_handler_keyboard:
	I8259A_INT_HANDLER INDEX_8259A_KEYBOARD

