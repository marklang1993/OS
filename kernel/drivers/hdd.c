#include "dbg.h"
#include "io_port.h"
#include "lib.h"
#include "drivers/i8259a.h"
#include "drivers/hdd.h"

#define COPY_BUF(dst, src, size) \
	kassert(!ENABLE_SPLIT_KUSPACE); \
	memcpy((void *)(dst), (void *)(src), size)

#define PRE_DEV_USE \
	dev_num = param->dev_num; \
	ptr_descriptor = &hdd_descriptors[dev_num]; \
	if (ptr_descriptor->ref_cnt == 0) \
		panic("HDD %d DOES NOT OPEN!\n", dev_num)


/* ATA Channel */
#define IDE_CH_IS_PRIMARY		1	/* Only Primary IDE channel is supported */
#define IDE_CH_MASK			(IDE_CH_IS_PRIMARY << 7)

/* HDD Ports (WRITE_READ) */
#define PORT_HDD_DATA			(0x170 | IDE_CH_MASK)
#define PORT_HDD_FEATURE		(0x171 | IDE_CH_MASK)
#define PORT_HDD_ERROR			PORT_HDD_FEATURE
#define PORT_HDD_SECTOR_CNT		(0x172 | IDE_CH_MASK)
#define PORT_HDD_LBA_L			(0x173 | IDE_CH_MASK)
#define PORT_HDD_LBA_M			(0x174 | IDE_CH_MASK)
#define PORT_HDD_LBA_H			(0x175 | IDE_CH_MASK)
#define PORT_HDD_DEV			(0x176 | IDE_CH_MASK)
#define PORT_HDD_CMD			(0x177 | IDE_CH_MASK)
#define PORT_HDD_STATUS			PORT_HDD_CMD
#define PORT_HDD_DEV_CTRL		(0x376 | IDE_CH_MASK)
#define PORT_HDD_ALT_STATUS		PORT_HDD_DEV_CTRL

/* HDD BIOS Physical Address */
#define BIOS_HDD_CNT_DRV		0x475	/* Physical address for getting count of drivers */

/* HDD commands */
#define HDD_IDENTIFY			0xec
#define HDD_READ			0x20
#define HDD_WRITE			0x30

/* HDD Macros */
#define HDD_TIMEOUT			500	/* *10ms */


/* # HDD internal struct/union */

/* Harddisk sector operation parameters */
struct hdd_sector_param
{
        BOOL is_master;
        /* Base sector */
        uint32 pos;
        /* Count of sectors */
        uint32 count;
        /* Memory address of buffer in other process */
        void *buf_address;
};

/* ATA IDENTIFY struct (Incomplete) */
#pragma pack(push, 1)
struct ata_identify
{
	uint16 skip0;
	uint16 cnt_cylinders;
	uint16 skip1;
	uint16 cnt_headers;
	uint16 skip2[2];
	uint16 cnt_sectors;
	uint16 vendor_id_1[3];
	char serial_number[20];
	uint16 skip3[3];
	char firmware_rev[8];
	char model_number[40];
	char skip4;
	char vendor_id_2;
	uint16 skip5;
	struct {
		char skip0;
		uint16 is_DMA_sup:1;
		uint16 is_LBA_sup:1;
		uint16 skip1:6;
		uint16 skip2;
	} capabilities;
	uint16 skip6[9];
	uint32 cnt_addressable_sectors;
};
#pragma pack(pop)

/* HDD descriptor */
struct hdd_descriptor {
	uint32 ref_cnt; /* Reference count */
	struct ata_identify id_info;
};

/* 1. HDD Device Register */
union hdd_dev_reg
{
	uint8 data;		/* # Register data */

	struct
	{
		uint8 lba_haddr:4;	/* CHS: Header number
					 * LBA: LBA28 high address: bit 24~27
					 */
		uint8 drv_sel:1;	/* DRV selector: master OR slave */
		uint8 fix_1:1;
		uint8 is_lba:1;		/* LBA enable flag */
		uint8 fix_2:1;
	} b;
};
#define HDD_DEV_REG_GEN(is_lba, drv_sel, hs) \
	(0xa0 | ((is_lba & 0x1) << 6) | ((drv_sel & 0x1) << 4) | \
	((is_lba ? (hs >> 24) : hs) & 0xf))
