#ifndef _DRV_TTY_H_
#define _DRV_TTY_H_

#include "type.h"
#include "drivers/vga.h"

/* TTY Macros */
#define TTY_END_ROW(start, count) 	(start + count - 1)

/* TTY Constants */
#define TTY_VGA_BUF_SIZE		(COUNT_CRT_MAX_COL * 2)


/* TTY struct */
struct tty
{
	/* tty vga resource */
	uint32 start_row;	/* Screen range */
	uint32 row_count;
	uint32 cursor_row;	/* Cursor position */
	uint32 cursor_col;
	struct vga_char vga_buf[TTY_VGA_BUF_SIZE];

	BOOL is_active;		/* Flags : indicate is this tty under operation*/
};


/* TTY functions */
void tty_process(tty *ptr_tty);

#endif
