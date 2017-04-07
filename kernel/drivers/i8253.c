#include "io_port.h"
#include "drivers/i8253.h"

/* 8253 Ports */
#define PORT_8253_COUNTER_0             0x40
#define PORT_8253_COUNTER_1             0x41
#define PORT_8253_COUNTER_2             0x42
#define PORT_8253_CTRL_REG              0x43


/* 8253 Control Register Data */
union i8253_ctrl_reg_data
{
	uint8 data;		/* Actual data */

	uint8 unit:1;		/* Unit */
	uint8 mode:3;		/* Mode */
	uint8 operation:2;	/* Read/Write/Latch Operation */
	uint8 selection:2;	/* Selection of Counter */
};

/*
 # Initialize 8253 chip
 */
void i8253_init(void)
{
	union i8253_ctrl_reg_data data;
	uint32 counter_val;

	/* Init. 8253 Controller Register Data */
	data.data = 0;
	data.unit = UNIT_8253_BIN;
	data.mode = MODE_8253_2;
	data.operation = OP_8253_RW_L_H;
	data.selection = SEL_8253_COUNTER_0;

	/* Init. 8253 Controller Register */
	io_out_byte(PORT_8253_CTRL_REG, data.data);

	/* Init. 8253 Counter0 */
	counter_val = FEQ_8253 / (1000 / INTERVAL_8253);
	io_out_byte(PORT_8253_COUNTER_0, (counter_val & 0xff));
	io_out_byte(PORT_8253_COUNTER_0, ((counter_val & 0xff00) >> 8));
}
