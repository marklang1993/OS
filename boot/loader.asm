%include "boot.inc"

WriteString:
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ax, StackSegment
	mov ss, ax

 
        mov dx, 0200h                   ; Get Row:Column
        mov cx, [StrMessageLen]  	; Get the length of String from stack   
        mov bp, StrMessage		; Get the address of String from stack

        
        mov ah, 13h                     ; Parameter: show string
        mov al, 01h                     ; Parameter: continue to show
        mov bx, 000ch                   ; Page: 0, Back: Black, Font: Red
        int 10h
        
	jmp $


StrMessage		db		"Loader is running..."
StrMessageLen		dw		$ - StrMessage
