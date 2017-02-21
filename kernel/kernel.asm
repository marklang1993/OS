%include "boot/pm.inc"
; Kernel GDT Selector
KERNEL_GDT_FLAT_C_Selector		equ		1 << 3
KERNEL_GDT_FLAT_DRW_Selector		equ		2 << 3
KERNEL_GDT_STACK_Selector		equ		3 << 3
KERNEL_GDT_VIDEO_Selector		equ		(4 << 3) + SEL_RPL_3	


[section .text]

; External functions
extern print_string
extern kernel_init

; External variables
extern gdt_ptr

global _start		; Export entry function for linker

_start:
	; Display "K" for kernel	
	mov ah, 0fh
	mov al, 'K'
	mov [gs:((80 * 2 + 4) * 2)], ax

	; Switch gdt_ptr from loader to kernel 
	sgdt [gdt_ptr]
	call kernel_init
	lgdt [gdt_ptr]

	jmp KERNEL_GDT_FLAT_C_Selector:Kernel_Start 

Kernel_Start:
	; Initialize registers -- ds, es, ss, gs
	mov ax, KERNEL_GDT_FLAT_DRW_Selector
	mov ds, ax
	mov es, ax
	mov ax, KERNEL_GDT_STACK_Selector
	mov ss, ax
	mov ax, KERNEL_GDT_VIDEO_Selector
	mov gs, ax

	; Clear EFLAGS
	push dword 0
	popfd

	; Display string
	push dword 0
	push dword 13
	push dword [StrLen_KernelRunning]
	push Str_KernelRunning
	call print_string
	add esp, 16

	jmp $


[section .data]
Str_KernelRunning 		db	"Kernel is running!"
StrLen_KernelRunning		dd	$ - Str_KernelRunning		; == 18


