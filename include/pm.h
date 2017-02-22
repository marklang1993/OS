// Protect mode struct definition

#ifndef _PM_H_
#define _PM_H_

#include "type.h"


// NOTE: must be same as the definition in kernel.asm & pm.inc
// # GDT / LDT / IDT Attribute Flags

// G
#define G_BYTE		0		// Limit granularity is 1 Byte
#define G_4KB		0x800		// Limit granularity is 4 KB

// D/B
// -- D bit : Executable code segment descriptor
#define D_EC_32		0x400		// 32 bits address, 32 / 8 bits imm number
#define D_EC_16		0		// 16 bits address, 16 / 8 bits imm number
// -- B bit : Expand-down data segment descriptor
#define B_ED_4GB	0x400		// Segment upper limit = 4 GB
#define B_ED_64KB	0		// Segment upper limit = 64 KB
// -- B / D : Stack segment descriptor
#define BD_S_32		0x400		// push, pop, call, etc... -- Use esp
#define BD_S_16		0		// push, pop, call, etc..  -- Use sp

// AVL -- Used by system software

// P
#define P_T		0x80		// Present in memory
#define P_F		0		// Not exist in memory

// DPL
#define DPL_0		0		// DPL 0
#define DPL_1		0x20		// DPL 1
#define DPL_2		0x40		// DPL 2
#define DPL_3		0x60		// DPL 3

// S
#define S_DC		0x10		// Data / Code segment
#define S_SG		0		// System / Gate segment

// Type - Data, Code, System, Gate 
// -- Data Segment
#define TYPE_D_R	0		// Reable
#define TYPE_D_A	1		// Accessed
#define TYPE_D_W	2		// Writable
#define TYPE_D_E	4		// Expand-Down
// -- Code Segment
#define TYPE_C_E	8		// Execute
#define TYPE_C_A	1		// Accessed
#define TYPE_C_R	2		// Read
#define TYPE_C_C	4		// Confirmed
// -- System
#define TYPE_S_A286TSS	1		// Available 286 TSS
#define TYPE_S_LDT	2		// LDT
#define TYPE_S_B286TSS	3		// Busy 286 TSS
#define TYPE_S_A386TSS	9		// Available 386 TSS
#define TYPE_S_B386TSS	0x0b		// Busy 386 TSS
// -- Gate
#define TYPE_G_286CALL	4		// 286 Call Gate
#define TYPE_G_286INT	6		// 286 Interrupt Gate
#define TYPE_G_286TRAP	7		// 286 Trap Gate
#define TYPE_G_386CALL	0x0c		// 386 Call Gate
#define TYPE_G_386INT	0x0e		// 386 Interrupt Gate
#define TYPE_G_386TRAP	0x0f		// 386 Trap Gate
#define TYPE_G_TASK	5		// Task Gate

// GDT / LDT / IDT Selector Flags
#define SEL_TI_L	4	// LDT Selector
#define SEL_TI_G	0	// GDT Selector
#define SEL_RPL_0	0	// RPL 0
#define SEL_RPL_1	1	// RPL 1
#define SEL_RPL_2	2	// RPL 2
#define SEL_RPL_3	3	// RPL 3

// Kernel GDT Selector
#define KERNEL_GDT_FLAT_C_SELECTOR	1 << 3
#define KERNEL_GDT_FLAT_DRW_SELECTOR	2 << 3
#define KERNEL_GDT_STACK_SELECTOR	3 << 3
#define KERNEL_GDT_VIDEO_SELECTOR	(4 << 3) + SEL_RPL_3


#pragma pack(push, 2)
// NOTE: if not pack(1), then gcc will add 1~3 bytes padding for any non 32-bit data member -- default is pack(4). 
// NOTE: asm related struct, order is important

/* GDT/LDT */
struct descriptor 
{
	uint16	segment_limit_1;		// Segment Limit (bit 0-15)
	uint16	base_address_1;			// Base Address (bit 0-15)
	uint8	base_address_2;			// Base Address (bit 16-23)
	uint8   attribute_1;			// P, DPL, S, Type
	uint8	attribute_2;			// G, D/B, AVL, Segment Limit (bit 16-19)
	uint8   base_address_3;			// Base Address (bit 24-31)		
};
// NOTE: must use uint16, since there is no PADDING added in asm code.

#define DESCRIPTOR_SIZE			sizeof(struct descriptor)

/* Call/Interrupt/Trap Gate  */
struct gate_descriptor
{
	uint16 offset_1;		// Offset(bit 0-15) w.r.t within the segment decribed by selector
	uint16 selector;		// Selector of interrupt handler code
	uint8 count_param;		// Count of param (0~31) / Reserved (Interrupt gate)
	uint8 attribute;		// Attribute
	uint16 offset_2;		// Offset(bit 16-31) w.r.t within the segment decribed by selector
};

#define GATE_DESCRIPTOR_SIZE		sizeof(struct gate_descriptor)

/* Pointer of all kinds of descriptor */
struct descriptor_ptr
{
	uint16 limit;			// limit of array of descriptors
	void *ptr_base;			// Descriptors' base address
};

#pragma pack(pop)

#endif