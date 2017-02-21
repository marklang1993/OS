global print_string

[section .text]

; # void print_string(char *ptr_string, uint32 size, uint32 row, uint32 col);
; @ ptr_string: string address in ds
; @ size: size of string
; @ row: row of displaying
; @ col: column of displaying

; NOTE: assume gs stored the descriptor of display memory
print_string:
	push ebp		; Save frame pointer
	mov ebp, esp		; Set new frame pointer

	push eax		; Save changed registers
	push ebx
	push ecx
	push edi

	mov eax, [ss:(ebp+16)]	; Get row
	mov ebx, [ss:(ebp+20)]	; Get column
	
	; Calculate address offset -- edi = (80 * row + col) * 2
	mov ecx, 80
	mul ecx			; Result is in edx:eax, discard edx
	
	add eax, ebx
	shl eax, 1
	mov edi, eax
	
	; Get other parameters
	mov ecx, [ss:(ebp+12)]	; Get size
	mov ebx, [ss:(ebp+8)]	; Get string address
	
	; Display string from last char to first char
	add ebx, ecx
	dec ebx

	shl ecx, 1		; NOTE: each char take 2 bytes
	add edi, ecx	
	sub edi, 2
	shr ecx, 1

print_string_Loop:
	mov ah, 0fh		; Char: white, Back: black
	mov al, [ds:ebx]	; Get 1 char
	mov [gs:edi], ax	; Write display memory
	dec ebx
	sub edi, 2		; edi -= 2 
	loop print_string_Loop	; Continue to write display memory

	pop edi			; Restore changed registers
	pop ecx
	pop ebx
	pop eax
	
	pop ebp
	ret
	

