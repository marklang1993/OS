#include "lib.h"
#include "part_table.h"
#include "drivers/fs.h"
#include "drivers/hdd_part.h"

/* File System Driver Message Dispatcher */
void fs_message_dispatcher(void)
{
/*
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *payload;
	uint8 data[0xa00];
	uint32 i;

        msg.type = HDD_MSG_OPEN;
	payload = (struct ipc_msg_payload_hdd *)msg.payload;
	payload->dev_num = HDD_DEV_PM;
        comm_msg(DRV_PID_HDD, &msg);

        msg.type = HDD_MSG_OPEN;
	payload = (struct ipc_msg_payload_hdd *)msg.payload;
	payload->dev_num = HDD_DEV_PS;
        comm_msg(DRV_PID_HDD, &msg);

	msg.type = HDD_MSG_WRITE;
	memset(data, 0xbb, 0xa00);
	payload = (struct ipc_msg_payload_hdd *)msg.payload;
	payload->dev_num = HDD_DEV_PM;
	payload->base_low = 0x3f0;
	payload->base_high = 0x0;
	payload->size = 0x16;
	payload->buf_address = data;
	comm_msg(DRV_PID_HDD, &msg);


	msg.type = HDD_MSG_READ;
	memset(data, 0x0, 0xa00);
	payload = (struct ipc_msg_payload_hdd *)msg.payload;
	payload->dev_num = HDD_DEV_PS;
	payload->base_low = 0x3f0;
	payload->base_high = 0;
	payload->size = 0x20;
	payload->buf_address = data;
	comm_msg(DRV_PID_HDD, &msg);

	printk("Data:");
	for (i = 0; i < 0x400; ++i) {
		if (i % 16 == 0) {
			printk("\nROW:%x  ", i/16);
		}
		printk("%x ", data[i]);
	}

	printk("\nContinue to run fs.c\n");
*/
	struct proc_msg msg;
	struct ipc_msg_payload_hdd_part *payload;

	msg.type = HDDP_MSG_OPEN;
	payload = (struct ipc_msg_payload_hdd_part *)msg.payload;
	payload->dev_num = HDDP_DEV_NUM_GEN(0, 0);
	comm_msg(DRV_PID_HDDP, &msg);

	msg.type = HDDP_MSG_OPEN;
	payload->dev_num = HDDP_DEV_NUM_GEN(4, 'a');
	comm_msg(DRV_PID_HDDP, &msg);

        while(1);
}
