;##############################
;         Definition
;##############################
; Stack - Address Range		(1000h:FFFFh~1000h:0000h = 64 KB)
StackSegment 			equ 		01000h
StackOffset	   		equ 		0ffffh

; Common Buffer - Address Range (2000h:0000h~2000h:FFFFh = 64 KB)
BufferSegment			equ		02000h
BufferOffset			equ 		0000h

; Loader.bin - Address Range 	(3000h:0000h~3000h:FFFFh = 64 KB)
LoaderSegment			equ		03000h
LoaderOffset   			equ 		0000h

; Floppy Related
FloppyCylinders			equ		80
FloppyHeaders			equ		2
FloppySectors			equ		18
FloppyTotalSectors		equ		2880
FloppyBytesPerSector		equ		512

; FAT12 Related
RootFolderAreaOffset		equ		19
RootFolderEntrySize		equ		32
RootFolderFileNameLength	equ		11
RootFolderCheckFileCount	equ		FloppyBytesPerSector / RootFolderEntrySize
RootFolderEntryCount		equ		224
RootFolderAreaSize		equ		RootFolderEntryCount * RootFolderEntrySize / FloppyBytesPerSector
FileStartClusterOffset		equ		01ah
DataAreaOffset			equ		RootFolderAreaSize + RootFolderAreaOffset
DataAreaStartClusterNo		equ		2			; Data area starts from cluster 2
