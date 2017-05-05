#ifndef _PARTITION_TABLE_H_

#include "type.h"

#define BYTE_PER_SECTOR			512
#define BASE_PARTITION_TABLE		0x1be
#define COUNT_M_PART_TABLE_ENTRY	4
#define COUNT_L_PART_TABLE_ENTRY	2
#define PART_TYPE_EXTENDED		5

#define BOOTABLE			0x80
#define UNBOOTABLE			0x0

#pragma pack(push, 1)

struct partition_table_entry
{
	uint8 is_bootable;

	uint8 base_header;
	union {
		uint8 data;
		uint8 base_sector:6;
		uint8 base_cylinder_h:2;
	} base_cs;
	uint8 base_cylinder_l;

	uint8 type;

	uint8 limit_header;
	union {
		uint8 data;
		uint8 limit_sector:6;
		uint8 limit_cylinder_h:2;
	} limit_cs;
	uint8 limit_cylinder_l;
	
	uint32 base_sector_lba;
	uint32 cnt_sectors;
};
#define PARTITION_TABLE_ENTRY_SIZE	sizeof(struct partition_table_entry)

#pragma pack(pop)


#endif
