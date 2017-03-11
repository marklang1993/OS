#ifndef _DRV_I8253_H
#define _DRV_I8253_H

#include "type.h"

// 8253 Ports
#define PORT_8253_COUNTER_0		0x40
#define PORT_8253_COUNTER_1		0x41
#define PORT_8253_COUNTER_2		0x42
#define PORT_8253_CTRL_REG		0x43

// 8253 Modes
#define MODE_8253_0			0	// Interrupt on Terminal Count
#define MODE_8253_1			1	// Hardware Retriggerable one-shot
#define MODE_8253_2			2	// Rate Generator
#define MODE_8253_3			3	// Square Wave
#define MODE_8253_4			4	// Software Triggered Strobe
#define MODE_8253_5			5	// Hardware Triggered Strobe

// 8253 Units
#define UNIT_8253_BIN			0
#define UINT_8253_BCD			1

// 8253 Operations
#define OP_8253_LATCH			0
#define OP_8253_RW_H			1
#define OP_8253_RW_L			2
#define OP_8253_RW_L_H			3

// 8253 Selection
#define SEL_8253_COUNTER_0		0
#define SEL_8253_COUNTER_1		1
#define SEL_8253_COUNTER_2		2

// 8253 Frequency (Hz)
#define FEQ_8253			1193180

// 8253 Interval (ms)
#define INTERVAL_8253			10

// 8253 Functions
void i8253_init(void);

#endif
