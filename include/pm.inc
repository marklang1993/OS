; ##### GDT / LDT Descriptor #####
; Please refer wiki page: Global Descriptor Table

; Usage: Descriptor Base, Limit, Attribute
;	Base Address	: 32 bits
;	Segment Limit	: 20 bits
;	Attribute	: See Attribute Flag (12 bits)
; Each descriptor has size of 64 bits

%macro	Descriptor 3	; 3 means this macro takes 3 parameters

	dw	%2 & 0ffffh				; Segment Limit (bit 0-15)
	dw	%1 & 0ffffh				; Base Address (bit 0-15)
	db	(%1 >> 16) & 0ffh 			; Base Address (bit 16-23)
	db	%3 & 0ffh				; P, DPL, S, Type
	db	((%2 >> 16) & 0fh) | ((%3 >> 4) & 0f0h)	; G, D/B, AVL, Segment Limit (bit 16-19)
	db	(%1 >> 24) & 0ffh			; Base Address (bit 24-31)

%endmacro


; ##### Descriptor Table Pointer (GDT / LDT)#####

; Usage: Limit, Base
;	Base Address	: 32 bits (base address of DT)
;	Limit		: 16 bits (length of DT - 1)
; Each descriptor table pointer has size of 48 bits

%macro DTPointer 2	; 2 means this macro takes 2 parameters

	dw	%2		; Descriptor Table Limit (bit 0-15)
	dd	%1		; Base Address (bit 16 - 47)

%endmacro


; ##### Attribute Flag #####

; # G
G_BYTE		equ		0		; Limit granularity is 1 Byte
G_4KB		equ		800h		; Limit granularity is 4 KB

; # D/B
; -- D bit : Executable code segment descriptor
D_EC_32		equ		400h		; 32 bits address, 32 / 8 bits imm number
D_EC_16		equ		0		; 16 bits address, 16 / 8 bits imm number
; -- B bit : Expand-down data segment descriptor
B_ED_4GB	equ		400h		; Segment upper limit = 4 GB
B_ED_64KB	equ		0		; Segment upper limit = 64 KB
; -- B / D : Stack segment descriptor
BD_S_32		equ		400h		; push, pop, call, etc... -- Use esp
BD_S_16		equ		0		; push, pop, call, etc..  -- Use sp

; # AVL
; Used by system software

; # P
P_T		equ		80h		; Present in memory
P_F		equ		0		; Not exist in memory

; # DPL
DPL_0		equ		0		; DPL 0
DPL_1		equ		20h		; DPL 1
DPL_2		equ		40h		; DPL 2
DPL_3		equ		60h		; DPL 3

; # S
S_DC		equ		10h		; Data / Code segment
S_SG		equ		0		; System / Gate segment

; # Type - Data, Code, System, Gate 
; -- Data Segment
TYPE_D_R	equ		0		; Reable
TYPE_D_A	equ		1		; Accessed
TYPE_D_W	equ		2		; Writable
TYPE_D_E	equ		4		; Expand-Down
; -- Code Segment
TYPE_C_E	equ		8		; Execute
TYPE_C_A	equ		1		; Accessed
TYPE_C_R	equ		2		; Read
TYPE_C_C	equ		4		; Confirmed
; -- System
TYPE_S_A286TSS	equ		1		; Available 286 TSS
TYPE_S_LDT	equ		2		; LDT
TYPE_S_B286TSS	equ		3		; Busy 286 TSS
TYPE_S_A386TSS	equ		9		; Available 386 TSS
TYPE_S_B386TSS	equ		0bh		; Busy 386 TSS
; -- Gate
TYPE_G_286CALL	equ		4		; 286 Call Gate
TYPE_G_286INT	equ		6		; 286 Interrupt Gate
TYPE_G_286TRAP	equ		7		; 286 Trap Gate
TYPE_G_386CALL	equ		0ch		; 386 Call Gate
TYPE_G_386INT	equ		0eh		; 386 Interrupt Gate
TYPE_G_386TRAP	equ		0fh		; 386 Trap Gate
TYPE_G_TASK	equ		5		; Task Gate

; ##### Selector Flags #####
SEL_TI_L	equ		4		; LDT Selector
SEL_TI_G	equ		0		; GDT Selector
SEL_RPL_0	equ		0		; RPL 0
SEL_RPL_1	equ		1		; RPL 1
SEL_RPL_2	equ		2		; RPL 2
SEL_RPL_3	equ		3		; RPL 3


