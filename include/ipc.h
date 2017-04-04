#ifndef _IPC_H_
#define _IPC_H_

#include "errors.h"
#include "proc.h"

/* IPC pid */
#define IPC_PROC_ALL		USER_PROCESS_COUNT
#define IPC_PROC_NO		USER_PROCESS_COUNT + 1

/* IPC message source */
typedef uint32 IPC_MSG_SRC;
#define IPC_MSG_INTTERUPT	0

/* IPC message type */
typedef uint32 IPC_MSG_TYPE;
#define IPC_MSG_INT_KEYBOARD	0

struct proc_msg
{
	IPC_MSG_SRC	msg_src;
	IPC_MSG_TYPE	msg_type;
};

/* IPC - send_msg() / recv_msg() - In Ring 1/3
 * comm_msg(): send & receive message (wrapper)
 */
rtc send_msg(uint32 dst, struct proc_msg *ptr_msg);
rtc recv_msg(uint32 src, struct proc_msg *ptr_msg);
rtc comm_msg(uint32 dst, struct proc_msg *ptr_msg);

/* IPC - sys_call_send_msg() / sys_call_recv_msg() - In Ring 0 */
rtc sys_call_send_msg(void *base_arg);
rtc sys_call_recv_msg(void *base_arg);


#endif
