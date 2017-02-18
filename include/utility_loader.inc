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
	
	mov cx, 4			; Set loop register -- 4 bytes
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
	loop WriteNumber_Loop		; Continue Loop

	pop bx				; Get Row:Column

	mov [WriteNumber_StrBuf], edx	; Write result string to string buffer
	push word WriteNumber_StrBuf	; Push address of the string buffer 
	push word 4			; 4 bytes string
	push word bx			; Set Row:Column
	call WriteString

	ret


; # Write Number(32-bit)
; Order of pushing stack: 32-bit number, Row:Column
WriteNumberEx:
	pop ax				; Save return address
	pop cx				; Row:Column
	pop ebx				; Get 32 bit number	
	push ax				; Restore return address
	
	mov ax, bx			; Save Low Byte
	shr ebx, 16			; Shift High Byte to right position
	
	; bx <--> cx			; High Byte display position
	; ax <--> dx			; Low Byte display position
	mov dx, cx			; Calculate Low Byte display position
	add dl, 4
	
	push bx				; Push High Byte display parameters
	push cx
	push ax				; Push Low Byte display parameters
	push dx
	
	call WriteNumber
	call WriteNumber

	ret


; # Get Address Range Descriptor Structures (ARDS)
GetARDS:
	; Display Title
	push word Str_DisplayARDSTitle
	push word [StrLen_DisplayARDSTitle]
	push 0300h
	call WriteString
	
	; Initialize Registers
	mov ebx, 0
	mov ax, BufferSegment		; Set es:di
	mov es, ax
	mov ax, BufferOffset
	mov di, ax
	mov ecx, 20			; ARDS is 20 bytes
	mov edx, 0534d4150h		; "SMAP"

GetARDS_Next:
	mov ax, 0e820h			; Set ax as function code of getting ARDS
	int 15h				; Get ARDS

	jc GetARDS_Failed		; If CF = 1, then failed to get ARDS

	; Display ARDS
	push ebx			; Save registers
	push ecx
	push edx
	push di
	push es

	call DisplayARDS		; Go to display current ARDS

	pop es				; Restore registers
	pop di
	pop edx
	pop ecx
	pop ebx

	cmp ebx, 0			; Check : more ARDS?
	jne GetARDS_Next		

	ret
	
GetARDS_Failed:
	; Display the failed message
	push word Str_GetARDSFailed
	push word [StrLen_GetARDSFailed]
	push 0200h			; Row 02, Col 00
	call WriteString

	jmp $				; Stop here	


; # Display Address Range Descriptor Structures (ARDS)
DisplayARDS:
	mov cx, [DisplayARDS_Pos]	; Get display position
	
DisplayARDS_Loop:
	mov eax, [es:di]		; Get a 32-bit value from ARDS

	push es				; Save es, di
	push di

	push eax			; Display 32-bit value of ARDS
	push cx
	call WriteNumberEx

	pop di				; Restore di, es
	pop es

	mov cx, [DisplayARDS_Pos]	; Get display position

	add di, 4			; Increase the buffer pointer
	add cl, 9			; Increase the cursor pointer
	mov [DisplayARDS_Pos], cx	; Save display position
	
	cmp di, 20			; Check display finished
	jne DisplayARDS_Loop
	
	inc ch				; Row++
	xor cl, cl			; Reset Column
	mov [DisplayARDS_Pos], cx	; Save display position for next display

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


; #############################
;             Data
; #############################

WriteNumber_StrBuf:		db		"XXXX"			; 4 Bytes Buffers

Str_GetARDSFailed:		db		"Get ARDS Failed"
StrLen_GetARDSFailed:		dw		$ - Str_GetARDSFailed
Str_DisplayARDSTitle:		db		"   Base Address  |      Length     |  Type"
StrLen_DisplayARDSTitle:	dw		$ - Str_DisplayARDSTitle
DisplayARDS_Pos:		dw		0400h
