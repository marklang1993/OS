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
	jmp $

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
	call CopyLoader

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
	jmp $

; #############################
;          Utilities
; #############################

; # WriteString
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

; # ClearScreen
ClearScreen:
	mov ah, 06h;		
	mov al, 00h;
	mov cx, 00h;
	mov dh, 17h;			; To 23 Row
	mov dl, 4fh			; To 79 Column	
	mov bh, 00h;		
	int 10h;
	ret;

; # StringCompare
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

; # CopyLoader
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