/* NOTE: if LBA, "hs" is 28-bit LBA address! */

/* 2. HDD Status Register */
union hdd_status_reg
{
	uint8 data;		/* # Register data */

	struct
	{
		uint8 error:1;		/* Error bit */
		uint8 obsolete:2;	/* Obsolete bits */
		uint8 data_req:1;	/* Data Request - ready to transfer data */
		uint8 param:1;		/* Command dependent - formerly DSC bit */
		uint8 df_se:1;		/* Device fault / Stream error */
		uint8 dev_rdy:1;	/* Device ready */
		uint8 busy:1;		/* Busy. other bits are invalid if set */
	} b;
};

/* 3. HDD Device Control Register */
union hdd_dev_ctrl_reg
{
	uint8 data;		/* # Register data */

	struct
	{
		uint8 zero:1;		/* Always 0 */
		uint8 int_en:1;		/* Interrupt enable */
		uint8 sw_rst:1;		/* Software reset */
		uint8 invalid:4;	/* Invalid bits */
		uint8 hob:1;		/* High order byte - defined by 48-bit address feature set */
	} b;
};

/* #. HDD All Control Registers (WRITE) Struct
 * NOTE: this does not include HDD_DATA
 */
struct hdd_ctrl_regs
{
	union hdd_dev_ctrl_reg dev_ctrl;
	uint8 feature;
	uint8 sector_cnt;
	uint8 lba_l;
	uint8 lba_m;
	uint8 lba_h;
	union hdd_dev_reg dev;
	union hdd_status_reg cmd;
};


/* HDD internal global variables */
static BOOL hdd_is_init = FALSE;
static uint8 hdd_status;
static struct hdd_descriptor hdd_descriptors[HDD_MAX_DRIVES];

/*
 # Processing ata identity string
 @ str: raw string
 @ len: length of raw string
 */
static void ata_id_str_process(char *str, uint32 len)
{
	uint32 i, j;
	uint32 cnt_pairs = len / 2;
	char tmp;

	/* Swap high byte and low byte within a word */
	for (i = 0; i < cnt_pairs; ++i)
	{
		j = i * 2;
		tmp = str[j];
		str[j] = str[j + 1];
		str[j + 1] = tmp;
	}

	/* Add end mark of a c-style string */
	str[len - 1] = '\0';
}


/*
 # HDD wait for device ready
 */
static rtc hdd_wait_busy(void)
{
	union hdd_status_reg status;

	/* use volatile here? */
	while (1) {
		io_in_byte(PORT_HDD_STATUS, &status.data);
		if (!status.b.busy) {
			return OK;
		}
	}

	return ETIMEOUT;
}


/*
 # HDD send command
 @ ptr_hdd_ctrl: pointer to a hdd_ctrl_regs struct
 */
static void hdd_send_cmd(const struct hdd_ctrl_regs *ptr_hdd_ctrl)
{
	if (hdd_wait_busy() != OK)
		panic("HDD TIME OUT!\n");

	/* Write hdd device control register */
	io_out_byte(PORT_HDD_DEV_CTRL, ptr_hdd_ctrl->dev_ctrl.data);
	/* Write hdd parameters */	
	io_out_byte(PORT_HDD_FEATURE, ptr_hdd_ctrl->feature);
	io_out_byte(PORT_HDD_SECTOR_CNT, ptr_hdd_ctrl->sector_cnt);
	io_out_byte(PORT_HDD_LBA_L, ptr_hdd_ctrl->lba_l);
	io_out_byte(PORT_HDD_LBA_M, ptr_hdd_ctrl->lba_m);
	io_out_byte(PORT_HDD_LBA_H, ptr_hdd_ctrl->lba_h);
	io_out_byte(PORT_HDD_DEV, ptr_hdd_ctrl->dev.data);
	/* Write hdd command */	
	io_out_byte(PORT_HDD_CMD, ptr_hdd_ctrl->cmd.data);
}


/*
 # HDD_OPEN message handler
 @ param   : HDD_OPEN ipc message payload
 */
