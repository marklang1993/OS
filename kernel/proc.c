#include "lib.h"
#include "proc.h"

static uint32 scheduler_count = 0;

void process_scheduler(struct process user_process[])
{
	char msg[] = "Scheduler is Running: ";
	char count_str[] = "0000000000";
	uint32 pos = 0;

	pos = strlen(msg);
	print_cstring_pos(msg, 16, 0);

	itoa(scheduler_count, count_str);
	print_cstring_pos(count_str, 16, pos);

	++scheduler_count;
}
