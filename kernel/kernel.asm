LABEL_KERNEL:
	; Display "K" for kernel	
	mov ah, 0fh
	mov al, 'K'
	mov [gs:((80 * 2 + 4) * 2)], ax

	jmp $