static void hdd_dev_open(const struct ipc_msg_payload_hdd *param)
{
	uint32 dev_num;
	struct hdd_descriptor *ptr_descriptor;
	struct hdd_ctrl_regs ctrl_regs;
	uint16 buf[HDD_BYTES_PER_SECTOR / 2];	/* Store raw identify data */

	/* Get hdd descriptor and check */
	dev_num = param->dev_num;
	ptr_descriptor = &hdd_descriptors[dev_num];
	if (ptr_descriptor->ref_cnt != 0)
		panic("HDD %d REOPEN ERROR!\n", dev_num);

	/* Prepare the command */
	memset(&ctrl_regs, 0, sizeof(struct hdd_ctrl_regs));
	ctrl_regs.dev_ctrl.data = 0; /* Activate */
	ctrl_regs.dev.data = HDD_DEV_REG_GEN(
					0,
					param->dev_num == HDD_DEV_PM ?
						HDD_DRV_MASTER : HDD_DRV_SLAVE,
					0
					);
	ctrl_regs.cmd.data = HDD_IDENTIFY;
	/* Send command & wait for hdd interrupt */
	hdd_send_cmd(&ctrl_regs);
	wait_int();

	/* After hdd interrupt, read data */
	io_bulk_in_word(PORT_HDD_DATA, buf, (sizeof(buf) / sizeof(uint16)));

	/* Copy and format data - discard redundant parts */
	memcpy(
		&ptr_descriptor->id_info,
		buf,
		sizeof(struct ata_identify)
		);

	/* Print data */
	ata_id_str_process(
		ptr_descriptor->id_info.serial_number,
		sizeof(ptr_descriptor->id_info.serial_number));
	ata_id_str_process(ptr_descriptor->id_info.model_number,
		sizeof(ptr_descriptor->id_info.model_number));

	/* Increase reference count */
	ptr_descriptor->ref_cnt += 1;
}


/*
 # HDD_CLOSE message handler
 @ param   : HDD_CLOSE ipc message payload
 */
static void hdd_dev_close(const struct ipc_msg_payload_hdd *param)
{
	uint32 dev_num;
	struct hdd_descriptor *ptr_descriptor;

	/* Get hdd descriptor and check */
	PRE_DEV_USE;

	/* Decrease reference count */
	ptr_descriptor->ref_cnt -= 1;
}


/*
 # Ioctl call handler - Print ATA identity infomation
 @ ptr_descriptor: pointer to hdd descriptor
 */
static void hdd_ioctl_print_idinfo(const struct hdd_descriptor *ptr_descriptor)
{
	char str_yes[] = "YES";
	char str_no[] = "NO";
	uint32 capacity;

	/* Print out ata identity infomation */
	printk("HDD S/N: %s\n", ptr_descriptor->id_info.serial_number);

	printk("C/H/S: %u/%u/%u\n",
		ptr_descriptor->id_info.cnt_cylinders,
		ptr_descriptor->id_info.cnt_headers,
		ptr_descriptor->id_info.cnt_sectors);
	capacity = ptr_descriptor->id_info.cnt_addressable_sectors
			* HDD_BYTES_PER_SECTOR / 1000000;
	printk("Capacity: %u MB\n", capacity);

	printk("DMA: %s\t",
		ptr_descriptor->id_info.capabilities.is_DMA_sup == 1 ?
		str_yes : str_no);

	printk("LBA: %s\n",
		ptr_descriptor->id_info.capabilities.is_LBA_sup == 1 ?
		str_yes : str_no);
}

/*
 # HDD_IOCTL message handler
 @ param   : HDD_IOCTL ipc message payload
 */
static void hdd_dev_ioctl(struct ipc_msg_payload_hdd *const param)
{
	uint32 dev_num;
	struct hdd_descriptor *ptr_descriptor;

	/* Get hdd descriptor and check */
	PRE_DEV_USE;

	/* Determine ioctl message type */
	switch (param->ioctl_msg) {

	case HDD_IMSG_PRINT_ID:
		/* Print ATA indentity information */
		hdd_ioctl_print_idinfo(ptr_descriptor);
		break;

	default:
		panic("HDD IOCTL RECEIVED UNKNOWN MESSAGE!\n");
		break;
	}
}


/*
 # Harddisk sector read / write
 @ param   : sector read / write parameters
 @ is_read : is HDD_READ flag
 */
