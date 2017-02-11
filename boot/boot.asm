; #############################
;          Boot Sector
; #############################
%include "boot.inc"

org 		7c00h				; Load to memory address 0x7c00

; JUMP
jmp short LABEL_BOOTLOADER		; 2 Bytes short jmp
nop								; 1 Byte nop
								; 3 Bytes Totaly

; Header of FAT12 Floppy Disk
OEMName				db			'OEM Name'		; (8 Bytes) OEM String
BytesPerSector  	dw			512				; (2 Bytes) Count of Bytes per Sector
SectorPerCluster	db			1				; (1 Byte) Count of Sectors per Cluster
SectorsOfBoot		dw			1				; (2 Bytes) Count of Sectors for Booting
FATsCount			db 			2				; (1 Byte) Count of FATs
RootEntryCount		dw		    224				; (2 Bytes) Count of Root Entries
TotalSectors		dw			2880			; (2 Bytes)	Total count of sectors(2*80*18)
MediaDescriptor		db			0xf0			; (1 Byte) Media Descriptor
SectorsPerFAT		dw			9				; (2 Bytes) Count of sectors per FAT12
SectorsPerTrack		dw			18				; (2 Bytes) Count of sectors per Track
NumberOfHeads		dw			2				; (2 Bytes) Number of heads
HiddenSector		dd			0				; (4 Bytes) Count of hidden sectors
TotalSector			dd			0				; (4 Bytes) 
DriveNumber			db 			0				; (1 Byte) Drive Number for int 13h
Reserved			db			0				; (1 Byte) Not used
ExtendedBootSign	db			29h				; (1 Byte) Extended Boot Sign = 29h
VolumeID			dd			0				; (4 Bytes) ID of this volume
VolumeLabel			db			'VolumeLabel'	; (11 Bytes) Label of this volume
FileSystemType		db			'FAT12   '		; (8 Bytes)	File System Type

; Start of Boot Loader
LABEL_BOOTLOADER:
	mov ax, cs					; Get code segment address
	mov ds, ax					; Set data segment address
	mov es, ax					; Set extra segment address
	mov ss, ax					; Set stack segment address
	mov sp, BaseOfStackOffset	; Set stack pointer
	
	
	; Display Message
	call ClearScreen
	push word Str_Loading				; Transfer the address of that string
	push word [StrLen_Loading]			; Transfer the length of that string
	push word 0000h					; Set 0 Row : 0 Column
	call WriteString	
	add sp, 4					; Reset stack

	jmp $						; Stop at here

; #############################
;          Subrouting
; #############################

; WriteString
; Order of Pushing stack: length, address of string, column, row
WriteString:
	pop ax				; Save return address
	pop dx				; Get Row:Column
	pop cx				; Get the length of String from stack	
	pop bp				; Get the address of String from stack
	push ax				; Restore return address

	mov ah, 13h			; Parameter: show string
	mov al, 01h			; Parameter: continue to show
	mov bx, 000ch			; Page: 0, Back: Black, Font: Red
	int 10h			
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

; #############################
;              Data
; #############################
Str_Loading:			db 		"Booting From Floppy..."
StrLen_Loading:			dw		$ - Str_Loading



; End
times 510 - ($ - $$)	db 	0 
			dw 	0xaa55	; End mark of Boot Sector
