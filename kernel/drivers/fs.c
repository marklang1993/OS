#include "lib.h"
#include "part_table.h"
#include "drivers/fs.h"
#include "drivers/hdd.h"

/* File System Driver Message Dispatcher */
void fs_message_dispatcher(void)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *payload;
	uint8 data[0xa00];
	uint32 i;

        msg.type = HDD_MSG_OPEN;
        comm_msg(DRV_PID_HDD, &msg);

	msg.type = HDD_MSG_WRITE;
	memset(data, 0xbb, 0xa00);
	payload = (struct ipc_msg_payload_hdd *)msg.payload;
	payload->is_master = FALSE;
	payload->base_low = 0x3f0;
	payload->base_high = 0x0;
	payload->size = 0x16;
	payload->buf_address = data;
	comm_msg(DRV_PID_HDD, &msg);


	msg.type = HDD_MSG_READ;
	memset(data, 0x0, 0xa00);
	payload = (struct ipc_msg_payload_hdd *)msg.payload;
	payload->is_master = FALSE;
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
        while(1);
}
