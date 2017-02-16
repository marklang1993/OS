; #############################
;          Boot Sector
; #############################
%include "boot.inc"

org 		7c00h					; Load to memory address 07c0h:0000h = 7c00h

; JUMP
jmp short LABEL_BOOTLOADER				; 2 Bytes short jmp
nop							; 1 Byte nop
							; 3 Bytes Totaly

; Header of FAT12 Floppy Disk
OEMName			db			'OEM Name'		; (8 Bytes) OEM String
BytesPerSector  	dw			512			; (2 Bytes) Count of Bytes per Sector
SectorPerCluster	db			1			; (1 Byte) Count of Sectors per Cluster
SectorsOfBoot		dw			1			; (2 Bytes) Count of Sectors for Booting
FATsCount		db 			2			; (1 Byte) Count of FATs
RootEntryCount		dw			RootFolderEntryCount	; (2 Bytes) Count of Root Entries
TotalSectors		dw			2880			; (2 Bytes) Total count of sectors(2*80*18)
MediaDescriptor		db			0xf0			; (1 Byte) Media Descriptor
SectorsPerFAT		dw			9			; (2 Bytes) Count of sectors per FAT12
SectorsPerTrack		dw			18			; (2 Bytes) Count of sectors per Track
NumberOfHeads		dw			2			; (2 Bytes) Number of heads
HiddenSector		dd			0			; (4 Bytes) Count of hidden sectors
TotalSector		dd			0			; (4 Bytes) 
DriveNumber		db 			0			; (1 Byte) Drive Number for int 13h
Reserved		db			0			; (1 Byte) Not used
ExtendedBootSign	db			29h			; (1 Byte) Extended Boot Sign = 29h
VolumeID		dd			0			; (4 Bytes) ID of this volume
VolumeLabel		db			'VolumeLabel'		; (11 Bytes) Label of this volume
FileSystemType		db			'FAT12   '		; (8 Bytes)File System Type

; Start of Boot Loader
LABEL_BOOTLOADER:
	mov ax, cs					; Get code segment address
	mov ds, ax					; Set data segment address
	mov es, ax					; Set extra segment address
	mov bx, StackSegment				; Set stack segment address
	mov ss, bx
	mov sp, StackOffset				; Set stack pointer
	
	; Display Message
	call ClearScreen
	push word Str_Loading				; Transfer the address of that string
	push word [StrLen_Loading]			; Transfer the length of that string
	push word 0000h					; Set 0 Row : 0 Column
	call WriteString

	; ##### Boot Loader #####
	; Assume only 16 files in the root folder area (sector 19 on floppy = 512 bytes / 32 = 16 files)

	; Read Root Folder	
	push word RootFolderAreaOffset			; Prepare for reading root folder area
	call ReadFloppyOne

	xor cx, cx					; Clear cx as loop counter

LoaderSearch_Loop:	
	mov al, RootFolderEntrySize			; Set size of Root Entry Struct 
	mul cl						; Calculate offset of current file name under root folder
	
	push cx						; Save loop variable

	push word LoaderFileName			; Prepare for a string comparation 
	push ax
	push word RootFolderFileNameLength		; Maximum file name length 
	call StringCompare
	cmp cx, 0					; Check the string compare result
	jz LoaderFound					; Loader found if cx == 0
	
	pop cx						; Restore cx
	inc cx						; cx++
	cmp cx, RootFolderCheckFileCount		; Check maximum file count
	jnz LoaderSearch_Loop
	
	; Cannot find Loader
	push word Str_Failed
	push word [StrLen_Failed]
	push word 0100h					; Set 1 Row : 0 Column			
	call WriteString
	jmp $

LoaderFound:
	pop cx						; Get the index of current file
	mov al, RootFolderEntrySize 			; Calculate current position in root folder area
	mul cl
	add ax, FileStartClusterOffset			; Calculate absolute position of start cluster of LOADER	
	mov bx, ax
	
	mov ax, BufferSegment				; Set es = BufferSegment
	mov es, ax
	mov ax, [es:bx]
	push ax						; ### SAVE start cluster No. of Loader

	sub ax, DataAreaStartClusterNo			; Calculate start position of LOADER data area
	add ax, DataAreaOffset

	push ax						; Pass the floppy sector position
	call ReadFloppyOne				; Read one sector

	push word 0
	push word 0
	push word 512
	call CopyLoader

	pop ax						; ### Discard SAVE

	jmp LoaderSegment:LoaderOffset 

; #############################
;          Subrouting
; #############################

; WriteString
; Order of pushing stack: address of string, length, Row:Column
WriteString:
	pop ax				; Save return address
	pop dx				; Get Row:Column
	pop cx				; Get the length of String from stack	
	pop bp				; Get the address of String from stack
	push ax				; Restore return address

	mov ax, 0h			; Set es = 0000h since the string address is es:bp
	mov es, ax
	mov ah, 13h			; Parameter: show string
	mov al, 01h			; Parameter: continue to show
	mov bx, 000ch			; Page: 0, Back: Black, Font: Red
	int 10h			
	ret			

; WriteNumber(16-bit)
; Order of pushing stack: 16-bit number
WriteNumber:
	pop ax				; Save return address
	pop bx				; Get Number
	push ax				; Restore return address
	
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


	mov [StrBuffer], edx
	push word StrBuffer		; Go to display 
	push word 4
	push word 0100h
	call WriteString

	ret

; ClearScreen
ClearScreen:
	mov ah, 06h;		
	mov al, 00h;
	mov cx, 00h;
	mov dh, 17h;			; To 23 Row
	mov dl, 4fh			; To 79 Column	
	mov bh, 00h;		
	int 10h;
	ret;

; StringCompare
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

; CopyLoader
; Order	of pushing stack: address of buffer position (ds:si), address of loader position (es:di), length in bytes
CopyLoader:
	pop ax				; Save return address
	pop cx				; Get length
	pop di				; Get loader position
	pop si				; Get buffer position
	push ax				; Restore return address

	push ds				; Save ds

	mov ax, BufferSegment		; Set ds for src
	mov ds, ax
	mov ax, LoaderSegment		; Set es for dst
	mov es, ax

CopyLoader_Loop:
	mov al, [ds:si]			; Read 1 byte from buffer
	mov [es:di], al			; Write 1 byte to loader area
	
	inc si				; si++
	inc di				; di++
	dec cx				; cx--
	cmp cx, 0			; Check finished
	jnz CopyLoader_Loop

	pop ds				; Restore ds

	ret

; ### Floppy Disk Utilities ###

; Read One Sector of Floppy Disk
; Order of pushing stack: Sector position 
ReadFloppyOne:
	pop bx				; Save return address
        pop ax				; Get sector position (AX as dividend)
	push bx				; Restore return address
        
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

ReadFloppyOne_Read:
	mov ah, 02h			; Set Int 13h
	mov al, 1			; Read 1 sector

	int 13h

	jc ReadFloppyOne_Read		; If error occurs, CF will be set. Then read again

	ret

; #############################
;              Data
; #############################
Str_Loading:			db 		"Boot From Floppy..."
StrLen_Loading:			dw		$ - Str_Loading
Str_Failed:			db		"No Loader"
StrLen_Failed:			dw		$ - Str_Failed
StrBuffer:			db		"XXXX"			; 4 Bytes Buffers
LoaderFileName:			db		"LOADER  BIN"		; 11 Bytes - loader.bin

; End
times 510 - ($ - $$)	db 	0 
			dw 	0xaa55	; End mark of Boot Sector
