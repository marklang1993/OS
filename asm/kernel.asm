%include "boot.inc"

LABEL_KERNEL:
	mov ax, cs					; Get code segment address
	mov ds, ax					; Set data segment address
	mov es, ax					; Set extra segment address
	mov bx, StackSegment				; Set stack segment address
	mov ss, bx
	mov sp, StackOffset				; Set stack pointer
	
	; Display Message
	push word Str_KernelRunning			; Transfer the address of that string
	push word [StrLen_KernelRunning]		; Transfer the length of that string
	push word 0200h					; Set 1 Row : 0 Column
	call WriteString

	jmp $


; #############################
;          Subrouting
; #############################

; # Write String
; Order of pushing stack: address of string, length, Row:Column
WriteString:
	pop ax				; Save return address
	pop dx				; Get Row:Column
	pop cx				; Get the length of String from stack	
	pop bp				; Get the address of String from stack
	push ax				; Restore return address

	mov ax, cs			; Set es = cs since the segment of string and code is same, and display uses [es:bp]
	mov es, ax
	mov ah, 13h			; Parameter: show string
	mov al, 01h			; Parameter: continue to show
	mov bx, 000ch			; Page: 0, Back: Black, Font: Red
	int 10h			
	ret	

Str_KernelRunning:		db		"Kernel is running..."
StrLen_KernelRunning:		dw		$ - Str_KernelRunning

