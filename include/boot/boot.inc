;##############################
;         Definition
;##############################
; Stack - Address Range		(1000h:FFFCh~1000h:0000h = 64 KB - 8B)
StackSegment 			equ 		01000h
StackOffset	   		equ 		0fffch
StackBaseOffset	   		equ 		StackSegment * 10h + StackOffset		; Used in protect mode, ss: GDT_STACK_DRW

; Common Buffer - Address Range (2000h:0000h~2000h:FFFFh = 64 KB)
BufferSegment			equ		02000h
BufferOffset			equ 		0000h
BufferBaseOffset		equ		BufferSegment * 10h + BufferOffset		; Used in protect mode

; Loader.bin - Address Range 	(3000h:0000h~3000h:FFFFh = 64 KB)
LoaderSegment			equ		03000h
LoaderOffset   			equ 		0000h
LoaderBaseOffset		equ		LoaderSegment * 10h + LoaderOffset		; Used in protect mode

; Kernel.bin - Address Range	(4000h:0000h~4000h:FFFFh = 64 KB)
KernelFileSegment		equ		04000h
KernelFileOffset		equ		0000h
KernelFileBaseOffset		equ		KernelFileSegment * 10h + KernelFileOffset	; Used in protect mode

; Kernel in Memory - Address Range	(5000h:0000h~5000h:FFFFh = 64 KB)
KernelSegment			equ		05000h
KernelOffset			equ		0000h
KernelBaseOffset		equ		KernelSegment * 10h + KernelOffset		; Used in protect mode



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
EndClusterValue			equ		0ff8h
FAT0AreaOffset			equ		1
FAT0SectorCount			equ		8		


