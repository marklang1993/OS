#include "dbg.h"
#include "lib.h"
#include "drivers/hdd_part.h"

#define COPY_BUF(dst, src, size) \
	kassert(!ENABLE_SPLIT_KUSPACE); \
	memcpy((void *)(dst), (void *)(src), size)

#define PRE_DEV_USE \
	get_descriptor_ptr(param, &ptr_descriptor); \
	if (0 == ptr_descriptor->ref_cnt) \
		panic("HDDP - PARTITION %x NOT OPEN ERROR!\n", \
			param->dev_num)


/* Harddisk partition descriptor */
struct hdd_partition_descriptor
{
	BOOL is_valid; /* If not ture, others are invalid */
	uint32 ref_cnt;		/* Reference count */
	BOOL is_bootable;	/* Is partition bootable */
	uint32 type;		/* Partition type */
	uint32 base_sector;	/* Base usable sector index */
	uint32 last_sector;	/* Last sector index */
	uint32 cnt_sectors;	/* Count of usable sectors */
	uint32 rev_sectors;	/* Reserved sectors */
};

/* Harddisk MBR partition descriptor */
struct hdd_mbr_partition_descriptor
{
	struct hdd_partition_descriptor main;
	struct hdd_partition_descriptor logicals[PART_MAX_L_PER_EX_PART];
};

/* HDDP internal global variables */
static BOOL hddp_is_init = FALSE;
/* Harddisk partition table */
static struct hdd_mbr_partition_descriptor hdd_part_table[HDDP_MAX_MBR_P_CNT];


/*
 # Init. hdd partition descriptor
 @ ptr_descriptor       : pointer to hdd_partition_descriptor initialized
 @ ptr_last_descriptor  : pointer to last hdd_partition_descriptor initialized
 @ ptr_part_table_entry : raw partition table entry
 @ base_sector_offset   : base_sector_lba offset
 */
static void hddp_descriptor_init(
	struct hdd_partition_descriptor *const ptr_descriptor,
	const struct hdd_partition_descriptor *const ptr_last_descriptor,
	const struct partition_table_entry *const ptr_part_table_entry,
	const uint32 base_sector_offset
)
{
	/* MUST be a valid partition table entry */
	kassert(ptr_part_table_entry->type != 0);

	/* Init. */
	ptr_descriptor->is_valid = TRUE;
	ptr_descriptor->ref_cnt = 0;
	ptr_descriptor->is_bootable =
		(ptr_part_table_entry->is_bootable & PART_BOOTABLE) != 0 ?
		TRUE : FALSE;
	ptr_descriptor->type = ptr_part_table_entry->type;
	ptr_descriptor->base_sector = ptr_part_table_entry->base_sector_lba +
				base_sector_offset;
	ptr_descriptor->cnt_sectors = ptr_part_table_entry->cnt_sectors;
	ptr_descriptor->last_sector = ptr_descriptor->base_sector +
				ptr_descriptor->cnt_sectors - 1;
	/* Calculate reserved sectors of current partition */
	if (ptr_last_descriptor != NULL) {
		ptr_descriptor->rev_sectors = ptr_descriptor->base_sector -
					(ptr_last_descriptor->last_sector + 1);
	} else {
		ptr_descriptor->rev_sectors = ptr_descriptor->base_sector - 0;
	}

	/* Output partition descriptor information */
	printk("base: %d, end: %d, cnt: %d, rev_cnt: %d\n",
		ptr_descriptor->base_sector,
		ptr_descriptor->last_sector,
		ptr_descriptor->cnt_sectors,
		ptr_descriptor->rev_sectors);
}


/*
 # Get pointer of partition descriptor
 @ param         : hdd partition parameters
 @ pp_descriptor : pointer to pointer of partition descriptor
 */
