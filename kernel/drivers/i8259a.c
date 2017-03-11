#include "io_port.h"
#include "drivers/i8259a.h"

/*
 # 8259A Interrupt Dispatch function
 # Called by interrupt.asm
 @ index : 8259A interrupt index
 */
void i8259a_interrupt_dispatch(uint32 index)
{
	// Check boundary
	if(index < COUNT_INT_8259A)
	{
		// Check the existance of i8259a handler
		if(NULL != i8259a_handlers[index])
		{
			i8259a_handlers[index]();
		}
	}
}


/*
 # Initialize 8259A driver and chip
 */
void i8259a_init(void)
{
	int i;

	// # Init. 8259A interrupt handlers
	for (i = 0; i < COUNT_INT_8259A; ++i)
	{
		i8259a_handlers[i] = NULL;
	}

	// # Program 8259A
        // ICW1
        io_port_out(PORT_8259A_MAIN_0, DATA_8259A_ICW1);
        io_port_out(PORT_8259A_SLAVE_0, DATA_8259A_ICW1);
        // ICW2
        io_port_out(PORT_8259A_MAIN_1, DATA_8259A_MAIN_ICW2);
        io_port_out(PORT_8259A_SLAVE_1, DATA_8259A_SLAVE_ICW2);
        // ICW3
        io_port_out(PORT_8259A_MAIN_1, DATA_8259A_MAIN_ICW3);
        io_port_out(PORT_8259A_SLAVE_1, DATA_8259A_SLAVE_ICW3);
        // ICW4
        io_port_out(PORT_8259A_MAIN_1, DATA_8259A_ICW4);
        io_port_out(PORT_8259A_SLAVE_1, DATA_8259A_ICW4);
        // OCW1
        io_port_out(PORT_8259A_MAIN_1, DATA_8259A_MAIN_OCW1);
        io_port_out(PORT_8259A_SLAVE_1, DATA_8259A_SLAVE_OCW1);
}

/*
 # Set 8259A Interrupt Handler
 @ index   : 8259A interrupt index
 @ handler : 8259A interrupt handler
 */
rtc i8259a_set_handler(uint32 index, i8259a_handler_entry handler)
{
	// Check boundary and handler
	if(index >= COUNT_INT_8259A || NULL == handler)
	{
		return EINVARG;
	}

	i8259a_handlers[index] = handler;
	return OK;	
}


/*
 # Enable a 8259A interrupt
 @ index   : 8259A interrupt index
 */
rtc i8259a_int_enable(uint32 index)
{
	// Check boundary
	if(index >= COUNT_INT_8259A)
	{
		return EINVARG;
	}
	
	return OK;
}


/*
 # Disable a 8259A interrupt
 @ index   : 8259A interrupt index
 */
rtc i8259a_int_disable(uint32 index)
{
	// Check boundary
	if(index >= COUNT_INT_8259A)
	{
		return EINVARG;
	}

	return OK;
}

