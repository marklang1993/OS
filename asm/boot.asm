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
	push word 1					; Read 1 sector
	call ReadFloppy

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
	jmp $						; Stop here

LoaderFound:
	pop cx						; Get the index of current file
	mov al, RootFolderEntrySize 			; Calculate current position in root folder area
	mul cl
	add ax, FileStartClusterOffset			; Calculate absolute position of stroing start cluster of LOADER	
	mov bx, ax					; Set the offset of the area storing start cluster No.
	mov ax, BufferSegment				; Set the segment of the area storing start cluster No.
	mov es, ax
	mov ax, [es:bx]					; Get the start cluster of LOADER

	xor cx, cx					; Clear cx and use it as counter for copying loader data

LoadLoader:
	push ax						; Save current cluster No.
	sub ax, DataAreaStartClusterNo			; Calculate start position of LOADER data area
	add ax, DataAreaOffset
	mov [LoaderCopyPosition], cx			; Save cx

	push ax						; Pass the floppy sector position
	push word 1					; Read 1 sector
	call ReadFloppy

	mov cx, [LoaderCopyPosition]			; Restore cx
	push word 0					; Buffer position
	push word cx					; Loader data position
	push word FloppyBytesPerSector			; Length of data needed to copy
	call CopyData

	mov cx, [LoaderCopyPosition]			; Restore cx
	add cx, FloppyBytesPerSector			; cx += FloppyBytesPerSector (= 512)
	jo LoadLoaderOverflow				; If loader is more than 64KB, overflow
	mov [LoaderCopyPosition], cx			; Save cx	

	; Check next cluster(sector)
	call GetNextCluster
	mov cx, [LoaderCopyPosition]			; Restore cx
	cmp ax, EndClusterValue				; Compare with EndClusterValue
	jb LoadLoader					; ax < EndClusterValue => Continue Read

	jmp LoaderSegment:LoaderOffset			; GO TO LOADER 

LoadLoaderOverflow:
	push word Str_Overflow
	push word [StrLen_Overflow]
	push word 0100h					; Set 1 Row : 0 Column			
	call WriteString
	jmp $						; Stop here

; #############################
;          Utilities
; #############################

CopyDataDstSegment		equ		LoaderSegment
%include "utility_boot.inc"


; #############################
;              Data
; #############################
Str_Loading:			db 		"Boot from floppy..."
StrLen_Loading:			dw		$ - Str_Loading
Str_Failed:			db		"NO LOADER"
StrLen_Failed:			dw		$ - Str_Failed
Str_Overflow:			db		"LOADER IS TOO BIG"
StrLen_Overflow:		dw		$ - Str_Overflow
LoaderFileName:			db		"LOADER  BIN"		; 11 Bytes - loader.bin
LoaderCopyPosition:		dw		0

; End
times 510 - ($ - $$)	db 	0 
			dw 	0xaa55	; End mark of Boot Sector
