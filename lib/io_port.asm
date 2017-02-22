global io_port_out


[section .text]

; # void io_port_out(uint32 io_port, uint32 data);
; @ io_port: io port number
; @ data: data to be written (only lower 8 bits will be written)

io_port_out:
	push ebp                ; Save frame pointer
        mov ebp, esp            ; Set new frame pointer

	push eax		; Save changed registers
	push edx	

	xor eax, eax
	xor edx, edx

	mov ax, [ss:(ebp+12)]	; Get data
	mov dx, [ss:(ebp+8)]	; Get port io number

	out dx, al		; Write io port
	nop			; Delay
	nop
	nop
	nop


	pop edx			; Restore changed registers
	pop eax
	
	pop ebp			; Restore ebp
	ret
