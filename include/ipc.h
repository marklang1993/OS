#ifndef _IPC_H_
#define _IPC_H_

#include "errors.h"
#include "proc.h"

/* IPC pid */
#define IPC_PROC_ALL		USER_PROCESS_COUNT
#define IPC_PROC_NO		USER_PROCESS_COUNT + 1
#define IPC_PROC_INT		USER_PROCESS_COUNT + 2

/* IPC message source - could be real pid "OR" IPC pid */
typedef uint32 IPC_MSG_SRC;

/* IPC message type */
typedef uint32 IPC_MSG_TYPE;
#define IPC_MSG_INT_KEYBOARD	0

struct proc_msg
{
	IPC_MSG_SRC	msg_src;	/* Source PID */
	IPC_MSG_TYPE	msg_type;	/* Message Type */
};

/* IPC - send_msg() / recv_msg() - In Ring 1/3
 * comm_msg(): send & receive message (wrapper)
 */
rtc send_msg(uint32 dst, struct proc_msg *ptr_msg);
rtc recv_msg(uint32 src, struct proc_msg *ptr_msg);
rtc comm_msg(uint32 dst, struct proc_msg *ptr_msg);

/* IPC - interrupt related message processing
 * wait_int()   : wait for interrupt
 * resume_int() : restart the process after interrupt
 */
rtc wait_int();		/* ring 1/3 */
void resume_int();	/* ring 0 */

/* IPC - sys_call_send_msg() / sys_call_recv_msg() - In Ring 0 */
rtc sys_call_send_msg(void *base_arg);
rtc sys_call_recv_msg(void *base_arg);


#endif
