#ifndef _PARTITION_TABLE_H_
#define _PARTITION_TABLE_H_

#include "type.h"

/*
 * Current supported hdd devices:
 *
 * hd0     : primary & master hdd
 * hd1~4   : primary & master MBR partition 1~4
 * hd1a~1p : primary & master logical partition 1~16 at MBR partition 1
 * hd2a~2p : primary & master logical partition 1~16 at MBR partition 2
 * hd3a~3p : primary & master logical partition 1~16 at MBR partition 3
 * hd4a~4p : primary & master logical partition 1~16 at MBR partition 4
 *
 * hd5     : primary & slave hdd
 * hd6~9   : primary & slave MBR partition 1~4
 * hd6a~6p : primary & slave logical partition 1~16 at MBR partition 1
 * hd7a~7p : primary & slave logical partition 1~16 at MBR partition 2
 * hd8a~8p : primary & slave logical partition 1~16 at MBR partition 3
 * hd9a~9p : primary & slave logical partition 1~16 at MBR partition 4
 */

/* Partition table macros */
#define BASE_PARTITION_TABLE		0x1be
#define PART_MAX_PART_MBR		4			/* Maximum partitions in MBR */
#define PART_MAX_L_PER_EX_PART		16			/* Maximum logical partitions per extended partition */
#define COUNT_M_PART_TABLE_ENTRY	PART_MAX_PART_MBR	/* Count of MBR partition table entries */
#define COUNT_L_PART_TABLE_ENTRY	2			/* Count of logic partition table entries */
#define PART_TYPE_EXTENDED		0x5			/* Type of extended partition */

/* Bootable value mask */
#define PART_BOOTABLE			0x80
#define PART_UNBOOTABLE			0x0

#pragma pack(push, 1)

/* Partition table entry struct */
struct partition_table_entry
{
	uint8 is_bootable;

	uint8 base_header;
	union {
		uint8 data;

		struct {
		uint8 base_sector:6;
		uint8 base_cylinder_h:2;
		} b;

	} base_cs;
	uint8 base_cylinder_l;

	uint8 type;

	uint8 limit_header;
	union {
		uint8 data;

		struct {
		uint8 limit_sector:6;
		uint8 limit_cylinder_h:2;
		} b;

	} limit_cs;
	uint8 limit_cylinder_l;
	
	uint32 base_sector_lba;
	uint32 cnt_sectors;
};
#define PARTITION_TABLE_ENTRY_SIZE	sizeof(struct partition_table_entry)

#pragma pack(pop)


#endif