static void get_descriptor_ptr(
	const struct ipc_msg_payload_hddp *param,
	struct hdd_partition_descriptor **const pp_descriptor
)
{
	uint32 hddp_mbr_index; /* hdd partition mbr index */
	uint32 hddp_logical_index; /* hdd partition logical index */
	struct hdd_partition_descriptor *ptr_descriptor;

	/* Get partition table index */
	hddp_mbr_index = HDDP_GET_MBR_NUM(param->dev_num);
	hddp_logical_index = HDDP_GET_LOGICAL_NUM(param->dev_num);

	/* Get hdd partition descriptor */
	if (0 == hddp_logical_index) {
		/* Not a logical partition */
		ptr_descriptor = &(hdd_part_table[hddp_mbr_index].main);
	} else {
		/* Logical partition */
		ptr_descriptor = &(hdd_part_table[hddp_mbr_index].
			logicals[hddp_logical_index - 1]);
	}

	/* Check hdd partition type */
	if (NOT(IS_TRUE(ptr_descriptor->is_valid)))
		panic("HDDP - INVALID PARTITION %x\n", param->dev_num);
	if (ptr_descriptor->type == PART_TYPE_EXTENDED)
		panic("HDDP - EXTENDED PARTITION IS NOT SUPPORTED!\n");

	/* Return descriptor pointer */
	*pp_descriptor = ptr_descriptor;
}


/*
 # Calculate HDD base address
 @ param    : hdd partition parameters
 @ ptr_base : pointer to result of calculated base address
 */
static void calculate_hdd_base(
	const struct ipc_msg_payload_hddp *param,
	uint64 *ptr_base
)
{
	uint64 base_addr; /* Opeartion start address */
	uint64 limit_addr; /* Upper limit on address */
	uint64 end_addr; /* Opeartion end address */
	struct hdd_partition_descriptor *ptr_descriptor;

	/* Get hdd descriptor and check */
	PRE_DEV_USE;

	/* Calculate base address & limit address */
	if (param->is_reserved) {
		/* Reserved sectors */
		base_addr = ((uint64)(ptr_descriptor->base_sector
			- ptr_descriptor->rev_sectors))
			* HDD_BYTES_PER_SECTOR;
	} else {
		/* Non-reserved sectors */
		base_addr = ((uint64)ptr_descriptor->base_sector)
			* HDD_BYTES_PER_SECTOR;
	}
	base_addr += param->base_low;
	base_addr += ((uint64)param->base_high) << 32;
	limit_addr = ((uint64)(ptr_descriptor->last_sector + 1))
			* HDD_BYTES_PER_SECTOR;

	/* Check the end address of operating */
	if (0 == param->size)
		panic("HDDP - OPEARTION SIZE IS 0!\n");
	end_addr = base_addr;
	end_addr += (param->size - 1);
	if (end_addr >= limit_addr)
		panic("HDDP - OPEARTION OUT OF BOUND!\n");

	/* Return base address */
	*ptr_base = base_addr;

	/* Output partition descriptor information */
	printk("base: %d, end: %d, cnt: %d, rev_cnt: %d\n",
		ptr_descriptor->base_sector,
		ptr_descriptor->last_sector,
		ptr_descriptor->cnt_sectors,
		ptr_descriptor->rev_sectors);
}


/*
 # HDD opeartion in bytes
 @ hdd_dev_num : hdd minor device number
 @ is_read     : is read opeartion
 @ pos         : base address
 @ size        : size of bytes read / write
 @ buf         : buffer address
 */
static rtc hdd_op(
	uint32 hdd_dev_num,
	BOOL is_read,
	uint64 pos,
	uint32 size,
	void *buf
)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *ptr_payload_hdd; /* HDD Driver Payload */

	/* Send READ message to HDD driver */
	msg.type = IS_TRUE(is_read) ? HDD_MSG_READ : HDD_MSG_WRITE;
	ptr_payload_hdd = (struct ipc_msg_payload_hdd *)msg.payload;
	ptr_payload_hdd->dev_num = hdd_dev_num;
	ptr_payload_hdd->base_low = (uint32)(pos & 0xffffffffull);
	ptr_payload_hdd->base_high = (uint32)((pos & 0xffffffff00000000ull)
					>> 32);
	ptr_payload_hdd->size = size;
	ptr_payload_hdd->buf_address = buf;

	comm_msg(DRV_PID_HDD, &msg);
	/* Check result */
	if (HDD_MSG_ERROR == msg.type) {
		if (is_read) {
			panic("HDDP - HDD READ ERROR!\n");
		} else {
			panic("HDDP - HDD WRITE ERROR!\n");
		}
	}

	return OK;
}


