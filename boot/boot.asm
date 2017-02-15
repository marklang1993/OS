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
RootEntryCount		dw			224			; (2 Bytes) Count of Root Entries
TotalSectors		dw			2880			; (2 Bytes)	Total count of sectors(2*80*18)
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
FileSystemType		db			'FAT12   '		; (8 Bytes)	File System Type

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

	; Test data	
	push word 0h
	call ReadFloppyOne

	jmp $						; Stop at here

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

; ### Floppy Disk Utilities ###

; Read One Sector of Floppy Disk
; Order of pushing stack: Sector position   
ReadFloppyOne:
	pop bx				; Save return address
        pop ax				; Get sector position (AX as dividend)
	push bx				; Restore return address
        
	; Calculate physical position
	mov bl, 18			; bl is divisor
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

	; Debug Print
	mov ax, [es:10]
	push ax
	call WriteNumber
	
	ret

; #############################
;              Data
; #############################
Str_Loading:			db 		"Booting From Floppy..."
StrLen_Loading:			dw		$ - Str_Loading
StrBuffer:			db		"XXXX"			; 4 Bytes Buffers
FloppySectorsPerTrack:		dw		18


; End
times 510 - ($ - $$)	db 	0 
			dw 	0xaa55	; End mark of Boot Sector
