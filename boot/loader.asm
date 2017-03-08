%include "boot/boot.inc"
%include "boot/pm.inc"
%include "boot/page.inc"

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
        
	; ##### Load Kernel Binary File ######
 
	; Read Root Folder	
	push word RootFolderAreaOffset			; Prepare for reading root folder area
	push word 1					; Read 1 sector
	call ReadFloppy

	xor cx, cx					; Clear cx as loop counter

KernelFileSearch_Loop:	
	mov al, RootFolderEntrySize			; Set size of Root Entry Struct 
	mul cl						; Calculate offset of current file name under root folder
	
	push cx						; Save loop variable

	push word KernelFileName			; Prepare for a string comparation 
	push ax
	push word RootFolderFileNameLength		; Maximum file name length 
	call StringCompare
	cmp cx, 0					; Check the string compare result
	jz KernelFileFound				; Kernel file found if cx == 0
	
	pop cx						; Restore cx
	inc cx						; cx++
	cmp cx, RootFolderCheckFileCount		; Check maximum file count
	jnz KernelFileSearch_Loop
	
	; Cannot find kernel file
	push word Str_FindKernelFileFailed
	push word [StrLen_FindKernelFileFailed]
	push word 0100h					; Set 1 Row : 0 Column			
	call WriteString
	jmp $						; Stop here

KernelFileFound:
	pop cx						; Get the index of current file
	mov al, RootFolderEntrySize 			; Calculate current position in root folder area
	mul cl
	add ax, FileStartClusterOffset			; Calculate absolute position of stroing start cluster of kernel file	
	mov bx, ax					; Set the offset of the area storing start cluster No.
	mov ax, BufferSegment				; Set the segment of the area storing start cluster No.
	mov es, ax
	mov ax, [es:bx]					; Get the start cluster of kernel file

	xor cx, cx					; Clear cx and use it as counter for copying kernel file data

LoadKernelFile:
	push ax						; Save current cluster No.
	sub ax, DataAreaStartClusterNo			; Calculate start position of kernel data area
	add ax, DataAreaOffset
	mov [KernelFileCopyPosition], cx		; Save cx

	push ax						; Pass the floppy sector position
	push word 1					; Read 1 sector
	call ReadFloppy

	mov cx, [KernelFileCopyPosition]		; Restore cx
	push word 0					; Buffer position
	push word cx					; Kernel file data position
	push word FloppyBytesPerSector			; Length of data needed to copy
	call CopyData

	mov cx, [KernelFileCopyPosition]		; Restore cx
	add cx, FloppyBytesPerSector			; cx += FloppyBytesPerSector (= 512)
	jo LoadKernelFileOverflow			; If kernel file is more than 64KB, overflow
	mov [KernelFileCopyPosition], cx		; Save cx	

	; Check next cluster(sector)
	call GetNextCluster
	mov cx, [KernelFileCopyPosition]		; Restore cx
	cmp ax, EndClusterValue				; Compare with EndClusterValue
	jb LoadKernelFile				; ax < EndClusterValue => Continue Read
	
	; Kernel Bin Loaded
	jmp KernelFileLoaded				; Jump to KernelLoaded

LoadKernelFileOverflow:
	push word Str_Overflow
	push word [StrLen_Overflow]
	push word 0200h					; Set 2 Row : 0 Column			
	call WriteString
	jmp $ 						; Stop here

KernelFileLoaded:
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
	jmp dword GDT_FLAT_CODE_Selector:(LoaderBaseOffset + LABEL_LOADER_32BIT_CODE)

; #############################
;      Externel Functions
; #############################

CopyDataDstSegment		equ		KernelFileSegment
%include "boot/utility_boot.inc"
%include "boot/utility_loader.inc"


[SECTION .data16]
[BITS 16]

Str_LoaderRunning:		db		"Loader is running..."
StrLen_LoaderRunning:		dw		$ - Str_LoaderRunning
Str_FindKernelFileFailed:	db		"NO KERNEL"
StrLen_FindKernelFileFailed:	dw		$ - Str_FindKernelFileFailed
Str_Overflow:			db		"KERNEL IS TOO BIG"
StrLen_Overflow:		dw		$ - Str_Overflow

KernelFileName:			db		"KERNEL  BIN"		; 11 Bytes - kernel.bin
KernelFileCopyPosition:		dw		0


[SECTION .gdt]
[BITS 32]

