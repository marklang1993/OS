;##############################
;       Memory Definition
;##############################
; Stack - Address Range			(1FFFCh~10000h = 64 KB - 8B)
; Kernel in Memory - Address Range	(50000h~5FFFFh = 64 KB)

%include "pm.inc"
%include "proc.inc"

[section .text]

; External functions
extern kernel_init
extern kernel_main


; External variables
extern gdt_ptr
extern idt_ptr
extern tss
extern user_process
extern kernel_esp

extern int_global_reenter;

global _start		; Export entry function for linker
global process_restart
global process_restart_reenter

_start:
	; Display "K" for kernel	
	mov ah, 0fh
	mov al, 'K'
	mov [gs:((80 * 2 + 4) * 2)], ax

	; Switch gdt_ptr from loader to kernel 
	sgdt [gdt_ptr]
	call kernel_init
	lgdt [gdt_ptr]
	lidt [idt_ptr]

	jmp KERNEL_GDT_FLAT_C_Selector:Kernel_Start 

Kernel_Start:
	; Initialize registers -- ds, es, fs, ss, gs
	mov ax, KERNEL_GDT_FLAT_DRW_Selector
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax
	mov ax, KERNEL_GDT_VIDEO_Selector
	mov gs, ax

	; Clear EFLAGS
	push dword 0
	popfd

	; Reset esp to kernel esp
	mov esp, KERNEL_ESP

	; Jmp to kernel_main
	call kernel_main

	; Prepare for starting user process
	; # Load TSS
	mov ax, KERNEL_GDT_FLAT_TSS_Selector
	ltr ax
	; # Switch esp to the top of user process stack frame of index 0
	mov eax, user_process

	; # Start the user process of index 0
process_restart:
	
	; Check all interrupt re-enter are done
	cmp dword [int_global_reenter], 0
	jne process_restart_reenter	; Not the last re-enter interrupt 

	; Switch esp back to user process stack frame
	mov [kernel_esp], esp	; Save kernel esp
	mov esp, eax		; Set esp to the new user process stack frame
	; Load LDT
	lldt [esp + PROCESS_LDT_PTR_OFFSET]
	; Reset esp_0 on tss - esp position of process table entry for next clock interrupt
        lea eax, [esp + PROCESS_BOTTOM_STACK_FRAME_OFFSET]
        mov dword [tss + TSS_ESP_0_OFFSET], eax

process_restart_reenter:

	; Restore all registers of user process / Restore all registers of last interrupt
	pop gs			
	pop fs
	pop es
	pop ds
	popad

	; Restart user process / Restart last interrupt
	iretd
