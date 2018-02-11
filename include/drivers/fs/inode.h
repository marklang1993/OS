#ifndef _FS_INODE_H_
#define _FS_INODE_H_

#include "drivers/fs/dinode.h"

#define INODE_TABLE_COUNT 128 /* count of inode in inode table */

/* inode status */
#define INODE_INVALID	0	/* inode is invalid */
#define INODE_VALID		1	/* inode is valid and resource is available */
#define INODE_BUSY		2	/* corresponding resource is busy */

/* inode struct stored in memory */
struct inode {
	/*
	 * Some comments here:
	 * 1. Both "major_dev" and "minor_dev" in inode will be same as dinode's
	 *    if the FILE is a real file on the disk, not device;
	 * 2. "dinode_index" will be the index of dinode if the FILE is a real file
	 *    on the disk;
	 * 3. If FILE is a device, both "dinode_index" and "disk_inode" are INVALID.
	 */
	uint32 major_dev;			/* Major Device Number */
	uint32 minor_dev;			/* Minor Device Number */
	uint32 ref_cnt;				/* Reference Count - Open/Close */
	uint32 status;				/* inode Status */

	/* Used by real file only */
	uint32 mbr_index;			/* MBR index of corresponding partition */
	uint32 logical_index;		/* Logical index of corresponding partition */
	uint32 dinode_index;		/* Index of dinode */
	struct dinode disk_inode;	/* Disk inode Copy */
};

/* inode related functions */
void inode_table_init(void);

/* NOTE: All these functions must be called by the functions from file_desc.c */
/* Create inode from dinode */
int32 create_inode_from_file(
	uint32 dinode_index,
	const struct fs_partition_descriptor *ptr_descriptor
);
/* Create inode from device */
int32 create_inode_from_device(
	uint32 major_dev,
	uint32 minor_dev
);

/* Remove a to file table entry */
BOOL unlink_inode(uint32 inode_index);

#endif
