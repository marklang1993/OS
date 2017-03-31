#include "lib.h"
#include "syscall.h"

/* SYS_CALL table defined here */
sys_call_fp sys_call_table[] = {
	sys_call_test
};

/*
 # Dispatch system call request
 @ base_arg : base address of all arguments
 */
void sys_call_dispatch(void *base_arg)
{
	uint32 num = *((uint32 *)base_arg);
	int32 *ptr_ret = ((int32 *)base_arg) + 1;

	while(1);
}

void sys_call_test(void)
{
	;
}
