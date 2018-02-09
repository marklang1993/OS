#include "dbg.h"
#include "type.h"
#include "lib.h"
#include "drivers/fs/dinode.h"
#include "drivers/fs/directory.h"
#include "drivers/fs/fs_lib.h"

/*
 # Parse a path (TODO: support relative path, currently only full path is supported)
 * path          : full file path
 * mode          : file open mode
 * ptr_descriptor: file system partition descriptor
 @ RETURN        : target dinode index
 */
static int32 parse_path(
	const char *path,
	FILELIB_OP_FLAG mode,
	const struct fs_partition_descriptor *ptr_descriptor
)
{
	uint32 len_path = strlen(path);
	char cur_name[FS_MAX_PATH_LENGTH];
	struct dinode cur_dinode;
	int32 dinode_index = DINODE_ROOT_DIRE_IDX; /* Assume: start from root directory */
	int32 next_dinode_index = -1;
	/* position of last occurance of the slash */
	const char *ptr_pos_last;
	uint32 pos_last;

	uint32 i, j;
	uint32 cnt;
	BOOL ret;

	/* Check path length */
	if (len_path >= FS_MAX_PATH_LENGTH) {
		panic("PATH IS LONGER THAN %d\n", FS_MAX_PATH_LENGTH);
	} else if (len_path == 1) {
		panic("PATH LENGTH IS 1.\n");
	}

	/* Get position of last occurance of the slash */
	ptr_pos_last = strrchr(path, '/');
	pos_last = (uint32)ptr_pos_last - (uint32)path;
	/* printk("last occurance is %d\n", pos_last); */

	/* Traverse the path */
	for(i = 0; i < len_path; ++i)
	{
		if (path[i] == '/'){
			/* Find target file OR next directory */
			i = i + 1; /* Skip the slash */
			for (j = i; j < len_path; ++j)
			{
				if (path[j] == '/'){
					--j;
					break;
				}
			}

			/* path[i] ~ path[j] is the file name OR next directory name */
			/* Construct that name */
			cnt = j - i + 1; /* calculate length of (current) name */
			memcpy(cur_name, &path[i], cnt);
			cur_name[cnt] = '\0';
			/* printk("current file name: %s\n", cur_name); */

			/* Find next "file" */
			ret = read_dinode(dinode_index, ptr_descriptor, &cur_dinode);
			if (NOT(IS_TRUE(ret))) {
				/* probably this is a file system error, must panic */
				panic("READ DINODE %d ERROR!\n", dinode_index);
			}
			next_dinode_index = search_dinode_by_directory(&cur_dinode, cur_name, ptr_descriptor);
			/* printk("%s next_dinode_index = %d\n", cur_name, next_dinode_index); */

			/* Check the results */
			if (next_dinode_index < 0) {
				/* The dinode with the current name does not exist */
				if ((i - 1) == pos_last) {
					/* This name should be a file name */
					if (mode == 0) {
						/* Mode is READ - error */
						panic("FILE DOES NOT EXIST.");
						return -1;

					} else {
						/* Mode is WRITE or APPEND - Need to create the target file */
						/*
						 * 1. Find & Occupy a free dinode
						 * 2. Create a directory entry
						 * 3. Update directory record
						 * 4. Return that dinode index
						 */

						/* 1. Find & Occupy a free dinode */
						next_dinode_index = get_dinode(ptr_descriptor);
						if (next_dinode_index < 0) {
							panic("NOT ENOUGH FREE DINODE\n");
						}
						/* 2. Create a directory entry */
						panic("TODO");

					}

				} else {
					/* The directory does not exist */
					panic("DIRECTORY DOES NOT EXIST");
					return -1;
				}
			}

			/* update i & dinode_index */
			i = j;
			dinode_index = next_dinode_index;

		} else {
			/* Should NOT get to there */
			panic("PATH IS INVALID - %s\n", path);
		}
	}

	/* File exists */
	return next_dinode_index;
}

