%include "syscall.inc"

global sys_call

[section .text]

; # void sys_call(uint32 num, int32 *ptr_ret, ...);
; @ num    : system call number
; @ ptr_ret: pointer to return value
; @ ...    : other arguments of corresponding system call
sys_call:

	lea ebx, [esp + 4]	; Get base address of all arguments

	; Go to ring 0
	; ebx : base address of all arguments
	int INT_SYS_CALL
	ret
