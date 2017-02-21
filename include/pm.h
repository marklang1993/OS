// Protect mode struct definition

#ifndef _PM_H_
#define _PM_H_

#include "type.h"

#pragma pack(2)
// NOTE: if not pack(2), then gcc will add 2-byte padding for any 16-bit data member -- default is pack(4). 
// NOTE: asm related struct, order is important

struct descriptor 
{
	uint16	segment_limit_1;	// Segment Limit (bit 0-15)
	uint16	base_address_1;		// Base Address (bit 0-15)
	uint8	base_address_2;		// Base Address (bit 16-23)
	uint8	attribute_1;		// P, DPL, S, Type
	uint8	attribute_2;		// G, D/B, AVL, Segment Limit (bit 16-19)
	uint8	base_address_3;		// Base Address (bit 24-31)
};

#define DESCRIPTOR_SIZE		sizeof(struct descriptor)

struct descriptor_ptr
{
	uint16 limit;			// limit of array of descriptors
	struct descriptor *ptr_base;	// Descriptors' base address
};

#endif
