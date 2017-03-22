#include "interrupt.h"
#include "lib.h"
#include "proc.h"

struct process *current_user_process;

/*
 # Clock Interrupt Handler
 */
void process_scheduler(void)
{
	uint32 current_pid, next_pid;
	uint32 offset_idx;

	current_pid = current_user_process->pid;
	
	// Check remained cycles: cycles == 0 => next process
	if (0 == current_user_process->cycles)
	{
		// Switch to next process & refill the remained cycles of current process
		current_user_process->cycles = USER_PROCESS_COUNT - current_user_process->priority;

		next_pid = (current_pid + 1) % USER_PROCESS_COUNT;
		if(next_pid < current_pid)
		{
			// wrap back
			offset_idx = current_pid - next_pid;
			current_user_process -= offset_idx;
		}
		else
		{
			// forward
			offset_idx = next_pid - current_pid;
			current_user_process += offset_idx;
		}
	}
	else
	{
		// Remained on current process
		current_user_process->cycles -= 1;
	}
}

