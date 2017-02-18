%include "boot.inc"
%include "pm.inc"
%include "page.inc"

[SECTION .text16]
[BITS 16]		; align with 16 bits
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
	jmp $						; Stop here

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
	call CopyData

	mov cx, [KernelCopyPosition]			; Restore cx
	add cx, FloppyBytesPerSector			; cx += FloppyBytesPerSector (= 512)
	jo LoadKernelOverflow				; If Kernel is more than 64KB, overflow
	mov [KernelCopyPosition], cx			; Save cx	

	; Check next cluster(sector)
	call GetNextCluster
	mov cx, [KernelCopyPosition]			; Restore cx
	cmp ax, EndClusterValue				; Compare with EndClusterValue
	jb LoadKernel					; ax < EndClusterValue => Continue Read
	
	; Kernel Loaded
	jmp KernelLoaded				; Jump to KernelLoaded

LoadKernelOverflow:
	push word Str_Overflow
	push word [StrLen_Overflow]
	push word 0200h					; Set 2 Row : 0 Column			
	call WriteString
	jmp $ 						; Stop here

KernelLoaded:
	call FloppyMotorOff				; Turn off the floppy motor
	call GetARDS					; Get ARDS

	; ##### Switch to Protect Mode  #####
	lgdt [GDT_Pointer]				; Load GDTR
	cli						; Turn off interrupt
	
	in al, 92h					; Turn on A20	
	or al, 2
	out 92h, al

	mov eax, cr0					; Set cr0 PE bit
	or eax, 1
	mov cr0, eax	

	; Jump to 32bit code (enter into protect mode)
	jmp dword GDT_FLAT_C_Selector:(LoaderBaseOffset + LABEL_LOADER_32BIT_CODE)

; #############################
;      Externel Functions
; #############################

CopyDataDstSegment		equ		KernelSegment
%include "utility_boot.inc"
%include "utility_loader.inc"


[SECTION .data16]
[BITS 16]

Str_LoaderRunning:		db		"Loader is running..."
StrLen_LoaderRunning:		dw		$ - Str_LoaderRunning
Str_FindKernelFailed:		db		"NO KERNEL"
StrLen_FindKernelFailed:	dw		$ - Str_FindKernelFailed
Str_Overflow:			db		"KERNEL IS TOO BIG"
StrLen_Overflow:		dw		$ - Str_Overflow

KernelFileName:			db		"KERNEL  BIN"		; 11 Bytes - kernel.bin
KernelCopyPosition:		dw		0


[SECTION .gdt]
[BITS 32]

; GDT Area
GDT_START:		Descriptor	0,		0,		0	; Empty desciptor for indexing
GDT_FLAT_C:		Descriptor	0,		0fffffh,	TYPE_C_E + TYPE_C_R + S_DC + P_T + D_EC_32 + G_4KB
GDT_FLAT_DRW:		Descriptor	0,		0fffffh,	TYPE_D_W + TYPE_D_R + S_DC + P_T + D_EC_32 + G_4KB
GDT_STACK_DRW:		Descriptor	010000h,	0fh,		TYPE_D_W + TYPE_D_R + S_DC + P_T + D_EC_32 + G_4KB
GDT_VIDEO:		Descriptor	0b8000h,	0ffffh,		TYPE_D_W + TYPE_D_R + S_DC + P_T + DPL_3

GDT_Length		equ		$ - GDT_START		; GDT Length
GDT_Pointer:		DTPointer	LoaderBaseOffset + GDT_START,	GDT_Length - 1

; GDT Selector
GDT_FLAT_C_Selector	equ		GDT_FLAT_C - GDT_START 
GDT_FLAT_DRW_Selector	equ		GDT_FLAT_DRW - GDT_START
GDT_STACK_DRW_Selector	equ		GDT_STACK_DRW - GDT_START 
GDT_VIDEO_Selector	equ		(GDT_VIDEO - GDT_START) + SEL_RPL_3


[SECTION .text32]
[BITS 32]

LABEL_LOADER_32BIT_CODE:

	; Initialize registers in 32-bit protect mode
	mov ax, GDT_FLAT_DRW_Selector		; Set ds, es, fs
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ax, GDT_STACK_DRW_Selector		; Set ss, esp
	mov ss, ax
	mov esp, StackBaseOffset

	mov ax, GDT_VIDEO_Selector		; Set gs for video memory
	mov gs, ax

	
	mov ah, 0fh
	mov al, 'P'

	mov [gs:((80 * 2 + 0) * 2)], ax
	jmp $


