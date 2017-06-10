#include "dbg.h"
#include "lib.h"
#include "drivers/hdd_part.h"

#define COPY_BUF(dst, src, size) \
        kassert(!ENABLE_SPLIT_KUSPACE); \
        memcpy((void *)(dst), (void *)(src), size)


/* Harddisk partition descriptor */
struct hdd_partition_descriptor
{
	BOOL is_valid; /* If not ture, others are invalid */
	BOOL is_open;
	BOOL is_bootable;
	uint32 type;
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
	struct hdd_partition_descriptor *ptr_descriptor,
	const struct hdd_partition_descriptor const *ptr_last_descriptor,
	const struct partition_table_entry const *ptr_part_table_entry,
	const uint32 base_sector_offset
)
{
	/* MUST be a valid partition table entry */
	kassert(ptr_part_table_entry->type != 0);

	/* Init. */
	ptr_descriptor->is_valid = TRUE;
	ptr_descriptor->is_open = FALSE;
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
/*	printk("base: %d, end: %d, cnt: %d, rev_cnt: %d\n",
		ptr_descriptor->base_sector,
		ptr_descriptor->last_sector,
		ptr_descriptor->cnt_sectors,
		ptr_descriptor->rev_sectors);
*/
}


/*
 # Calculate HDD base address
 @ param    : hdd partition parameters
 @ ptr_base : pointer to result of calculated base address
 */
static void calculate_hdd_base(
	const struct ipc_msg_payload_hdd_part *param,
	uint64 *ptr_base
)
{
	uint32 hddp_mbr_index; /* hdd partition mbr index */
	uint32 hddp_logical_index; /* hdd partition logical index */
	uint64 base_addr; /* Opeartion start address */
	uint64 limit_addr; /* Upper limit on address */
	uint64 end_addr; /* Opeartion end address */
	const struct hdd_partition_descriptor *ptr_descriptor;

	/* Get partition table index */
	hddp_mbr_index = HDDP_GET_MBR_NUM(param->dev_num);
	hddp_logical_index = HDDP_GET_LOGICAL_NUM(param->dev_num);

	/* Validate */
	if (hddp_mbr_index >= HDDP_MAX_MBR_P_CNT)
		panic("HDDP: MBR INDEX OUT OF BOUND!");
	if (hddp_logical_index > PART_MAX_L_PER_EX_PART) /* valid range of logical index is 0~16 */
		panic("HDDP: LOGICAL INDEX OUT OF BOUND!");

	/* Get hdd partition descriptor */
	if (0 == hddp_logical_index) {
		/* Not a logical partition */
		ptr_descriptor = &(hdd_part_table[hddp_mbr_index].main);
	} else {
		/* Logical partition */
		ptr_descriptor = &(hdd_part_table[hddp_mbr_index].logicals[hddp_logical_index]);
	}

	/* Check hdd partition type */
	if (ptr_descriptor->type == PART_TYPE_NULL)
		panic("HDDP: NULL PARTITION!");
	if (ptr_descriptor->type == PART_TYPE_EXTENDED)
		panic("HDDP: EXTENDED PARTITION IS NOT SUPPORTED!");

	/* Calculate base address & limit address */
	if (param->is_reserved) {
		/* Reserved sectors */
		base_addr = ((uint64)(ptr_descriptor->base_sector
			- ptr_descriptor->rev_sectors)) * HDD_BYTES_PER_SECTOR;
	} else {
		/* Non-reserved sectors */
		base_addr = ((uint64)ptr_descriptor->base_sector) * HDD_BYTES_PER_SECTOR;
	}
	base_addr += param->base_low;
	base_addr += ((uint64)param->base_high) << 32;
	limit_addr = ((uint64)(ptr_descriptor->last_sector + 1)) * HDD_BYTES_PER_SECTOR;

	/* Check the end address of operating */
	if (0 == param->size)
		panic("HDDP: OPEARTION SIZE IS 0!");
	end_addr = base_addr;
	end_addr += (param->size - 1);
	if (end_addr >= limit_addr)
		panic("HDDP: OPEARTION OUT OF BOUND!");

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

	/* Check */
	if (hdd_dev_num > HDD_DEV_PS)
		panic("HDDP: SECONDARY IDE IS NOT SUPPORTED!");

	/* Send READ message to HDD driver */
	msg.type = IS_TRUE(is_read) ? HDD_MSG_READ : HDD_MSG_WRITE;
	ptr_payload_hdd = (struct ipc_msg_payload_hdd *)msg.payload;
	ptr_payload_hdd->dev_num = hdd_dev_num;
	ptr_payload_hdd->base_low = (uint32)(pos & 0xffffffffull);
	ptr_payload_hdd->base_high = (uint32)((pos & 0xffffffff00000000ull) >> 32);
	ptr_payload_hdd->size = size;
	ptr_payload_hdd->buf_address = buf;

	comm_msg(DRV_PID_HDD, &msg);
	/* Check result */
	if (HDD_MSG_ERROR == msg.type) {
		if (is_read) {
			panic("HDDP: HDD READ ERROR!");
		} else {
			panic("HDDP: HDD WRITE ERROR!");
		}
	}

	return OK;
}


