#ifndef _PROC_H_
#define _PROC_H_

#include "interrupt.h"
#include "ipc.h"
#include "pm.h"
#include "type.h" 

/* Process constants */
#define PROCESS_NAME_LENGTH	16
#define PROCESS_STACK_SIZE	0x100	/* 0x100 * 4 byte = 1 KB */
#define USER_PROCESS_COUNT	3

/* Process status */
typedef uint32 PROC_STATUS;
#define PROC_UNINIT		0	/* Empty - Can init.*/
#define PROC_RUNNABLE		1	/* Wait for scheduler */
#define PROC_RUNNING		2	/* Active */
#define PROC_SENDING		3	/* Sending message - Blocked */
#define PROC_RECEVING		4	/* Receving message - Blocked */
#define PROC_SLEEP		5	/* Sleep - Blocked */
#define PROC_DEAD		6	/* Dead - Can init. */

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
	uint32 apt_sender;			/* Acceptable sender - as receiver */
	uint32 receiver;			/* Message receiver - as sender */
	struct process *proc_sending_to;	/* The process which the current process sends to */
	struct process *proc_recving_from;	/* The process which the current process receives from */
};

/* Process related Functions */
void process_scheduler(void);
void schedule(void);

#endif
