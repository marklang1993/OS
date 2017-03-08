// Process definition
#ifndef _PROC_H_
#define _PROC_H_

#include "interrupt.h"
#include "pm.h"
#include "type.h" 

#define PROCESS_NAME_LENGTH	16
#define PROCESS_STACK_SIZE	0x100		// 0x100 * 4 byte = 1 KB
#define USER_PROCESS_COUNT	3

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
	uint32 dummy_esp;			// This esp is invalid - ignored by popad()
	uint32 ebx;
	uint32 edx;
	uint32 ecx;
	uint32 eax;
	struct int_plc_stack_frame int_frame;	// Used for priviledge change
/* High Address --- Pushed earlier*/
};


/* Process Struct */
struct process
{
	struct process_stack_frame stack_frame;
	uint32 ldt_ptr;
	struct descriptor ldt[LDT_COUNT];	// 0: Code; 1: Data
	uint32 stack[PROCESS_STACK_SIZE];	// Process Stack
	uint32 cycles;				// Remained cycles of this process

	uint32 pid;				// Process ID
	uint32 priority;			// Priority
	char name[PROCESS_NAME_LENGTH];		// Name
};

#endif
