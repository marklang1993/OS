; ##### GDT / LDT Descriptor #####
; Please refer wiki page: Global Descriptor Table

; Usage: Descriptor Base, Limit, Attribute
;	Base Address	: 32 bits
;	Segment Limit	: 20 bits
;	Attribute	: See Attribute Flag (12 bits)

%macro	Descriptor 3	; 3 means this macro takes 3 parameters

	dw	%2 & 0ffffh				; Segment Limit (bit 0-15)
	dw	%1 & 0ffffh				; Base Address (bit 0-15)
	db	(%1 >> 16) & 0ffh 			; Base Address (bit 16-23)
	db	%3 & 0ffh				; P, DPL, S, Type
	db	((%2 >> 16) & 0fh) | ((%3 >> 4) & 0f0h)	; G, DB, AVL, Segment Limit (bit 16-19)
	db	(%1 >> 24) & 0ffh			; Base Address (bit 24-31)

%endmacro

; Each descriptor has size of 64 bits


; ##### Attribute Flag #####

; # G
G_BYTE		equ		0		; Limit granularity is 1 Byte
G_4KB		equ		800h		; Limit granularity is 4 KB

; # DB


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

; # Type 
; -- Data Segment
TYPE_D_R	equ		0		; Reable
TYPE_D_A	equ		1		; Accessed
TYPE_D_W	equ		2		; Writable
TYPE_D_E	equ		4		; Expand-Down
; -- Code Segment
TYPE_C_E	equ		8		; Execute
