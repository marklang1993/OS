global io_out_byte
global io_in_byte

[section .text]

; # void io_out_byte(uint32 io_port, uint32 data);
; @ io_port: io port number
; @ data: data to be written (only lower 8 bits will be written)

io_out_byte:
	push ebp                ; Save frame pointer
        mov ebp, esp            ; Set new frame pointer

	push eax		; Save changed registers
	push edx	

	xor eax, eax
	xor edx, edx

	mov eax, [ss:(ebp+12)]	; Get data
	mov dx, [ss:(ebp+8)]	; Get port io number
	out dx, al		; Write io port

	pop edx			; Restore changed registers
	pop eax
	
	pop ebp			; Restore ebp
	ret


; # void io_in_byte(uint32 io_port, uint32 *ptr_data);
; @ io_port: io port number
; @ ptr_data: pointer to output data area (only lower 8 bits of data is valid)

io_in_byte:
	push ebp                ; Save frame pointer
        mov ebp, esp            ; Set new frame pointer

	push eax		; Save changed registers
	push ebx
	push edx	

	xor eax, eax
	xor edx, edx

	mov edx, [ss:(ebp+8)]	; Get port io number
	mov ebx, [ss:(ebp+12)]	; Get pointer to output data area

	in al, dx		; Read io port
	mov [ebx], eax		; Put data
	
	pop edx			; Restore changed registers
	pop ebx
	pop eax
	
	pop ebp			; Restore ebp
	ret

