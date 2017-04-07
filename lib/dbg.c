#include "dbg.h"
#include "interrupt.h"
#include "lib.h"

void assert_fail(
	BOOL is_kernel,
	const char *exp,
	const char *file,
	const char *base_file,
	uint32 line
)
{
	printk("ASSERTION FAILED: %s\n\tFILE: %s\n\tBASE_FILE: %s\n\tLINE:%u",
		exp, file, base_file, line);

	while(1);	/* Stop here */
}


void panic(const char *format, ...)
{
	char msg[] = "KERNEL PANIC: ";
	const char *ptr_msg = msg;
	const char **pp_msg = &ptr_msg;
	uint32 row, col;

	row = 0;
	col = 0;

	cli();		/* Turn off interrupt */

	printb(&row, &col, pp_msg);
	printb(&row, &col, &format);
	
	while(1);	/* Stop here */
}
