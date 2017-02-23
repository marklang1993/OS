// Utility functions declaration
// NOTE: Implemetation of some functions are in asm file

#ifndef _LIB_H_
#define _LIB_H_

#include "type.h"

void memcpy(void *src, void *dst, uint32 size);

void print_string(const char *ptr_string, uint32 length, uint32 row, uint32 col);

void itoa(uint32 value, char *str);
void print_set_location(uint32 row, uint32 col);
void print_cstring(const char *ptr_string);	// Used for c-style char array
void print_uint32(uint32 value);

#endif