; GDT Area
GDT_START:		Descriptor	0,		0,		0	; Empty desciptor for indexing
GDT_FLAT_CODE:		Descriptor	0,		0fffffh,	TYPE_C_E + TYPE_C_R + S_DC + P_T + D_EC_32 + G_4KB
GDT_FLAT_DRW:		Descriptor	0,		0fffffh,	TYPE_D_W + TYPE_D_R + S_DC + P_T + D_EC_32 + G_4KB
GDT_VIDEO:		Descriptor	0b8000h,	0ffffh,		TYPE_D_W + TYPE_D_R + S_DC + P_T + DPL_3

GDT_Length		equ		$ - GDT_START		; GDT Length
GDT_Pointer:		DTPointer	LoaderBaseOffset + GDT_START,	GDT_Length - 1

; GDT Selector
GDT_FLAT_CODE_Selector	equ		GDT_FLAT_CODE - GDT_START 
GDT_FLAT_DRW_Selector	equ		GDT_FLAT_DRW - GDT_START
GDT_VIDEO_Selector	equ		(GDT_VIDEO - GDT_START) + SEL_RPL_3


[SECTION .text32]
[BITS 32]

LABEL_LOADER_32BIT_CODE:

	; Initialize registers in 32-bit protect mode
	mov ax, GDT_FLAT_DRW_Selector		; Set ds, es, fs, ss
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax
	mov esp, StackBaseOffset

	mov ax, GDT_VIDEO_Selector		; Set gs for video memory
	mov gs, ax

	; Display "P" for protect mode
	mov ah, 0fh
	mov al, 'P'
	mov [gs:((80 * 2 + 0) * 2)], ax


	; # Enable pageing
	mov eax, [ds:(LoaderBaseOffset + PhysicalMemorySize)]	; Get Memory Size
	shr eax, 22						; Calculate PDE count -- div by 0x400000 = 4MB per PDE
	mov [ds:(LoaderBaseOffset + PDECount)], eax		; Save PDE count

	; Initialize Page Directory
	push ecx						; Save PDE count
	mov ecx, eax						; Use ecx as loop count of PDE
	mov ebx, PDBaseAddress					; Set base address of page directory table
	mov eax, PTBaseAddress					; Set base address of page table	
	add eax, PAGE_P + PAGE_RW + PAGE_U + PDE_PS_4K		; Present + RW + User + 4K -- PDE
InitPageDir_Loop:
	mov [ebx], eax						; Save page directory entry
	add ebx, PageEntrySize					; ebx++ -- Each page directory entry has size of 4 Bytes
	add eax, PageSize					; eax++ -- Each page table has size of 4KB
	loop InitPageDir_Loop

	; Set ecx as loop counter
	pop ecx
	shl ecx, 10						; Count of PDE * 1024 --> shl 10 bits
	mov ebx, PTBaseAddress					; Set base address of page table
	xor eax, eax						; Set the base address of the memory space -- 0x0
	add eax, PAGE_P + PAGE_RW + PAGE_U + PTE_ND		; Present + RW + User + Not Written -- PTE	
InitPageTable_Loop:
	mov [ebx], eax						; Save page table entry
	add ebx, PageEntrySize					; ebx++ -- Each page table entry has size of 4 Bytes
	add eax, PageSize					; eax++ -- Each page has size of 4KB
	loop InitPageTable_Loop

	; Enable paging
	mov eax, PDBaseAddress					; Set cr3
	mov cr3, eax
	mov eax, cr0						; Set cr0
	or eax, 8000000h					; PG -- 32nd bit
	mov cr0, eax
	
	; Display "P" for paging	
	mov ah, 0fh
	mov al, 'P'
	mov [gs:((80 * 2 + 2) * 2)], ax

	; Copy kernel
	call CopyKernel

	; Jump to kernel
	jmp dword eax


; #############################
;    Utilities (Protect Mode)
; #############################

; C calling convention
; Order of pushing stack: from right to left
; Caller cleans the stack
; NOTE: ss is different with cs, ds, es

; ##### High Address #####
; |...				|
; |parameters from caller	|	+ 8 * n
; |return address		|	+ 4
; |caller's ebp			|	+ 0
; --------------------------------ebp
; |local variables		|	- 4 * n
; |saved changed registers	|
; |...				|
; ##### Low Address #####

