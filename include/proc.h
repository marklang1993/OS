#ifndef _PROC_H_
#define _PROC_H_

#include "buffer.h"
#include "interrupt.h"
#include "ipc.h"
#include "pm.h"
#include "type.h" 

/* Driver Process ID definition */
#define DRV_PID_HDD		4
#define DRV_PID_HDDP		5
#define DRV_PID_FS		6

/* Process constants */
#define PROCESS_NAME_LENGTH	16
#define PROCESS_STACK_SIZE	0x400	/* 0x400 * 4 byte = 4 KB */
#define USER_PROCESS_COUNT	7

/* Process status */
typedef uint32 PROC_STATUS;
#define PROC_UNINIT		0	/* Empty - Can init.*/
#define PROC_RUNNABLE		1	/* Wait for scheduler */
#define PROC_RUNNING		2	/* Active */
#define PROC_SENDING		3	/* Sending message - Blocked */
#define PROC_RECEIVING		4	/* Receiving message - Blocked */
#define PROC_WAIT_INT		5	/* Wait for interrupt - Blocked */
#define PROC_SLEEP		6	/* Sleep - Blocked */
#define PROC_DEAD		7	/* Dead - Can init. */

/* Process Stack Frame - Kernel & User*/
struct process_stack_frame
{
/* Low Address --- Pushed later*/
	uint32 gs;
	uint32 fs;
	uint32 es;
	uint32 ds;
	uint32 edi;
	uint32 esi;
	uint32 ebp;
	uint32 dummy_esp;			/* This esp is invalid - ignored by popad() */
	uint32 ebx;
	uint32 edx;
	uint32 ecx;
	uint32 eax;
	struct int_plc_stack_frame int_frame;	/* Used for privilege change */
/* High Address --- Pushed earlier*/
};


/* Process Struct */
struct process
{
	struct process_stack_frame stack_frame;
	uint32 ldt_ptr;
	struct descriptor ldt[LDT_COUNT];	/* 0: Code; 1: Data */
	uint32 stack[PROCESS_STACK_SIZE];	/* Process Stack */
	uint32 cycles;				/* Remained cycles of this process */

	uint32 pid;				/* Process ID */
	uint32 priority;			/* Priority */
	char name[PROCESS_NAME_LENGTH];		/* Name */

	PROC_STATUS status;			/* Process status */
	struct proc_msg msg_buf;		/* Process message buffer - copy from user space OR kernel space */
	BOOL is_interrupt;			/* Interrupt occur flag */
	struct process *proc_sending_to; 	/* The process which the current process is sending to */
	struct cbuf recv_queue;			/* Queue of all sending process */
};

/* Process related Functions */
void process_scheduler(void);
void schedule(void);

#endif