/*
 # HDDP_READ / HDDP_WRITE message handler
 @ param   : hdd partition opeartion parameters
 @ is_read : is read opeartion
 */
static void hddp_dev_op(const struct ipc_msg_payload_hdd_part *param, BOOL is_read)
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
		panic("HDDP: DEVICE OPERATION FAILED!");
}


/*
 # HDDP_OPEN message handler
 @ param : hdd partition open parameters
 */
static void hddp_dev_open(const struct ipc_msg_payload_hdd_part *param)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *ptr_payload_hdd; /* HDD Driver Payload */
	struct partition_table_entry main_part_table_buf[COUNT_M_PART_TABLE_ENTRY];
	struct partition_table_entry logic_part_table_buf[COUNT_L_PART_TABLE_ENTRY];
	struct hdd_partition_descriptor *ptr_last_mbr_part_descriptor;
	struct hdd_partition_descriptor *ptr_last_logical_part_descriptor;
	uint32 hdd_dev_num; /* hdd minor device number */
	uint32 hdd_part_table_base; /* hdd_part_table base index */
	uint32 extended_part_sector_offset; /* extended partition table offset in sectors */
	uint64 extended_part_offset; /* extended partition table offset in bytes */
	rtc ret;
	uint32 i, j;

	/* Determine: primary master HDD / primary slave HDD */
	hdd_dev_num = HDDP_GET_MBR_NUM(param->dev_num) / PART_MAX_PART_MBR;
        /* Locate the base index of entry in HDD Partition Table */
	hdd_part_table_base = hdd_dev_num * PART_MAX_PART_MBR;
        /* Init. ptr_last_mbr_part_descriptor */
	ptr_last_mbr_part_descriptor = NULL;
	/* Send OPEN message to HDD driver */
	msg.type = HDD_MSG_OPEN;
	ptr_payload_hdd = (struct ipc_msg_payload_hdd *)msg.payload;
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
		if (0 == main_part_table_buf[i].type)
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
			ptr_last_mbr_part_descriptor = &hdd_part_table[i + hdd_part_table_base].main;
			continue;
		}

		/* Continue to process logical partitions */
		extended_part_sector_offset =
			hdd_part_table[i + hdd_part_table_base].main.base_sector;
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
		if (0 == logic_part_table_buf[0].type) {
			/* Update pointer of last MBR partition descriptor */
			ptr_last_mbr_part_descriptor = &hdd_part_table[i + hdd_part_table_base].main;
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
			ptr_last_logical_part_descriptor = &hdd_part_table[i + hdd_part_table_base].logicals[j];
			/* Read next logical partition table */
			ret = hdd_op(
				hdd_dev_num,
				TRUE,
				extended_part_offset
				+ logic_part_table_buf[1].base_sector_lba
				* HDD_BYTES_PER_SECTOR,
				PARTITION_TABLE_ENTRY_SIZE * COUNT_L_PART_TABLE_ENTRY,
				logic_part_table_buf);

			/* Check current raw partition table entry is empty */
			if (0 == logic_part_table_buf[0].type)
				break;

			/* Process this partition table entry */
			++j;
			hddp_descriptor_init(
				&hdd_part_table[i + hdd_part_table_base].logicals[j],
				ptr_last_logical_part_descriptor,
				&logic_part_table_buf[0],
				ptr_last_logical_part_descriptor->base_sector
					+ ptr_last_logical_part_descriptor->cnt_sectors
			);
		}
		/* Update pointer of last MBR partition descriptor */
		ptr_last_mbr_part_descriptor = &hdd_part_table[i + hdd_part_table_base].main;
	}
}


/*
 # Initialize Harddisk Partition Driver (Ring 0)
 */
void hddp_init(void)
{
	memset(hdd_part_table, 0x0, sizeof(hdd_part_table));
}


/*
 # Harddisk Partition Driver Message Dispatcher
 */
void hddp_message_dispatcher(void)
{
	rtc ret;
	struct proc_msg msg;
	uint32 src;

	while(1) {
		/* Receive message from other processes */
		recv_msg(IPC_PROC_ALL, &msg);
		src = msg.src;
		printk("HDDP MSG TYPE: 0x%x\n", msg.type);

		/* Check message type */
		switch(msg.type) {
		case HDDP_MSG_OPEN:
			hddp_dev_open((const struct ipc_msg_payload_hdd_part *)msg.payload);
			break;

		case HDDP_MSG_WRITE:
			hddp_dev_op((const struct ipc_msg_payload_hdd_part *)msg.payload, FALSE);
			break;

		case HDDP_MSG_READ:
			hddp_dev_op((const struct ipc_msg_payload_hdd_part *)msg.payload, TRUE);
			break;

		case HDDP_MSG_CLOSE:
			break;

		default:
			panic("HDDP RECEIVED UNKNOWN MESSAGE!\n");
		}

		/* Response message */
		msg.type = HDDP_MSG_OK;
		send_msg(src, &msg);
	}
}