/*
 # HDD operation (r/w) in bytes
 @ param : operation parameters
 */
void fslib_hdd_op(const struct fs_hdd_op *param)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hddp *payload;

	payload = (struct ipc_msg_payload_hddp *)msg.payload;

	/* Prepare message */
	msg.type = IS_TRUE(param->is_read) ? HDDP_MSG_READ : HDDP_MSG_WRITE;
	payload->dev_num = HDDP_DEV_NUM_GEN(
			param->fs_mbr_index,
			LOGICAL_CONVERT(param->fs_logical_index)
			);
	payload->is_reserved = FALSE;
	payload->base_low = UINT64_LOW(param->base);
	payload->base_high = UINT64_HIGH(param->base);
	payload->size = param->size;
	payload->buf_address = param->buf_address;
	comm_msg(DRV_PID_HDDP, &msg);

	/* Check result */
	if (HDDP_MSG_OK != msg.type) {
		if (param->is_read) {
			panic("FS - HDDP READ ERROR!\n");
		} else {
			panic("FS - HDDP WRITE ERROR!\n");
		}
	}
}

/* 
 # Read bytes on the disk
 * index         : block index w.r.t. fs partition descriptor
 * ptr_descriptor: file system partition descriptor
 * out_buf       : output buffer
 * size			 : size of buffer
 */
BOOL fslib_read_bytes(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	void *out_buf,
	uint32 size
)
{
	struct fs_hdd_op hdd_op_param;

	hdd_op_param.fs_mbr_index = ptr_descriptor->mbr_index;
	hdd_op_param.fs_logical_index = ptr_descriptor->logical_index;
	hdd_op_param.base = FS_BLOCK2BYTE(index);
	hdd_op_param.size = size;
	hdd_op_param.buf_address = out_buf;
	hdd_op_param.is_read = TRUE;
	fslib_hdd_op(&hdd_op_param);

	return TRUE;
}

/* 
 # Write bytes to the disk
 * index         : block index w.r.t. fs partition descriptor
 * ptr_descriptor: file system partition descriptor
 * in_buf        : input buffer
 * size			 : size of buffer
 */
BOOL fslib_write_bytes(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	const void *in_buf,
	uint32 size
)
{
	struct fs_hdd_op hdd_op_param;

	hdd_op_param.fs_mbr_index = ptr_descriptor->mbr_index;
	hdd_op_param.fs_logical_index = ptr_descriptor->logical_index;
	hdd_op_param.base = FS_BLOCK2BYTE(index);
	hdd_op_param.size = size;
	hdd_op_param.buf_address = (void *)in_buf;
	hdd_op_param.is_read = FALSE;
	fslib_hdd_op(&hdd_op_param);

	return TRUE;
}

/*
 # Read a block on the disk
 * index         : block index w.r.t. fs partition descriptor
 * ptr_descriptor: file system partition descriptor
 * out_block     : output block
 */
BOOL fslib_read_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	struct fs_data_block *out_block
)
{
	return fslib_read_bytes(
		index,
		ptr_descriptor,
		out_block,
		FS_BYTES_PER_BLOCK
	);
}

/*
 # Write a block to the disk
 * index         : block index w.r.t. fs partition descriptor
 * ptr_descriptor: file system partition descriptor
 * in_block      : input block
 */
BOOL fslib_write_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	const struct fs_data_block *in_block
)
{
	return fslib_write_bytes(
		index,
		ptr_descriptor,
		in_block,
		FS_BYTES_PER_BLOCK
	);
}

/*
 # Get a free datablock
 * ptr_descriptor: file system partition descriptor
 @ RETURN        : usable data block index, -1 when error
 */
int32 fslib_get_data_block(const struct fs_partition_descriptor *ptr_descriptor)
{
	return -1;
}

/*
 # Free a used datablock
 * index         : index of data block will not be used
 * ptr_descriptor: file system partition descriptor
 */
