/* Common type definitions */


#ifndef _TYPE_H_
#define _TYPE_H_

#ifndef NULL
	#define 	NULL		0
#endif

/* BOOL type related */
#define 	TRUE		1
#define		FALSE		0
#define		NOT(x)		(x ^ TRUE)
#define		IS_TRUE(x)	(TRUE == x)

/* uint64 type related */
#define		TO_UINT64(high, low) \
	((uint64)((((uint64)high) << 32) + (uint64)low))
#define		UINT64_HIGH(val) \
	((uint32)((val & 0xffffffff00000000ull) >> 32))
#define		UINT64_LOW(val) \
	((uint32)(val & 0xffffffffull))

/* Numeric value definitions */
typedef unsigned long long	uint64;
typedef unsigned int 		uint32;
typedef unsigned short		uint16;
typedef unsigned char		uint8;
typedef signed long long	int64;
typedef signed int		int32;
typedef signed short		int16;
typedef signed char		int8;
typedef unsigned int		BOOL;

/* Function pointer definitions */
typedef void (*ptr_void_function)(void);	/* Function pointer of void type function */

#endif