static void hdd_dev_sector_op(const struct hdd_sector_param *param, BOOL is_read)
{
	struct hdd_ctrl_regs ctrl_regs;
	union hdd_status_reg status;
	uint16 buf[HDD_BYTES_PER_SECTOR / 2]; /* Store raw data read from OR write to hd */
	uint8 *p_buf; /* Byte pointer to buffer */
	uint32 base_sector, cnt_sectors;
	uint32 sectors_left;
	uint8 *p_ext; /* Pointer to external memory */

	/* Validate */
	kassert(param->buf_address != NULL);
	kassert(param->count != 0);
	kassert(param->pos + param->count <= HDD_LBA28_MAX);

	base_sector = param->pos;
	cnt_sectors = param->count;

	/* Read */
	p_buf = (uint8 *)buf;
	p_ext = (uint8 *)param->buf_address;
	while(cnt_sectors != 0) {

		/* Prepare the command */
		memset(&ctrl_regs, 0, sizeof(struct hdd_ctrl_regs));
		ctrl_regs.lba_l = (uint8)(base_sector & 0xff);
		ctrl_regs.lba_m = (uint8)((base_sector & 0xff00) >> 8);
		ctrl_regs.lba_h = (uint8)((base_sector & 0xff0000) >> 16);
		ctrl_regs.dev.data = HDD_DEV_REG_GEN(1,
			IS_TRUE(param->is_master) ? HDD_DRV_MASTER : HDD_DRV_SLAVE,
			base_sector);
		if (cnt_sectors < 0xff) {
			sectors_left = cnt_sectors;
			ctrl_regs.sector_cnt = cnt_sectors;
			base_sector += cnt_sectors;
			cnt_sectors = 0;
		} else {
			sectors_left = 0xff;
			ctrl_regs.sector_cnt = 0xff;
			base_sector += 0xff;
			cnt_sectors -= 0xff;
		}
		/* Prepare for register command */
		ctrl_regs.cmd.data = IS_TRUE(is_read) ? HDD_READ : HDD_WRITE;

		/* Send command & wait for hdd interrupt */
		hdd_send_cmd(&ctrl_regs);

		while(sectors_left != 0) {
			if (IS_TRUE(is_read)) {
				wait_int();
				/* Read Data */
				io_bulk_in_word(
					PORT_HDD_DATA,
					buf,
					(sizeof(buf) / sizeof(uint16))
				);

				/* Copy data to other process */
				COPY_BUF(p_ext, p_buf, HDD_BYTES_PER_SECTOR);

			} else {
				/* Check status */
				io_in_byte(PORT_HDD_STATUS, &status.data);
				if (0 == status.b.data_req)
					panic("HDD DATA_REQ CLEAR!\n");

				/* Copy data from other process */
				COPY_BUF(p_buf, p_ext, HDD_BYTES_PER_SECTOR);
				/* Write Data */
				io_bulk_out_word(
					PORT_HDD_DATA,
					buf,
					(sizeof(buf) / sizeof(uint16))
				);
				wait_int();
			}
			/* Update position */
			p_ext += HDD_BYTES_PER_SECTOR;
			--sectors_left;
		}
	}
}


/*
 # HDD_READ & HDD_WRITE message handler
 @ param   : HDD_READ / HDD_WRITE ipc message payload
 @ is_read : is HDD_READ flag
 */
