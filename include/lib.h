// Utility functions declaration
// NOTE: Implemetation of some functions are in asm file

#ifndef _LIB_H_
#define _LIB_H_

#include "errors.h"
#include "drivers/fs/fs_lib.h"

/* After spliting kernel and user space memory,
 * this should be set to non-zero value.
 * MUST user copy_to_kernel() & copy_to_user().
 */
#define ENABLE_SPLIT_KUSPACE	0

#define MAX(a, b)		(a < b ? b : a)
#define MIN(a, b)		(a > b ? a : b)

/* memory.asm */
void memcpy(void *dst, const void *src, uint32 size);
void memset(void *ptr, uint32 val, uint32 size);

/* print.c */
uint32 abs(int32 n);
void itoa(uint32 value, char *str, int base);
uint32 strlen(const char *str);
void strcpy(char *destination, const char *source);
void print_set_location(uint32 row, uint32 col);
void print_cstring_pos(const char *ptr_string, uint32 row, uint32 col); /* Used for c-style char array */
void print_cstring(const char *ptr_string);				/* Used for c-style char array */
void print_uint32_pos(uint32 value, uint32 row, uint32 col);
void print_uint32(uint32 value);

/* printk.c */
void printb(uint32 *ptr_row, uint32 *ptr_col, const char **print_base);
void printf(const char *format, ...);
void printk(const char *format, ...);

/* file.c */
typedef FILELIB_OP_FLAG FILE_OP_FLAG; /* File Operation Flags */
#define O_READ		FLIB_O_READ
#define O_WRITE		FLIB_O_WRITE
#define O_CREATE	FLIB_O_CREATE
#define O_APPEND	FLIB_O_APPEND

#define STDIN       FILE_STDIN
#define STDOUT      FILE_STDOUT
#define STDERR      FILE_STDERR

typedef int32 FILE;
FILE open(const char *filename, FILE_OP_FLAG flag);
uint32 write(FILE fd, const void *buf, uint32 size);
uint32 read(FILE fd, void *buf, uint32 size);
rtc close(FILE fd);
rtc mkfs(uint8 mbr_index, uint8 logical_index);

#endif
