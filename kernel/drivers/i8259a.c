#include "io_port.h"
#include "drivers/i8259a.h"

/* 8259A Interrupt Handlers */
static i8259a_handler_entry i8259a_handlers[COUNT_INT_8259A];

/*
 # 8259A Interrupt Dispatch function
 # Called by interrupt.asm
 @ index : 8259A interrupt index
 */
void i8259a_interrupt_dispatch(uint32 index)
{
	/* Check boundary */
	if (index < COUNT_INT_8259A) {
		/* Check the existance of i8259a handler */
		if (NULL != i8259a_handlers[index]) {
			i8259a_handlers[index]();
		}
	}
}


/*
 # Initialize 8259A Driver and Chip
 */
void i8259a_init(void)
{
	int i;

	/* # Init. 8259A interrupt handlers */
	for (i = 0; i < COUNT_INT_8259A; ++i) {
		i8259a_handlers[i] = NULL;
	}

	/* # Program 8259A */
        /* ICW1 */
        io_out_byte(PORT_8259A_MAIN_0, DATA_8259A_ICW1);
        io_out_byte(PORT_8259A_SLAVE_0, DATA_8259A_ICW1);
        /* ICW2 */
        io_out_byte(PORT_8259A_MAIN_1, DATA_8259A_MAIN_ICW2);
        io_out_byte(PORT_8259A_SLAVE_1, DATA_8259A_SLAVE_ICW2);
        /* ICW3 */
        io_out_byte(PORT_8259A_MAIN_1, DATA_8259A_MAIN_ICW3);
        io_out_byte(PORT_8259A_SLAVE_1, DATA_8259A_SLAVE_ICW3);
        /* ICW4 */
        io_out_byte(PORT_8259A_MAIN_1, DATA_8259A_ICW4);
        io_out_byte(PORT_8259A_SLAVE_1, DATA_8259A_ICW4);
        /* OCW1 */
        io_out_byte(PORT_8259A_MAIN_1, DATA_8259A_MAIN_OCW1);
        io_out_byte(PORT_8259A_SLAVE_1, DATA_8259A_SLAVE_OCW1);
}

/*
 # Set 8259A Interrupt Handler
 @ index          : 8259A interrupt index
 @ handler        : 8259A interrupt handler
 */
rtc i8259a_set_handler(
	uint32 index, 
	i8259a_handler_entry handler
)
{
	/* Check boundary */
	if (index >= COUNT_INT_8259A) {
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
	uint8 value;

	/* Check boundary */
	if (index >= COUNT_INT_8259A) {
		return EINVARG;
	}

	/* Check master OR slave */
	if (index < COUNT_INT_8259A_MASTER) {
		/* Master */
		io_in_byte(PORT_8259A_MAIN_1, &value);
		value = value & (~(1 << index));
		io_out_byte(PORT_8259A_MAIN_1, value);

	} else {
		/* Slave */
		index -= COUNT_INT_8259A_MASTER;
		io_in_byte(PORT_8259A_SLAVE_1, &value);
		value = value & (~(1 << index));
		io_out_byte(PORT_8259A_SLAVE_1, value);
	}
	
	return OK;
}


/*
 # Disable a 8259A interrupt
 @ index   : 8259A interrupt index
 */
rtc i8259a_int_disable(uint32 index)
{
	uint8 value;

	/* Check boundary */
	if (index >= COUNT_INT_8259A) {
		return EINVARG;
	}

	/* Check master OR slave */
	if (index < COUNT_INT_8259A_MASTER) {
		/* Master */
		io_in_byte(PORT_8259A_MAIN_1, &value);
		value = value | (1 << index);
		io_out_byte(PORT_8259A_MAIN_1, value);

	} else {
		/* Slave */
		index -= COUNT_INT_8259A_MASTER;
		io_in_byte(PORT_8259A_SLAVE_1, &value);
		value = value | (1 << index);
		io_out_byte(PORT_8259A_SLAVE_1, value);
	}

	return OK;
}