static void hdd_dev_data_op(const struct ipc_msg_payload_hdd *param, BOOL is_read)
{
	/* HDD device descriptor */
	uint32 dev_num;
	struct hdd_descriptor *ptr_descriptor;
	/* Bytes related position */
	uint64 base_pos, limit_pos;
	uint32 start_pos, cnt_bytes_left;
	/* Sectors related position */
	uint32 base_sector, cnt_sectors;
	/* Buffer related */
	uint16 buf[HDD_BYTES_PER_SECTOR / 2]; /* Store raw data read from OR write to hd */
	uint8 *p_buf; /* Byte pointer to buffer */
	/* Sector opeation parameter */
	struct hdd_sector_param sector_param;
	/* Target memory pointer */
	uint8 *p_target;

	/* Get hdd descriptor and check */
	PRE_DEV_USE;

	/* Set "is_master" in sector_param */
	sector_param.is_master = dev_num == HDD_DEV_PM ? TRUE : FALSE;
	/* Validate parameters */
	kassert(param->buf_address != NULL);
	kassert(param->size != 0);

	/* Calculate base & end position */
	base_pos = param->base_high;
	base_pos = base_pos << 32;
	base_pos += param->base_low;
	kassert(base_pos < HDD_LBA28_BYTE_MAX); /* Check for LBA28 maximum byte position */
	limit_pos = base_pos + param->size;
	kassert(base_pos < limit_pos); /* Check for overflow in addition of uint64 */
	kassert(limit_pos < HDD_LBA28_BYTE_MAX); /* Check for LBA28 maximum byte position */

	/* Calculate start & end offsets */
	start_pos = (uint32)(base_pos % HDD_BYTES_PER_SECTOR);
	cnt_bytes_left = (uint32)(limit_pos % HDD_BYTES_PER_SECTOR);

	/* Calculate base sector position & count of sectors */
	base_sector = (uint32)(base_pos / HDD_BYTES_PER_SECTOR);
	cnt_sectors = (uint32)(limit_pos / HDD_BYTES_PER_SECTOR) - base_sector;
	cnt_sectors += cnt_bytes_left > 0 ? 1 : 0; /* Check reading 1 more sector is needed */

	kassert(cnt_sectors != 0);
	p_buf = (uint8 *)buf;
	p_target = param->buf_address;

	if (1 == cnt_sectors) {
		/* Prepare for reading 1 sector */
		sector_param.pos = base_sector;
		sector_param.count = 1;
		sector_param.buf_address = p_buf;
		/* Read */
		hdd_dev_sector_op(&sector_param, TRUE);

		/* Determine READ or WRITE */
		if (IS_TRUE(is_read)) {
			/* READ - Return data */
			if (0 == cnt_bytes_left) {
				/* Aligned in the end */
				COPY_BUF(p_target,
					p_buf + start_pos,
					HDD_BYTES_PER_SECTOR - start_pos);

			} else {
				/* Not aligned in the end */
				COPY_BUF(p_target,
					p_buf + start_pos,
					cnt_bytes_left - start_pos);
			}

		} else {
			/* WRITE - Directly overwrite on the buffer */
			if (0 == cnt_bytes_left) {
				/* Aligned in the end */
				COPY_BUF(p_buf + start_pos,
					p_target,
					HDD_BYTES_PER_SECTOR - start_pos);

			} else {
				/* Not aligned in the end */
				COPY_BUF(p_buf + start_pos,
					p_target,
					cnt_bytes_left - start_pos);
			}
			/* Write back */
			hdd_dev_sector_op(&sector_param, FALSE);		
		}

	} else {
		/* 1. Prepare for operation on the 1st sector */
		sector_param.pos = base_sector;
		sector_param.count = 1;
		sector_param.buf_address = p_buf;
		/* Read */
		hdd_dev_sector_op(&sector_param, TRUE);

		/* Guarantee aligned in the end of 1st sector */
		/* Determine READ or WRITE */
		if (IS_TRUE(is_read)) {
			/* READ - Return data */
			COPY_BUF(p_target,
				p_buf + start_pos,
				HDD_BYTES_PER_SECTOR - start_pos);
		} else {
			/* WRITE - Directly overwrite on the buffer */
			COPY_BUF(p_buf + start_pos,
				p_target,
				HDD_BYTES_PER_SECTOR - start_pos);
			/* Write back */
			hdd_dev_sector_op(&sector_param, FALSE);
		}

		/* Update postion */
		++base_sector;
		--cnt_sectors;
		kassert(!ENABLE_SPLIT_KUSPACE);
		p_target += HDD_BYTES_PER_SECTOR - start_pos;

		/* 2. Prepare for operation on remained sectors, other than last sector */
		if (cnt_sectors > 1) {
			sector_param.pos = base_sector;
			sector_param.count = cnt_sectors - 1;
			kassert(!ENABLE_SPLIT_KUSPACE);
			sector_param.buf_address = p_target;
			/* READ or WRITE */
			hdd_dev_sector_op(&sector_param, is_read);
			/* Update position */
			base_sector += cnt_sectors - 1;
			kassert(!ENABLE_SPLIT_KUSPACE);
			kassert(
				(uint64)(cnt_sectors - 1)
				* (uint64)HDD_BYTES_PER_SECTOR
				+ (uint32)p_target
				< 0xffffffffull
				);
			p_target += (cnt_sectors - 1) * HDD_BYTES_PER_SECTOR;
			cnt_sectors = 1;
		}

		/* 3. Prepare for operation on the last sector */
		kassert(cnt_sectors == 1);
		sector_param.pos = base_sector;
		sector_param.count = 1;
		sector_param.buf_address = p_buf;
		/* Read */
		hdd_dev_sector_op(&sector_param, TRUE);
		/* Determine READ or WRITE */
		if (IS_TRUE(is_read)) {
			/* READ - Return data */
			if (0 == cnt_bytes_left) {
                                /* Copy an entire sector */
				COPY_BUF(p_target,
					p_buf,
					HDD_BYTES_PER_SECTOR);

			} else {
				/* Copy the remained bytes */
				COPY_BUF(p_target,
					p_buf,
					cnt_bytes_left);
			}

		} else {
			/* WRITE - Directly overwrite on the buffer */
			if (0 == cnt_bytes_left) {
                                /* Copy an entire sector */
				COPY_BUF(p_buf,
					p_target,
					HDD_BYTES_PER_SECTOR);

			} else {
				/* Copy the remained bytes */
				COPY_BUF(p_buf,
					p_target,
					cnt_bytes_left);
			}
			/* Write back */
			hdd_dev_sector_op(&sector_param, FALSE);
		}
	}
}


