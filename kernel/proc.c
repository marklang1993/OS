#include "interrupt.h"
#include "lib.h"
#include "proc.h"

struct process *current_user_process;
static uint32 scheduler_count = 0;

/*
 # Process Scheduler Function - Main Body
 */ 
static void scheduler_main()
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


/*
 # Clock Interrupt Handler
 */
void process_scheduler(void)
{
	char msg[] = "Scheduler is Running! Times: ";
	char count_str[] = "0000000000";
	uint32 pos = 0;

	// Print Message
	pos = strlen(msg);
	print_cstring_pos(msg, 16, 0);

	itoa(scheduler_count, count_str);
//	itoa((uint32)current_user_process, count_str);
	print_cstring_pos(count_str, 16, pos);

	++scheduler_count;

	// Go to real scheduler
	scheduler_main();
}
