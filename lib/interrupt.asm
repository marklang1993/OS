global sti
global cli
global int_handler_clock
global int_handler_default 

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
	; Display changed character for clock interrupt	
	mov byte [gs:((80 * 2 + 8) * 2 + 1)], 0fh
	inc byte [gs:((80 * 2 + 8) * 2)]
	
	push eax
	mov al, 20h
	out 20h, al	; Send EOI
	pop eax	

	iretd
