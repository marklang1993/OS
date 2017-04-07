#ifndef _DRV_HDD_H_
#define _DRV_HDD_H_

#include "proc.h"
#include "type.h"

/* Harddisk Driver Message Type */
#define HDD_MAGIC_NUM	'h'

#define HDD_MSG_OK	IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 0)
#define HDD_MSG_ERROR	IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, -1)

#define HDD_MSG_OPEN	IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 10)
#define HDD_MSG_WRITE	IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 11)
#define HDD_MSG_READ	IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 12)
#define HDD_MSG_CLOSE	IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 13)


/* Harddisk Driver Functions */
void hdd_interrupt_handler(void);

void hdd_init(void);
void hdd_message_dispatcher(void);


#endif