void fslib_put_data_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor
)
{
	;
}

/*
 # Read a datablock on the disk
 * index         : data block index w.r.t. fs partition descriptor
 * ptr_descriptor: file system partition descriptor
 * out_data_block: output data block
 */
BOOL fslib_read_data_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	struct fs_data_block *out_data_block
)
{
	return fslib_read_block(
		index + ptr_descriptor->sb.data_start,
		ptr_descriptor,
		out_data_block
	);
}

/*
 # Write a datablock to the disk
 * index         : block index w.r.t. fs partition descriptor
 * ptr_descriptor: file system partition descriptor
 * in_data_block : input data block
 */
BOOL fslib_write_data_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	const struct fs_data_block *in_data_block
)
{
	return fslib_write_block(
		index + ptr_descriptor->sb.data_start,
		ptr_descriptor,
		in_data_block
	);
}

/*
 # Find an empty bit in a bitmap block
 * bitmap_block: bitmap block
 * RETURN:       empty bit index, -1 when the bitmap is full
 */
int32 fslib_find_empty_bitmap_block(
	const struct fs_data_block *bitmap_block
)
{
	uint32 byte_idx, bit_idx;
	byte cur_byte;

	/* Find within 1 block */
	for (byte_idx = 0; byte_idx < FS_BYTES_PER_BLOCK; ++byte_idx)  {
		cur_byte = bitmap_block->data[byte_idx];
		/* Find within 1 byte */
		for (bit_idx = 0; bit_idx < 8; ++bit_idx) {
			if ((cur_byte & 0x1) == 0)
			{
				/* Empty bit found */
				return (byte_idx * 8 + bit_idx);
			}
			/* Not found -> shift to next bit */
			cur_byte = (byte)(cur_byte >> 1);
		}
	}

	/* All bits are used */
	return -1;
}

/*
 # Set a bit in a bitmap block
 * index: bit index
 * val  : value of bit to be set
 * bitmap_block: bitmap block
 */
void fslib_set_bitmap_block(
	uint32 index,
	BOOL val,
	struct fs_data_block *bitmap_block
)
{
	uint32 byte_idx, bit_idx;
	byte byte_in_block;
	byte mask;

	/* Check boundary */
	kassert(index < (FS_BYTES_PER_BLOCK * 8));

	/* Calculate */
	byte_idx = index / 8;
	bit_idx = index % 8;

	if (IS_TRUE(val)) {
		mask = (byte)(1 << bit_idx);
		/* Set to 1 */
		byte_in_block = bitmap_block->data[byte_idx];
		byte_in_block = (byte)(byte_in_block | mask);
		bitmap_block->data[byte_idx] = byte_in_block;
	} else {
		mask = (byte)(~(1 << bit_idx));
		/* Set to 0 */
		byte_in_block = bitmap_block->data[byte_idx];
		byte_in_block = (byte)(byte_in_block & mask);
		bitmap_block->data[byte_idx] = byte_in_block;
	}
}

/*
 # Build superblock by given partition info.
 * ptr_descriptor: file system partition descriptor
 */
