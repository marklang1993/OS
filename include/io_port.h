// IO port operation functions declaration
// NOTE: Implemetation are in asm file

#ifndef _IO_PORT_H_
#define _IO_PORT_H_

#include "type.h"

void io_out_byte(uint16 io_port, uint8 data);
void io_in_byte(uint16 io_port, uint8 *ptr_data);

void io_bulk_out_word(uint16 io_port, uint16 *ptr_data, uint32 count);
void io_bulk_in_word(uint16 io_port, uint16 *ptr_data, uint32 count);

#endif

