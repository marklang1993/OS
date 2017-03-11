#ifndef _DRV_KEYBOARD_H
#define _DRV_KEYBOARD_H

#include "type.h"

// Keyboard Ports
#define PORT_8042_BUF_R			0x60
#define PORT_8042_BUF_W			PORT_8042_BUF_R	
#define PORT_8042_STAT_R		0x64
#define PORT_8042_CTRL_W		PORT_8042_STAT_R

// Status Register Mask
#define MASK_8042_OUT_BUF		0x1		// Out to system
#define MASK_8042_IN_BUF		0x2		// Out to 8042
#define MASK_8042_SYS_FLAG		0x4
#define MASK_8042_CMD_DATA		0x8
#define MASK_8042_TIME_OUT		0x40
#define MASK_8042_PARITY_ERR		0x80

// Keyboard Functions
void keyboard_interrupt_handler(void);
void keyboard_failed_interrupt_handler(void);

#endif
