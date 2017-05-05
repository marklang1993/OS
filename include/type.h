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

/* Numeric value definitions */
typedef unsigned int 	uint32;
typedef unsigned short 	uint16;
typedef unsigned char	uint8;
typedef signed int	int32;
typedef signed short	int16;
typedef signed char	int8;
typedef unsigned int	BOOL;

/* Function pointer definitions */
typedef void (*ptr_void_function)(void);	/* Function pointer of void type function */

#endif

