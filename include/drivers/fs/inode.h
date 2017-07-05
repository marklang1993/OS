#ifndef _FS_INODE_H_
#define _FS_INODE_H_

#include "drivers/fs/dinode.h"

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

#endif
