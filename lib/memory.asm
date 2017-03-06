global memcpy
global memset

[section .text]

; # void memcpy(void *dst, const void *src, uint32 size);
; @ dst: destination address in ds
; @ src: source address in ds
; @ size: size of memory copied
memcpy:
	push ebp		; Save frame pointer
	mov ebp, esp		; Set new frame pointer

	push eax		; Save changed registers
	push ecx
	push esi
	push edi

	mov ecx, [ss:(ebp+16)]	; Get size of memory needed to be copied
	mov esi, [ss:(ebp+12)]	; Get src. address (wrt. ds)
	mov edi, [ss:(ebp+8)]	; Get dst. address (wrt. ds)

memcpy_Loop:
	mov al, [ds:esi]	; Get 1 byte from src. address
	mov [ds:edi], al	; Write 1 byte to dst. address
	inc esi			; esi++
	inc edi			; edi++
	loop memcpy_Loop

	pop edi			; Restore changed registers
	pop esi
	pop ecx
	pop eax

	pop ebp			; Restore ebp
	ret


; # void memset(void *ptr, uint32 val, uint32 size);
; @ ptr: address in ds
; @ val: value will be set (only lower 8 bits are valid)
; @ size: size of memory set
memset:
	push ebp		; Save frame pointer
	mov ebp, esp		; Set new frame pointer

	push eax		; Save changed registers
	push ecx
	push edi

	mov ecx, [ss:(ebp+16)]	; Get size of memory needed to be set
	mov eax, [ss:(ebp+12)]	; Get value
	mov edi, [ss:(ebp+8)]	; Get address (wrt. ds)

memset_Loop:
	mov [ds:edi], al	; Write 1 byte to dst. address
	inc edi			; edi++
	loop memset_Loop

	pop edi			; Restore changed registers
	pop ecx
	pop eax

	pop ebp			; Restore ebp
	ret