; # void Memcpy(void *src, void *dst, uint32 size);
; @ src: source address in ds
; @ dst: destination address in ds
; @ size: size of memory copied
Memcpy:
	push ebp		; Save frame pointer
	mov ebp, esp		; Set new frame pointer

	push eax		; Save changed registers
	push ecx
	push esi
	push edi

	mov ecx, [ss:(ebp+16)]	; Get size of memory needed to be copied
	mov edi, [ss:(ebp+12)]	; Get dst address (wrt. ds)
	mov esi, [ss:(ebp+8)]	; Get src address (wrt. ds)

Memcpy_Loop:
	mov ax, [ds:esi]	; Get 1 byte from src memory
	mov [ds:edi], ax	; Write 1 byte to dst memory
	inc esi			; esi++
	inc edi			; edi++
	loop Memcpy_Loop

	pop edi			; Restore changed registers
	pop esi
	pop ecx
	pop eax

	pop ebp			; Restore ebp
	ret


; # void* CopyKernel();
; @ RETURN: kernel entry point address in cs
CopyKernel:
	push ebp		; Save frame pointer
	mov ebp, esp		; Set new frame pointer

	; # Local variables
	push dword 0		; Kernel entry point address
	push dword 0		; Program header table start offset
	push dword 0		; Program header table size	
	push dword 0		; Program header table count

	push eax		; Save changed registers
	push ebx
	push ecx
	
	; Read ELF header
	mov ebx, KernelFileBaseOffset		; Get kernel elf file base address
	mov eax, [ds:(ebx+0x18)]		; Get & save kernel entry point
	mov [ss:(ebp-4)], eax
	mov eax, [ds:(ebx+0x1c)]		; Get & save program header table start offset
	mov [ss:(ebp-8)], eax
	xor eax, eax				; Get & save program header table size
	mov ax, [ds:(ebx+0x2a)]
	mov [ss:(ebp-12)], eax
	xor eax, eax				; Get & save program header table count
	mov ax, [ds:(ebx+0x2c)]
	mov [ss:(ebp-16)], eax


	; Read program header table
	mov ecx, [ss:(ebp-16)]		; Get count of program header
	mov ebx, [ss:(ebp-8)]		; Get program header table start offset
	add ebx, KernelFileBaseOffset	; Calculate program header table start address

CopyKernel_ProcessPH_Loop:
	push ebx			; Call CopyKernel_ProgramSegment & Store offset
	call CopyKernel_ProgramSegment
	pop ebx				; Clean stack & Get offset
	
	mov eax, [ss:(ebp-12)]		; Goto next program header
	add ebx, eax
	
	loop CopyKernel_ProcessPH_Loop

	; # Clean & Return value
	pop ecx			; Restore changed registers
	pop ebx
	pop eax	

	mov eax, [ss:(ebp-4)]	; Get kernel entry point as return value
	add esp, 16		; Clear local variables
	pop ebp			; Restore ebp
	ret


; # void CopyKernel_ProgramSegment(void* ph_addr);
; @ ph_addr: Program header address in ds
CopyKernel_ProgramSegment:
	push ebp		; Save frame pointer
	mov ebp, esp		; Set new frame pointer

	; # Local variables
	push dword 0		; Program segment offset in the file
	push dword 0		; Program segment virtual address
	push dword 0		; Program segment size

	push eax		; Save changed registers
	push ebx
	push ecx

	mov ebx, [ss:(ebp+8)]	; Get program header address

	; Read program header
	mov eax, [ds:(ebx+4)]	; Get & Save program segment offset
	mov [ss:(ebp-4)], eax
	mov eax, [ds:(ebx+8)]	; Get & Save program segment virtual address
	mov [ss:(ebp-8)], eax
	mov eax, [ds:(ebx+16)]	; Get & Save program segment size
	mov [ss:(ebp-12)], eax

	; Check Size
	mov ecx, [ss:(ebp-12)]	; Get program segment size
	cmp ecx, 0
	jz CopyKernel_ProgramSegment_Done

	; Copy
	push ecx		; push size
	mov ebx, [ss:(ebp-8)]	; Get & push program segment virtual address
	push ebx
	mov ebx, [ss:(ebp-4)]	; Get & process & push program segment offset
	add ebx, KernelFileBaseOffset
	push ebx

	call Memcpy		; Copy the memory
	add esp, 12		; Clean the call stack


CopyKernel_ProgramSegment_Done:
	pop ecx			; Restore changed registers
	pop ebx
	pop eax	

	add esp, 12		; Clear local variables
	pop ebp			; Restore ebp
	ret