/*
 # HDDP_READ / HDDP_WRITE message handler
 @ param   : hdd partition opeartion parameters
 @ is_read : is read opeartion
 */
static void hddp_dev_op(
	const struct ipc_msg_payload_hddp *param,
	BOOL is_read
)
{
	uint64 hdd_base_address;
	uint32 hdd_dev_num; /* hdd minor device number */
	rtc ret;

	/* Determine: primary master HDD / primary slave HDD */
	hdd_dev_num = HDDP_GET_MBR_NUM(param->dev_num) / PART_MAX_PART_MBR;

	/* Calculate hdd base address */
	calculate_hdd_base(param, &hdd_base_address);

	/* Execute operation on HDD */
	ret = hdd_op(
		hdd_dev_num,
		is_read,
		hdd_base_address,
		param->size,
		param->buf_address
	);

	/* Check the result */
	if (ret != OK)
		panic("HDDP - DEVICE OPERATION FAILED!\n");
}


/*
 # Read partition table by hdd minor device number
 @ hdd_dev_num : hdd minor device number
 */
static void read_part_table(uint32 hdd_dev_num)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *ptr_payload_hdd; /* HDD Driver Payload */
	struct partition_table_entry main_part_table_buf[COUNT_M_PART_TABLE_ENTRY];
	struct partition_table_entry logic_part_table_buf[COUNT_L_PART_TABLE_ENTRY];
	struct hdd_partition_descriptor *ptr_last_mbr_part_descriptor;
	struct hdd_partition_descriptor *ptr_last_logical_part_descriptor;
	uint32 hdd_part_table_base; /* hdd_part_table base index */
	uint32 extended_part_sector_offset; /* extended partition table offset in sectors */
	uint64 extended_part_offset; /* extended partition table offset in bytes */
	rtc ret;
	uint32 i, j;

	/* Locate the base index of descriptor in HDD Partition Table */
	hdd_part_table_base = hdd_dev_num * PART_MAX_PART_MBR;
	/* Init. ptr_last_mbr_part_descriptor */
	ptr_last_mbr_part_descriptor = NULL;
	/* Send OPEN message to HDD driver */
	msg.type = HDD_MSG_OPEN;
	ptr_payload_hdd = (struct ipc_msg_payload_hdd *)msg.payload;
	ptr_payload_hdd->dev_num = hdd_dev_num;
	comm_msg(DRV_PID_HDD, &msg);

	/* Send IOCTL - PRINT_ID message to HDD driver */
	msg.type = HDD_MSG_IOCTL;
	ptr_payload_hdd = (struct ipc_msg_payload_hdd *)msg.payload;
	ptr_payload_hdd->ioctl_msg = HDD_IMSG_PRINT_ID;
	ptr_payload_hdd->dev_num = hdd_dev_num;
	comm_msg(DRV_PID_HDD, &msg);

	/* Read MBR partition table */
	ret = hdd_op(
		hdd_dev_num,
		TRUE,
		BASE_PARTITION_TABLE,
		PARTITION_TABLE_ENTRY_SIZE * COUNT_M_PART_TABLE_ENTRY,
		main_part_table_buf
		);

	/* Process MBR partition table */
	for (i = 0; i < COUNT_M_PART_TABLE_ENTRY; ++i) {
		/* Check current raw MBR partition table entry is empty */
		if (PART_TYPE_NULL == main_part_table_buf[i].type)
			continue;

		/* Process MBR partition table entry */
		hddp_descriptor_init(
			&hdd_part_table[i + hdd_part_table_base].main,
			ptr_last_mbr_part_descriptor,
			&main_part_table_buf[i],
			0
			);

		/* Check current MBR partition is EXTENDED partition */
		if (PART_TYPE_EXTENDED !=
			hdd_part_table[i + hdd_part_table_base].main.type) {
			/* Update pointer of last MBR partition descriptor */
			ptr_last_mbr_part_descriptor = &hdd_part_table
				[i + hdd_part_table_base].main;
			continue;
		}

		/* Continue to process logical partitions */
		extended_part_sector_offset =
			hdd_part_table[i + hdd_part_table_base]
			.main.base_sector;
		extended_part_offset = extended_part_sector_offset *
					HDD_BYTES_PER_SECTOR +
					BASE_PARTITION_TABLE;
		/* Read partition table entry of 1st logical partition */
		ret = hdd_op(
			hdd_dev_num,
			TRUE,
			extended_part_offset,
			PARTITION_TABLE_ENTRY_SIZE * COUNT_L_PART_TABLE_ENTRY,
			logic_part_table_buf
			);

		/* Check current raw logical partition table entry is empty */
		if (PART_TYPE_NULL == logic_part_table_buf[0].type) {
			/* Update pointer of last MBR partition descriptor */
			ptr_last_mbr_part_descriptor = &hdd_part_table
				[i + hdd_part_table_base].main;
			continue;
		}

		/* Process logical partition table entry */
		j = 0;
		hddp_descriptor_init(
			&hdd_part_table[i + hdd_part_table_base].logicals[j],
			ptr_last_mbr_part_descriptor,
			&logic_part_table_buf[0],
			extended_part_sector_offset
			);

		/* Check next logical partition table */
		while (PART_TYPE_EXTENDED == logic_part_table_buf[1].type) {
			ptr_last_logical_part_descriptor = &hdd_part_table
				[i + hdd_part_table_base].logicals[j];
			/* Read next logical partition table */
			ret = hdd_op(
				hdd_dev_num,
				TRUE,
				extended_part_offset
					+ logic_part_table_buf[1].base_sector_lba
					* HDD_BYTES_PER_SECTOR,
				PARTITION_TABLE_ENTRY_SIZE
					* COUNT_L_PART_TABLE_ENTRY,
				logic_part_table_buf);

			/* Check current raw partition table entry is empty */
			if (PART_TYPE_NULL == logic_part_table_buf[0].type)
				break;

			/* Process this partition table entry */
			++j;
			hddp_descriptor_init(
				&hdd_part_table[i + hdd_part_table_base]
					.logicals[j],
				ptr_last_logical_part_descriptor,
				&logic_part_table_buf[0],
				ptr_last_logical_part_descriptor->base_sector
					+ ptr_last_logical_part_descriptor
						->cnt_sectors
			);
		}
		/* Update pointer of last MBR partition descriptor */
		ptr_last_mbr_part_descriptor = &hdd_part_table
			[i + hdd_part_table_base].main;
	}
}


