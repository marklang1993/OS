#ifndef _FS_INODE_H_
#define _FS_INODE_H_

#include "drivers/fs/fs.h"
#include "type.h"

#pragma pack(push, 1)

/* dinode common macros */
#define DINODE_DIRECT_CNT	11	/* Direct reference count */
#define DINODE_INDIRECT_CNT	1	/* Indirect reference count */
#define DINODE_REF_CNT		(DINODE_DIRECT_CNT + DINODE_INDIRECT_CNT)

/* dinode type */
#define DINODE_INVALID		0
#define DINODE_FILE		1
#define DINODE_DIRECTORY	2
#define DINODE_DEVICE		3

/* Null reference */
#define DINODE_REF_NULL		0

/* inode struct stored on disk */
struct dinode {
	uint16 type;				/* Resource Type */
	uint16 link_cnt;			/* Count of Links to this dinode */
	/* Used by DINODE_DEVICE */
	uint32 major_dev;			/* Major Device Number */
	uint32 minor_dev;			/* Minor Device Number */
	/* Used by DINODE_FILE */
	uint32 size;				/* File Size in Bytes */
	uint32 block_ref[DINODE_REF_CNT];	/* Block References */
};
#define DINODE_SIZE		sizeof(struct dinode)

/* indirect block reference */
struct indir_block_ref {
	uint32 block_ref[FS_BYTES_PER_BLOCK / sizeof(uint32) - 1];
	uint32 next_ref;	/* Next indir_block_ref */
};


/* inode status */
#define INODE_INVALID	0	/* inode is invalid */
#define INODE_VALID	1	/* inode is valid and resource is available */
#define INODE_BUSY	2	/* corresponding resource is busy */

/* inode struct stored in memory */
struct inode {
	uint32 major_dev;		/* Major Device Number */
	uint32 minor_dev;		/* Minor Device Number */
	uint32 inode_num;		/* inode Number (dinode Index...) */
	uint32 ref_cnt;			/* Reference Count */
	uint32 status;			/* inode Status */

	struct dinode disk_inode;	/* Disk inode Copy */
};

#pragma pack(pop)

#endif