/*
 # Initialize Harddisk Driver (Ring 0)
 */
void hdd_init(void)
{
	uint8 cnt_hdd;

	/* Get count of harddisk drive */
	cnt_hdd = *((uint8 *)BIOS_HDD_CNT_DRV);

	if (0 == cnt_hdd) {
		panic("NO HARDDISK DRIVE FOUND!\n");
	} else if (cnt_hdd > HDD_MAX_DRIVES) {
		panic("MORE THAN %d HARDDISK DRIVES!\n", cnt_hdd);
	}

	/* Init. hdd descriptor */
	memset(hdd_descriptors, 0x0, sizeof(hdd_descriptors));

	/* Init. harddisk interrupt */
	i8259a_set_handler(INDEX_8259A_HDD, hdd_interrupt_handler);
	i8259a_int_enable(INDEX_8259A_HDD);
	i8259a_int_enable(INDEX_8259A_SLAVE);	/* IMPORTANT! */

	/* Set Init. flag */
	hdd_is_init = TRUE;
}


/*
 # Harddisk Interrupt Handler (Ring 0)
 */
void hdd_interrupt_handler(void)
{
	/* Read HDD status */
	io_in_byte(PORT_HDD_STATUS, &hdd_status);

	/* Resume HDD driver function */
	resume_int(DRV_PID_HDD);
}


/*
 # Harddisk Driver Message Dispatcher
 */
void hdd_message_dispatcher(void)
{
	rtc ret;
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *ptr_payload;
	uint32 src;

	/* Check Init. flag */
	if (NOT(IS_TRUE(hdd_is_init)))
		panic("HDD DRIVER IS NOT INITIALIZED!\n");

	while(1) {
		/* Receive message from other processes */
		recv_msg(IPC_PROC_ALL, &msg);
		src = msg.src;
		ptr_payload = (struct ipc_msg_payload_hdd *)msg.payload;

		/* Check is the device number supported */
		if (ptr_payload->dev_num >= HDD_MAX_DRIVES)
			panic("UNSUPPORTED HDD DEVICE NUMBER!\n");

		/* Check message type */
		switch(msg.type) {
		case HDD_MSG_OPEN:
			hdd_dev_open(ptr_payload);
			break;

		case HDD_MSG_WRITE:
			hdd_dev_data_op(ptr_payload, FALSE);
			break;

		case HDD_MSG_READ:
			hdd_dev_data_op(ptr_payload, TRUE);
			break;

		case HDD_MSG_CLOSE:
			hdd_dev_close(ptr_payload);
			break;

		case HDD_MSG_IOCTL:
			hdd_dev_ioctl(ptr_payload);
			break;

		default:
			panic("HDD RECEIVED UNKNOWN MESSAGE!\n");
		}

		/* Response message */
		msg.type = HDD_MSG_OK;
		send_msg(src, &msg);
	}
}
