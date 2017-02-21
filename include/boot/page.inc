; ##### Paging Address#####
PDBaseAddress			equ		0100000h		; Page directory base address
PTBaseAddress			equ		PDBaseAddress + 01000h	; Page table start from 4KB page directory
PDEMaxCount			equ		0400h			; 1024 entries
PTEMaxCount			equ		0400h			; 1024 entries
PageEntrySize			equ		4			; Each paging entries takes 4 bytes 
PageSize			equ		01000h			; 1 page = 4KB

PDECount:			dw		0h			; Initialized after getting memory size


; ##### Paging Attribute Flag #####

; # Shared by Page-Directory Entry, Page-Table Entry
; # P
PAGE_NP		equ		0	; Presented in the memory
PAGE_P		equ		1	; Not presented in the memory

; # R/W
PAGE_R		equ		0	; Read only
PAGE_RW		equ		2	; Read / Write

; # U/S
PAGE_S		equ		0	; System level
PAGE_U		equ		4	; User level

; # PWT -- ignored when CD bit is cleared in cr0
PAGE_WB		equ		0	; Write-back used
PAGE_WT		equ		8	; Write-through used

; # PCD -- ignored when CD bit is cleared in cr0
PAGE_CACHE	equ		0	; Can be cached
PAGE_NCACHE	equ		10h	; Cannot be cached

; # A -- CPU will NOT clear this bit automatically. Set when accessed in 1st time.
PAGE_NA		equ		0	; Not accessed
PAGE_A		equ		20h	; Accessed

; # G -- If G is set and PGE bit is set in cr4, then this entry will be valid forever in TLB
PAGE_NG		equ		0	; Not global
PAGE_G		equ		100h	; Global


; # Page-Directory Entry
; # PS
PDE_PS_4K	equ		0	; Each page is 4KB
PDE_PS_OTHER	equ		80h	; Other size for each page


; # Page-Table Entry
; # D -- CPU will NOT clear this bit automatically. Set when written in 1st time.
PTE_ND		equ		0	; Not written
PTE_D		equ		40h	; Written (Dirty)

; # PAT -- Not used (Supported since Pentium III)