/*
 # HDDP_OPEN message handler
 @ param : hdd partition open parameters
 */
static void hddp_dev_open(const struct ipc_msg_payload_hddp *param)
{
	uint32 hdd_dev_num; /* hdd minor device number */
	uint32 hdd_part_table_base; /* hdd_part_table base index */
	uint32 cnt_valid; /* count of valid mbr partition table entries */
	struct hdd_partition_descriptor *ptr_descriptor;
	uint32 i;

	/* Determine: primary master HDD / primary slave HDD */
	hdd_dev_num = HDDP_GET_MBR_NUM(param->dev_num) / PART_MAX_PART_MBR;
	/* Locate the base index of entry in HDD Partition Table */
	hdd_part_table_base = hdd_dev_num * PART_MAX_PART_MBR;

	/* Check all mbr partitions of the corresponding HDD */
	cnt_valid = 0;
	for (i = 0; i < COUNT_M_PART_TABLE_ENTRY; ++i) {
		if (IS_TRUE(hdd_part_table[i + hdd_part_table_base]
			.main.is_valid)) {
			/* If this is a valid mbr partition */
			++cnt_valid;
		}
	}
	/* Determine is necessary to read partition table */
	if (0 == cnt_valid) {
		read_part_table(hdd_dev_num);
	}

	/* Check & Get specified partition is valid */
	/* NOTE: Checking is handled at the same time */
	get_descriptor_ptr(param, &ptr_descriptor);

	/* Temporarily not support open multiple times */
	if (ptr_descriptor->ref_cnt != 0)
		panic("HDDP - PARTITION %x REOPEN ERROR!\n", param->dev_num);

	/* Reference count increase */
	ptr_descriptor->ref_cnt += 1;
}


/*
 # HDDP_CLOSE message handler
 @ param : hdd partition close parameters
 */
