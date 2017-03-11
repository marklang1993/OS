#ifndef _DRV_I8259A_H_
#define _DRV_I8259A_H_

#include "errors.h"
#include "type.h"

// 8259A Ports 
#define PORT_8259A_MAIN_0               0x20
#define PORT_8259A_MAIN_1               0x21
#define PORT_8259A_SLAVE_0              0xa0
#define PORT_8259A_SLAVE_1              0xa1


// ICW1 -> PORT0, ICW2~4 -> PORT1
#define DATA_8259A_ICW1                 0x11
#define DATA_8259A_MAIN_ICW2            0x20    // IR0 <-> 32   
#define DATA_8259A_SLAVE_ICW2           0x28    // IR8 <-> 40
#define DATA_8259A_MAIN_ICW3            0x04    // Main IR2 (3rd bit) -> Slave 8259A
#define DATA_8259A_SLAVE_ICW3           0x02    // Slave 8259A -> Main IR2
#define DATA_8259A_ICW4                 0x01    // x86 mode, auto EOI

// OCW1 -> PORT1, OCW2(EOI) -> PORT0
#define DATA_8259A_MAIN_OCW1            0xfc    // Enable only clock & keyboard
#define DATA_8259A_SLAVE_OCW1           0xff    // Disable all interrupts
#define DATA_8259A_OCW2                 0x20    // Send EOI to notify 8259A interrupt processed

// 8259A interrupt index
#define INDEX_8259A_CLOCK               0
#define INDEX_8259A_KEYBOARD            1

#define COUNT_INT_8259A			16

// 8259A Types
typedef ptr_void_function i8259a_handler_entry;

// 8259A Functions
void i8259a_interrupt_dispatch(uint32 index);		// This function is used in interrupt.asm
void i8259a_failed_interrupt_dispatch(uint32 index);	// This function is used in interrupt.asm


void i8259a_init(void);
rtc i8259a_set_handler(uint32 index, i8259a_handler_entry handler, i8259a_handler_entry failed_handler);
rtc i8259a_int_enable(uint32 index);
rtc i8259a_int_disable(uint32 index);

#endif
