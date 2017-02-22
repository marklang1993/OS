global memcpy

[section .text]

; # void memcpy(void *src, void *dst, uint32 size);
; @ src: source address in ds
; @ dst: destination address in ds
; @ size: size of memory copied
memcpy:
	push ebp		; Save frame pointer
	mov ebp, esp		; Set new frame pointer

	push eax		; Save changed registers
	push ecx
	push esi
	push edi

	mov ecx, [ss:(ebp+16)]	; Get size of memory needed to be copied
	mov edi, [ss:(ebp+12)]	; Get dst address (wrt. ds)
	mov esi, [ss:(ebp+8)]	; Get src address (wrt. ds)

memcpy_Loop:
	mov ax, [ds:esi]	; Get 1 byte from src memory
	mov [ds:edi], ax	; Write 1 byte to dst memory
	inc esi			; esi++
	inc edi			; edi++
	loop memcpy_Loop

	pop edi			; Restore changed registers
	pop esi
	pop ecx
	pop eax

	pop ebp			; Restore ebp
	ret

