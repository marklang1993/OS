#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "type.h"

/* System call interrupt nubmer */
#define INT_SYS_CALL		0x80

/* SYS_CALL defined here */
#define SYS_CALL_TEST		0


/* SYS_CALL function type in ring 0 */
typedef ptr_void_function sys_call_fp;

/* SYS_CALL handler in ring 3, syscall.asm */
void sys_call(uint32 num, int32 *ptr_ret, ...);

/* SYS_CALL handler in ring 0 */
void sys_call_dispatch(void *base_arg);

/* SYS_CALL functions in ring 0 */
void sys_call_test(void);

#endif
