global io_out_byte
global io_in_byte
global io_out_data
global io_in_data


[section .text]

; # void io_out_byte(uint16 io_port, uint8 data);
; @ io_port: io port number
; @ data: data to be written (only lower 8 bits will be written)

io_out_byte:
	push ebp                ; Save frame pointer
	mov ebp, esp            ; Set new frame pointer

	push eax		; Save changed registers
	push edx

	xor eax, eax
	xor edx, edx

	mov dx, [ss:(ebp+8)]	; Get port io number
	mov eax, [ss:(ebp+12)]	; Get data

	out dx, al		; Write io port

	pop edx			; Restore changed registers
	pop eax
	
	pop ebp			; Restore ebp
	ret


; # void io_in_byte(uint16 io_port, uint8 *ptr_data);
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

	mov dx, [ss:(ebp+8)]	; Get port io number
	mov ebx, [ss:(ebp+12)]	; Get pointer to output data area

	in al, dx		; Read io port
	mov [ebx], eax		; Put data
	
	pop edx			; Restore changed registers
	pop ebx
	pop eax
	
	pop ebp			; Restore ebp
	ret


; # void io_out_data(uint16 io_port, void *ptr_data, uint32 size);
; @ io_port: io port number
; @ ptr_data: pointer to output data area
; @ size: size in bytes

io_out_data:
	push ebp                ; Save frame pointer
	mov ebp, esp            ; Set new frame pointer

	push ecx		; Save changed registers
	push edx
	push esi
	pushfd

	xor edx, edx		; Get port io number
	mov dx, [ss:(ebp+8)]
	mov esi, [ss:(ebp+12)]	; Get pointer to input data area
	mov ecx, [ss:(ebp+16)]	; Get size

	cld			; Write io port
	rep outsb

	popfd			; Restore changed registers
	pop esi
	pop edx
	pop ecx

	pop ebp			; Restore ebp
	ret

; # void io_in_data(uint16 io_port, void *ptr_data, uint32 size);
; @ io_port: io port number
; @ ptr_data: pointer to input data area
; @ size: size in bytes


io_in_data:
	push ebp                ; Save frame pointer
	mov ebp, esp            ; Set new frame pointer

	push eax		; Save changed registers
	push ecx
	push edx
	push edi
	push es
	pushfd

	xor edx, edx		; Get port io number
	mov dx, [ss:(ebp+8)]
	mov edi, [ss:(ebp+12)]	; Get pointer to input data area
	mov ecx, [ss:(ebp+16)]	; Get size

	xor eax, eax		; Set es
	mov ax, ds
	mov es, ax

	cld			; Read io port
	rep insb

	popfd			; Restore changed registers
	pop es
	pop edi
	pop edx
	pop ecx
	pop eax

	pop ebp			; Restore ebp
	ret
