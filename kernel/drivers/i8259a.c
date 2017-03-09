#include "io_port.h"
#include "drivers/i8259a.h"

void init_i8259a(void)
{
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