static void hddp_dev_close(const struct ipc_msg_payload_hddp *param)
{
	struct hdd_partition_descriptor *ptr_descriptor;

	/* Get hdd descriptor and check */
	PRE_DEV_USE;

	/* Reference count increase */
	ptr_descriptor->ref_cnt -= 1;
}


/*
 # Get partition information
 @ param          : pointer to payload of returning information
 @ ptr_descriptor : pointer to partition descriptor
 */
static void hddp_ioctl_get_info(
	struct ipc_msg_payload_hddp_get_info *const param,
	const struct hdd_partition_descriptor *ptr_descriptor
)
{
	/* Copy parameters of this partition descriptor */
	param->is_bootable = ptr_descriptor->is_bootable;
	param->type = ptr_descriptor->type;
	param->cnt_sectors = ptr_descriptor->cnt_sectors;
	param->rev_sectors = ptr_descriptor->rev_sectors;
}


/*
 # HDDP_IOCTL message handler
 @ param : hdd partition ioctl parameters
 */
static void hddp_dev_ioctl(struct ipc_msg_payload_hddp *const param)
{
	struct hdd_partition_descriptor *ptr_descriptor;

	/* Get hdd descriptor and check */
	PRE_DEV_USE;

	/* Determine ioctl message type */
	switch (param->ioctl_msg) {

	case HDDP_IMSG_GET_INFO:
                /* Get partition information */
                hddp_ioctl_get_info(
			(struct ipc_msg_payload_hddp_get_info *)param,
			ptr_descriptor
		);
                break;

        default:
                panic("HDDP - IOCTL RECEIVED UNKNOWN MESSAGE!\n");
                break;
        }

}


/*
 # Initialize Harddisk Partition Driver (Ring 0)
 */
void hddp_init(void)
{
	/* Init. hdd partition table */
	memset(hdd_part_table, 0x0, sizeof(hdd_part_table));

	/* Set Init. flag */
	hddp_is_init = TRUE;
}


/*
 # Harddisk Partition Driver Message Dispatcher
 */
void hddp_message_dispatcher(void)
{
	rtc ret;
	struct proc_msg msg;
	struct ipc_msg_payload_hddp *ptr_payload;
	uint32 src;
	uint32 hddp_mbr_index; /* hdd partition mbr index */
	uint32 hddp_logical_index; /* hdd partition logical index */

	/* Check Init. flag */
	if (NOT(IS_TRUE(hddp_is_init)))
		panic("HDDP DRIVER IS NOT INITIALIZED!\n");

	while(1) {
		/* Receive message from other processes */
		recv_msg(IPC_PROC_ALL, &msg);
		src = msg.src;
		ptr_payload = (struct ipc_msg_payload_hddp *)msg.payload;
		printk("HDDP MSG TYPE: 0x%x\n", msg.type);

		/* Get partition table index */
		hddp_mbr_index = HDDP_GET_MBR_NUM(ptr_payload->dev_num);
		hddp_logical_index = HDDP_GET_LOGICAL_NUM(ptr_payload->dev_num);
		/* Validate */
		if (hddp_mbr_index >= HDDP_MAX_MBR_P_CNT)
			panic("HDDP - MBR INDEX OUT OF BOUND!\n");
		/* NOTE: Valid range of logical index is 0~16 */
		if (hddp_logical_index > PART_MAX_L_PER_EX_PART)
			panic("HDDP - LOGICAL INDEX OUT OF BOUND!\n");

		/* Check message type */
		switch(msg.type) {
		case HDDP_MSG_OPEN:
			hddp_dev_open(ptr_payload);
			break;

		case HDDP_MSG_WRITE:
			hddp_dev_op(ptr_payload, FALSE);
			break;

		case HDDP_MSG_READ:
			hddp_dev_op(ptr_payload, TRUE);
			break;

		case HDDP_MSG_CLOSE:
			hddp_dev_close(ptr_payload);
			break;

		case HDDP_MSG_IOCTL:
			hddp_dev_ioctl(ptr_payload);
			break;

		default:
			panic("HDDP RECEIVED UNKNOWN MESSAGE!\n");
		}

		/* Response message */
		msg.type = HDDP_MSG_OK;
		send_msg(src, &msg);
	}
}
