#ifndef _DRV_I8259A_H_
#define _DRV_I8259A_H_


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
#define DATA_8259A_MAIN_OCW1            0xfe    // Enable only clock interrupt
#define DATA_8259A_SLAVE_OCW1           0xff    // Disable all interrupts
#define DATA_8259A_OCW2                 0x20    // Send EOI to notify 8259A interrupt processed


void init_i8259a(void);


#endif
