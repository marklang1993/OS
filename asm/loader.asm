%include "boot.inc"
%include "pm.inc"

; GDT Area










LABEL_LOADER:
	mov ax, cs					; Get code segment address
	mov ds, ax					; Set data segment address
	mov es, ax					; Set extra segment address
	mov bx, StackSegment				; Set stack segment address
	mov ss, bx
	mov sp, StackOffset				; Set stack pointer
	
	; Display Message
	push word Str_LoaderRunning			; Transfer the address of that string
	push word [StrLen_LoaderRunning]		; Transfer the length of that string
	push word 0100h					; Set 1 Row : 0 Column
	call WriteString
        
	; ##### Load Kernel ######
 
	; Read Root Folder	
	push word RootFolderAreaOffset			; Prepare for reading root folder area
	push word 1					; Read 1 sector
	call ReadFloppy

	xor cx, cx					; Clear cx as loop counter

KernelSearch_Loop:	
	mov al, RootFolderEntrySize			; Set size of Root Entry Struct 
	mul cl						; Calculate offset of current file name under root folder
	
	push cx						; Save loop variable

	push word KernelFileName			; Prepare for a string comparation 
	push ax
	push word RootFolderFileNameLength		; Maximum file name length 
	call StringCompare
	cmp cx, 0					; Check the string compare result
	jz KernelFound					; Kernel found if cx == 0
	
	pop cx						; Restore cx
	inc cx						; cx++
	cmp cx, RootFolderCheckFileCount		; Check maximum file count
	jnz KernelSearch_Loop
	
	; Cannot find Kernel
	push word Str_FindKernelFailed
	push word [StrLen_FindKernelFailed]
	push word 0100h					; Set 1 Row : 0 Column			
	call WriteString
	jmp $

KernelFound:
	pop cx						; Get the index of current file
	mov al, RootFolderEntrySize 			; Calculate current position in root folder area
	mul cl
	add ax, FileStartClusterOffset			; Calculate absolute position of stroing start cluster of KERNEL	
	mov bx, ax					; Set the offset of the area storing start cluster No.
	mov ax, BufferSegment				; Set the segment of the area storing start cluster No.
	mov es, ax
	mov ax, [es:bx]					; Get the start cluster of KERNEL

	xor cx, cx					; Clear cx and use it as counter for copying kernel data

LoadKernel:
	push ax						; Save current cluster No.
	sub ax, DataAreaStartClusterNo			; Calculate start position of KERNEL data area
	add ax, DataAreaOffset
	mov [KernelCopyPosition], cx			; Save cx

	push ax						; Pass the floppy sector position
	push word 1					; Read 1 sector
	call ReadFloppy

	mov cx, [KernelCopyPosition]			; Restore cx
	push word 0					; Buffer position
	push word cx					; Kernel data position
	push word FloppyBytesPerSector			; Length of data needed to copy
	call CopyKernel

	mov cx, [KernelCopyPosition]			; Restore cx
	add cx, FloppyBytesPerSector			; cx += FloppyBytesPerSector (= 512)
	jo LoadKernelOverflow				; If Kernel is more than 64KB, overflow
	mov [KernelCopyPosition], cx			; Save cx	

	; Check next cluster(sector)
	call GetNextCluster
	mov cx, [KernelCopyPosition]			; Restore cx
	cmp ax, EndClusterValue				; Compare with EndClusterValue
	jb LoadKernel					; ax < EndClusterValue => Continue Read


KernelLoaded:

	call FloppyMotorOff				; Turn off the floppy motor

	
	jmp KernelSegment:KernelOffset			; GO TO KERNEL
		

LoadKernelOverflow:
	push word Str_Overflow
	push word [StrLen_Overflow]
	push word 0200h					; Set 2 Row : 0 Column			
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

	dec cx				; cx--
	inc si				; si++
	inc di				; di++
	cmp cx, 0			; Check loop end
	jnz StringCompare_Loop

StringCompare_EndLoop:
	ret

; # Copy Kernel
; Order	of pushing stack: address of buffer position (ds:si), address of kernel position (es:di), length in bytes
CopyKernel:
	pop ax				; Save return address
	pop cx				; Get length
	pop di				; Get kernel position
	pop si				; Get buffer position
	push ax				; Restore return address

	push ds				; Save ds

	mov ax, BufferSegment		; Set ds for src
	mov ds, ax
	mov ax, KernelSegment		; Set es for dst
	mov es, ax

CopyKernel_Loop:
	mov al, [ds:si]			; Read 1 byte from buffer
	mov [es:di], al			; Write 1 byte to kernel code area
	
	inc si				; si++
	inc di				; di++
	dec cx				; cx--
	cmp cx, 0			; Check finished
	jnz CopyKernel_Loop

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


Str_LoaderRunning:		db		"Loader is running..."
StrLen_LoaderRunning:		dw		$ - Str_LoaderRunning
Str_FindKernelFailed:		db		"NO KERNEL"
StrLen_FindKernelFailed:	dw		$ - Str_FindKernelFailed
Str_Overflow:			db		"KERNEL IS TOO BIG"
StrLen_Overflow:		dw		$ - Str_Overflow

StrBuffer:			db		"XXXX"			; 4 Bytes Buffers
KernelFileName:			db		"KERNEL  BIN"		; 11 Bytes - kernel.bin
KernelCopyPosition:		dw		0
