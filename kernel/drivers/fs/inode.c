#include "lib.h"
#include "drivers/fs/inode.h"

/* Inode global variables */
struct inode inode_table[INODE_TABLE_COUNT];	/* Inode Table */


/*
 # Initialize Inode related data (Ring 0)
 */
void inode_init(void)
{
    /* Init. inode table */
	memset(inode_table, 0x0, sizeof(inode_table));
}