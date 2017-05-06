#include "lib.h"
#include "drivers/fs.h"
#include "drivers/hdd.h"

/* File System Driver Message Dispatcher */
void fs_message_dispatcher(void)
{
	struct proc_msg msg;

        msg.msg_type = HDD_MSG_OPEN;

        printk("FS SEND MSG!\n");
        send_msg(DRV_PID_HDD, &msg);
        while(1);
}