void fslib_build_superblock(struct fs_partition_descriptor *ptr_descriptor)
{
    uint32 remained_block_cnt; /* Count of current remained free blocks */
	uint32 dinode_tot_cnt; /* Total count of all dinode */
	uint32 data_map_tot_cnt; /* Total count of data map blocks */

    /* Check minimum requirement of sectors */
	if (ptr_descriptor->sector_cnt < FS_MINIMUM_SECTORS)
		panic("FS - NO ENOUGH SPACE FOR MKFS!\n");
	/* Build Superblock - Basic */
	ptr_descriptor->sb.magic_num = SUPERBLOCK_MAGIC_NUM;
	ptr_descriptor->sb.size = ptr_descriptor->sector_cnt / FS_FACTOR_BS;
	ptr_descriptor->sb.log_start = LOGBLOCK_IDX;
	ptr_descriptor->sb.log_cnt = LOGBLOCK_CNT;

	remained_block_cnt = ptr_descriptor->sb.size;
	/* Build Superblock - DINODE Map */
	remained_block_cnt -= SUPERBLOCK_CNT + LOGBLOCK_CNT;
	dinode_tot_cnt = ((remained_block_cnt / FS_DINODE_FB_RATIO) +
			(FS_CNT_DINODE_PER_BLOCK - 1)) / FS_CNT_DINODE_PER_BLOCK *
			FS_CNT_DINODE_PER_BLOCK; /* Round up */
	ptr_descriptor->sb.dinode_tot_cnt = dinode_tot_cnt;
	ptr_descriptor->sb.dinode_map_start = LOGBLOCK_IDX + LOGBLOCK_CNT;
	ptr_descriptor->sb.dinode_map_cnt = (dinode_tot_cnt +
				(FS_CNT_BIT_PER_BLOCK - 1)) /
				FS_CNT_BIT_PER_BLOCK; /* Round up */

	/* Build Superblock - DINODE */
	remained_block_cnt -= ptr_descriptor->sb.dinode_map_cnt;
	ptr_descriptor->sb.dinode_start = ptr_descriptor->sb.dinode_map_start +
			ptr_descriptor->sb.dinode_map_cnt;
	ptr_descriptor->sb.dinode_cnt = dinode_tot_cnt / FS_CNT_DINODE_PER_BLOCK;

	/* Build Superblock - Data Map */
	remained_block_cnt -= ptr_descriptor->sb.dinode_cnt;
	kassert(remained_block_cnt <= 0xffffffff - FS_CNT_BIT_PER_BLOCK);
	data_map_tot_cnt = remained_block_cnt + FS_CNT_BIT_PER_BLOCK; /* Round Up */
	data_map_tot_cnt /= FS_CNT_BIT_PER_BLOCK + 1;
	ptr_descriptor->sb.data_map_start = ptr_descriptor->sb.dinode_start +
			ptr_descriptor->sb.dinode_cnt;
	ptr_descriptor->sb.data_map_cnt = data_map_tot_cnt;

	/* Build Superblock - Data */
	remained_block_cnt -= ptr_descriptor->sb.data_map_cnt;
	ptr_descriptor->sb.data_start = ptr_descriptor->sb.data_map_start +
			ptr_descriptor->sb.data_map_cnt;
	ptr_descriptor->sb.data_cnt = remained_block_cnt;
}

/*
 # Build 1st dinode on a full data block
 * buffer: a full data block with size of FS_BYTES_PER_BLOCK
 */
void fslib_build_1st_dinode(byte *ptr_buffer)
{
/*
	printf("size of buffer: %d\n", sizeof(ptr_buffer));
	printf("address of buffer: 0x%x\n", ptr_buffer);
*/
	struct dinode root;

	/* Clean buffer of data block & dinode */
	memset(ptr_buffer, 0, FS_BYTES_PER_BLOCK);

	/* Init. 1st dinode */
	init_dinode(&root);
	root.type = DINODE_DIRECTORY;
	root.link_cnt = 1;

	/* Write 1st dinode to the buffer */
	/* printf("size of dinode is: %d\n", sizeof(struct dinode)); */
	memcpy(ptr_buffer, &root, sizeof(struct dinode));
}

/*
 # Open a file
 * path          : full file path
 * mode          : open mode
 * src_pid       : caller's pid
 * ptr_descriptor: file system partition descriptor
 @ RETURN	     : file descriptor id in the caller's process; -1 when error
 */
int32 fslib_open_file(
	const char *path,
	FILELIB_OP_FLAG mode,
	uint32 src_pid,
	const struct fs_partition_descriptor *ptr_descriptor
)
{
	int32 dinode_index = parse_path(path, mode, ptr_descriptor);
}