;##############################
;       Memory Definition
;##############################
; Stack - Address Range			(1FFFCh~10000h = 64 KB - 8B)
; Kernel in Memory - Address Range	(50000h~5FFFFh = 64 KB)

%include "pm.inc"

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

global _start		; Export entry function for linker
global process_restart

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

	sti			; Enable interrupt
	int 80h			; 0x80 interrupt test
	
	call kernel_main 	; Jmp to kernel_main

	; Prepare for starting user process
	; # Load LDT
	mov ax, KERNEL_GDT_FLAT_LDT_0_Selector
	lldt ax
	; # Load TSS
	mov ax, KERNEL_GDT_FLAT_TSS_Selector
	ltr ax
	; # Switch esp to PROCESS TABLE
	mov [kernel_esp], esp	; Save kernel esp
	mov esp, user_process	; Set esp to the top of user process stack frame

	; # Entry of restarting user process
process_restart:
	
	; Restore all registers of user process
	pop gs			
	pop fs
	pop es
	pop ds
	popad

	; Start user process
	iretd	
