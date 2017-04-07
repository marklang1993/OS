// IO port operation functions declaration
// NOTE: Implemetation are in asm file

#ifndef _IO_PORT_H_
#define _IO_PORT_H_

#include "type.h"

void io_out_byte(uint16 io_port, uint8 data);
void io_in_byte(uint16 io_port, uint8 *ptr_data);

void io_out_data(uint16 io_port, void *ptr_data, uint32 size);
void io_in_data(uint16 io_port, void *ptr_data, uint32 size);

#endif

