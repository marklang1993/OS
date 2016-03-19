	org 7c00h	; Load to memory address 0x7c00

	; Main
	mov ax, cs;	; Get code segment address
	mov ds, ax;	; Set data segment address
	mov es, ax;	; Set extra segment address
	
	
	; Display Message
	call ClearScreen
	push word Str		; Transfer the address of that string
	push word [StrLen]	; Transfer the length of that string
	call WriteString	
	add sp, 4		; Reset stack

	jmp $			; Stop at here

; WriteString
WriteString:
	pop ax			; Save return address
	pop cx			; Get the length of String from stack
	pop bp			; Get the address of String from stack
	push ax			; Restore return address

	mov ah, 13h		; Parameter: show string
	mov al, 01h		; Parameter: continue to show
	mov bx, 000ch		; Page: 0, Back: Black, Font: Red
	mov dh, 0h		; 0 Row
	mov dl, 0h		; 0 Column
	int 10h			
	ret			

; ClearScreen
ClearScreen:
	mov ah, 06h;		
	mov al, 00h;
	mov cx, 00h;
	mov dh, 17h;		; To 23 Row
	mov dl, 4fh		; To 79 Column	
	mov bh, 00h;		
	int 10h;
	ret;	

; Data
Str:			db 	"Hello, OS world!"
StrLen:			db	$ - Str

times 510 - ($ - $$)	db 	0 
			dw 	0xaa55




