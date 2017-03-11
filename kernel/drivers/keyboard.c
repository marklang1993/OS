#include "interrupt.h"
#include "io_port.h"
#include "lib.h"
#include "drivers/keyboard.h"

/*
 # Keyboard Interrupt Handler
 */
void keyboard_interrupt_handler(void)
{
	uint32 data;
	
	char data_str[] = "0000000000";

	io_in_byte(PORT_8042_BUF_R, &data);

	itoa(data, data_str);
	print_cstring_pos(data_str, 20, 0);
}

/*
 # Keyboard Re-enter Failed Interrupt Handler
 */
void keyboard_failed_interrupt_handler(void)
{
	uint32 status, data;
	
	do
	{
		// At least 1 scan code needs to be read
		io_in_byte(PORT_8042_BUF_R, &data);
		
		// Check status
		io_in_byte(PORT_8042_STAT_R, &status);
		status &= MASK_8042_OUT_BUF;
	}
	while(status != 0);	
		
	print_cstring_pos("*", 2, 8);
}
