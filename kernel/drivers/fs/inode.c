#include "dbg.h"
#include "lib.h"
#include "drivers/fs/inode.h"

/* Inode global variables */
struct inode inode_table[INODE_TABLE_COUNT];	/* Inode Table */


/*
 # Initialize inode Table (Ring 0)
 */
void inode_table_init(void)
{
    /* Init. inode table */
	memset(inode_table, 0x0, sizeof(inode_table));
}

/*
 * Create inode from dinode
 * @ dinode_index  : index of dinode of corresponding file
 * @ ptr_descriptor: file system partition descriptor
 * @ RETURN        : inode table index, -1 when error
 */
int32 create_inode_from_file(
	uint32 dinode_index,
	const struct fs_partition_descriptor *ptr_descriptor
)
{
    struct inode *inode_ptr;

    uint32 i;
    BOOL ret;

    /* Check has the dinode already existed */
    for (i = 0; i < INODE_TABLE_COUNT; ++i) {
        if (inode_table[i].status == INODE_VALID) {
            /* The dinode has already existed - Valid */
            inode_table[i].ref_cnt += 1; /* Simply increase the ref_cnt */
            return i;

        } else if (inode_table[i].status == INODE_BUSY) {
            /* The dinode has already existed - Busy */
            panic("DINODE EXISTED BUT BUSY\n");
        }
    }

    /* 
     * The given dinode is not found;
     * Find a slot for newly created inode.
     */
    for (i = 0; i < INODE_TABLE_COUNT; ++i) {
        if (inode_table[i].status == INODE_INVALID) {
            inode_ptr = &inode_table[i];
        }
    }
    /* inode table is full */
    if (i == INODE_TABLE_COUNT) {
        panic("INODE TABLE IS FULL\n");
    }

    /* Read dinode */
    ret = read_dinode(
        dinode_index,
        ptr_descriptor,
        &(inode_ptr->disk_inode)
    );
    kassert(IS_TRUE(ret));

    /* Set other parameters */
    inode_ptr->ref_cnt = 1;
    inode_ptr->mbr_index = ptr_descriptor->mbr_index;
    inode_ptr->logical_index = ptr_descriptor->logical_index;
    inode_ptr->dinode_index = dinode_index;
    inode_ptr->status = INODE_VALID;

    return i; /* Return index of inode table */
}

/*
 * Create inode from device
 * @ major_dev: major device number
 * @ minor_dev: minor device number
 * @ RETURN   : inode table index, -1 when error
 */
int32 create_inode_from_device(
	uint32 major_dev,
	uint32 minor_dev
)
{
    kassert(FALSE);
    return -1;
}

/*
 * Remove a to file table entry
 * @ inode_index: index of inode
 */
BOOL unlink_inode(uint32 inode_index)
{
    /* Do some checking */
    if (inode_index >= INODE_TABLE_COUNT) {
        return FALSE;
    }
    if (inode_table[inode_index].status != INODE_VALID) {
        return FALSE;
    }

    kassert(inode_table[inode_index].ref_cnt != 0);
    inode_table[inode_index].ref_cnt -= 1;
    /* If this inode is not refered by anyone, remove it */
    if (inode_table[inode_index].ref_cnt == 0) {
        memset(&(inode_table[inode_index]), 0, sizeof(struct inode));
    }

    return TRUE;
}