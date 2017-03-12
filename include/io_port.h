// IO port operation functions declaration
// NOTE: Implemetation are in asm file

#ifndef _IO_PORT_H_
#define _IO_PORT_H_

#include "type.h"

void io_out_byte(uint16 io_port, uint8 data);
void io_in_byte(uint16 io_port, uint8 *ptr_data);

#endif

