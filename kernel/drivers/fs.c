#include "lib.h"
#include "drivers/fs.h"
#include "drivers/hdd.h"

/* File System Driver Message Dispatcher */
void fs_message_dispatcher(void)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *payload;
	uint8 data[0xa00];
	uint32 i;

        msg.msg_type = HDD_MSG_OPEN;
        send_msg(DRV_PID_HDD, &msg);
	recv_msg(DRV_PID_HDD, &msg);

	msg.msg_type = HDD_MSG_WRITE;
	memset(data, 0xcc, 0xa00);
	payload = (struct ipc_msg_payload_hdd *)msg.payload;
	payload->is_master = FALSE;
	payload->pos = 1;
	payload->count = 5;
	payload->buf_address = data;
	send_msg(DRV_PID_HDD, &msg);


        while(1);
}
