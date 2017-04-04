#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "errors.h"

/* System call interrupt nubmer */
#define INT_SYS_CALL		0x80

/* SYS_CALL defined here */
#define SYS_CALL_TEST		0
#define SYS_CALL_SEND_MSG	1
#define SYS_CALL_RECV_MSG	2


/* SYS_CALL function type in ring 0 */
typedef rtc (*sys_call_fp)(void *);

/* SYS_CALL handler in ring 3, syscall.asm */
void sys_call(uint32 num, rtc *ptr_ret, ...);


/* TODO: Current design assumes the kernel uses
 * the same address space as the user process.
 * Add: copy_to_user() & copy_to_kernel();
 */
/* SYS_CALL handler in ring 0 */
void sys_call_dispatch(void *base_arg);

/* SYS_CALL functions in ring 0 */
rtc sys_call_test(void *base_arg);

#endif
