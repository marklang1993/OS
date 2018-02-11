#include "drivers/fs/file_desc.h"

struct file_table_entry file_table[FILE_TABLE_SIZE];	/* File Table */


/*
 # Initialize file Table (Ring 0)
 */
void file_table_init(void)
{
    memset(file_table, 0x0, sizeof(file_table));
}
