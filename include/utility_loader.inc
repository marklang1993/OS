; #############################
;          Utilities
; #############################

; # Write Number(16-bit)
; Order of pushing stack: 16-bit number, Row:Column
WriteNumber:
	pop ax				; Save return address
	pop dx				; Save Row:Column
	pop bx				; Get Number
	push ax				; Restore return address
	push dx				; Restore Row:Column
	
	xor cx, cx			; Clear loop register
	xor edx, edx			; Clear result register

WriteNumber_Loop:	
	xor ax, ax			; Clear ax
	mov al, bl			
	shr bx, 4			; Let next digit be on the rightmost position
	
	and al, 0fh			; Get the front digit
	add al, 48			; Add the offset
	
	cmp al,	57			; Check decimal digit or hexdecimal digit
	jbe WriteNumber_EndLoop		; Not a hexdecimal digit
	add al, 7			; Add the offset to become a real hexdecimal digit for char display

WriteNumber_EndLoop:	
	shl edx, 8			; Shift the former result to higher position
	add dl, al			; Save the result
	inc cx				; Loop register increase	
	cmp cx, 4			; Totally 4 digits for 16-bit number
	jne WriteNumber_Loop		; Continue Loop

	pop bx				; Get Row:Column

	mov [StrBuffer], edx		; Write result string to string buffer
	push word StrBuffer		; Push address of the string buffer 
	push word 4			; 4 bytes string
	push word bx			; Set Row:Column
	call WriteString

	ret


; #############################
;     Floppy Disk Utilities
; #############################

; # Turn Off Floppy Motor
FloppyMotorOff:
	push ax				; Save ax, dx
	push dx

	xor al, al			; Clear al
	mov dx, 3f2h			; Set port IO
	out dx, al			; Turn off floppy motor

	pop dx				; Restore dx, ax
	pop ax
	
	ret
