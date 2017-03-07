// Utility functions declaration
// NOTE: Implemetation of some functions are in asm file

#ifndef _LIB_H_
#define _LIB_H_

#include "type.h"

// memory.asm
void memcpy(void *dst, const void *src, uint32 size);
void memset(void *ptr, uint32 val, uint32 size);

// print_string.asm
void print_string(const char *ptr_string, uint32 length, uint32 row, uint32 col);

// print.c
void itoa(uint32 value, char *str);
uint32 strlen(const char *str);
void print_set_location(uint32 row, uint32 col);
void print_cstring_pos(const char *ptr_string, uint32 row, uint32 col); // Used for c-style char array
void print_cstring(const char *ptr_string);				// Used for c-style char array
void print_uint32(uint32 value);

#endif
