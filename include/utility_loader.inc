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

	; Calculate Memory Size
	push ebx			; Save registers
	push di
	push es
	call CalMemSizeByARDS
	pop es				; Restore registers
	pop di
	pop ebx

	cmp ebx, 0			; Check : more ARDS?
	jne GetARDS_Next		

	; Print Memory Size
	mov ax, [DisplayARDS_Pos]	; Adjust display position - Row
	inc ah
	push ax				; Save ax

	; Display Memory Size String
	push word Str_DisplayMemSize
	push word [StrLen_DisplayMemSize]
	push ax
	call WriteString

	pop ax				; Restore ax
	mov bx, [StrLen_DisplayMemSize]	; Adjust display position - Col
	add al, bl
	
	mov ebx, [PhysicalMemorySize]	; Get Physical Memory Size Value
	; Display Memory Size
	push ebx
	push ax
	call WriteNumberEx

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

	mov ax, BufferSegment		; Set es:di
	mov es, ax
	mov ax, BufferOffset
	mov di, ax
	
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


; # Calculate MemorySize By ARDS
CalMemSizeByARDS:
	; Note : This is a 32-bit OS, so high word (32-63 bit) will be ignored
	mov al, [PhysicalMemoryAccuStop]
	cmp al, 0			; Check stop flag
	je CalMemSizeByARDS_Run		; Stop flag is not set -> go to calculate
	
	ret

CalMemSizeByARDS_Run:
	mov ax, BufferSegment		; Set es:di
	mov es, ax
	mov ax, BufferOffset
	mov di, ax	

	mov eax, [es:di]		; Get base address (low word) from ARDS
	add di, 8			; Move pointer to Length (low word)
	mov ebx, [es:di]		; Get length (low word) from ARDS
	add eax, ebx			; Calculate new memory size
	add di, 8			; Move pointer to Type
	mov ebx, [es:di]		; Get type

	cmp ebx, 3			; Check is this the last valid ARDS
	jnz CalMemSizeByARDS_Update
	
	; Set stop flag
	mov byte [PhysicalMemoryAccuStop], 1

CalMemSizeByARDS_Update:
	mov [PhysicalMemorySize], eax		; Update current memory size

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
Str_DisplayMemSize:		db		"Physical Memory Size: "
StrLen_DisplayMemSize:		dw		$ - Str_DisplayMemSize
DisplayARDS_Pos:		dw		0400h
PhysicalMemorySize:		dd		0h			; Save memory size after GetARDS
PhysicalMemoryAccuStop:		db		0h			; 0 -- Continue to calculate; 1 -- Stop
