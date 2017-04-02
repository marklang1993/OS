#ifndef _IPC_H_
#define _IPC_H_

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


#endif
