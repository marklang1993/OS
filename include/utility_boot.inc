; #############################
;          Utilities
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

; # Clear Screen
ClearScreen:
	mov ah, 06h;		
	mov al, 00h;
	mov cx, 00h;
	mov dh, 17h;			; To 23 Row
	mov dl, 4fh			; To 79 Column	
	mov bh, 00h;		
	int 10h;
	ret;

; # String Compare
; Order	of pushing stack: address of src string (ds:si), address of dst string (es:di), length in bytes
; Return val is in cx:       0     - equal, 
;                      more than 0 - not equal
StringCompare:
	pop ax	
	pop cx				; get length in bytes
	pop di				; get address of dst string
	pop si				; get address of src string
	push ax 

	mov ax, cs			; set ds = cs = 0h
	mov ds, ax
	mov ax, BufferSegment		; set es = BufferSegment
	mov es, ax
	
StringCompare_Loop:
	mov al, [ds:si]			; Get a char from src string
	mov ah, [es:di]			; Get a char from dst string
	cmp al, ah			; Compare
	jnz StringCompare_EndLoop	; If not equal, then goto NotEqual processing

	inc si				; si++
	inc di				; di++
	loop StringCompare_Loop

StringCompare_EndLoop:
	ret

; # Copy Data
; Order	of pushing stack: address of buffer position (ds:si), address of data dst position (es:di), length in bytes
CopyData:
	pop ax				; Save return address
	pop cx				; Get length
	pop di				; Get data position
	pop si				; Get buffer position
	push ax				; Restore return address

	push ds				; Save ds

	mov ax, BufferSegment		; Set ds for src
	mov ds, ax
	mov ax, CopyDataDstSegment	; Set es for dst
	mov es, ax

CopyData_Loop:
	mov al, [ds:si]			; Read 1 byte from buffer
	mov [es:di], al			; Write 1 byte to data code area
	
	inc si				; si++
	inc di				; di++
	loop CopyData_Loop	
	
	pop ds				; Restore ds

	ret

; # Get Next Cluster of a File
; Order of pushing stack: Current cluster No.
; Return value : ax
GetNextCluster:

	push word FAT0AreaOffset	; Read FAT0
	push word FAT0SectorCount
	call ReadFloppy

	pop ax				; Get return address
	pop bx				; Get current cluster
	push ax				; Restore return address

	mov ax, BufferSegment		; Set segment register of buffer
	mov es, ax

	mov ax, bx			; Copy bx, use ax as base number
	mov dx, bx			; Copy bx, use dx as ODD/EVEN flag
	and dx, 1			; Check the cluster No. is odd or even
	cmp dx, 0
	jz GetNextCluster_Even

	; For odd No., -> bx = (ax - 1) / 2 * 3 + 1	mask: fff0h
	; For even No. -> bx = ax / 2 * 3		mask: 0fffh
	
	; Odd
	dec ax				; Change current cluster No. to even number

GetNextCluster_Even:
	shr ax, 1			; ax / 2
	mov bx, ax			; bx = ax * 3
	add bx, ax
	add bx, ax

	cmp dx, 1
	jz GetNextCluster_Odd

	mov ax, [es:bx]			; Get next cluster No.
	and ax, 0fffh			; Only 12 bits indicate the next cluster No.
	
	ret
	
GetNextCluster_Odd:
	inc bx

	mov ax, [es:bx]			; Get next cluster No.
	shr ax, 4			; Only 12 bits indicate the next cluster
	ret

; #############################
;     Floppy Disk Utilities
; #############################

; # Read N Sector of Floppy Disk
; Order of pushing stack: Sector position, count of sectors 
ReadFloppy:
	pop bx				; Save return address
	pop cx				; Get count of sectors
        pop ax				; Get sector position (AX as dividend)
	push bx				; Restore return address
	push cx				; Save count of sectors        

	; Calculate physical position
	mov bl, FloppySectors		; bl is divisor
	div bl				; ax / bl = al(quotient) ... ah(remainder)
	
	mov ch, al			; Cylinder
	shr ch, 1
	mov cl, ah			; Start sector No.
	inc cl
	mov dh, al			; Header
	and dh, 1
	mov dl, 0			; Driver No. ==> 0 means Driver A
	mov bx, BufferSegment		; Set buffer address
	mov es, bx
	mov bx, BufferOffset

ReadFloppy_Read:
	pop ax				; Restore count of sectors
	push ax				; Save count of sectors
	mov ah, 02h			; Set Int 13h
	; Note: al will be the count of sectors (0~15)

	int 13h

	jc ReadFloppy_Read		; If error occurs, CF will be set. Then read again

	pop ax				; Clean the stack
	ret

