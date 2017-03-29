#ifndef _DRV_TTY_H_
#define _DRV_TTY_H_

#include "buffer.h"
#include "drivers/vga.h"

/* TTY Macro Functions */
#define TTY_END_ROW(start, count) 	(start + count - 1)
#define TTY_SET_CUR_ROW(row)		ptr_tty->cursor_row = (row) % ptr_tty->row_count
#define TTY_SET_CUR_COL(col)		ptr_tty->cursor_col = (col) % COUNT_CRT_MAX_COL


/* TTY Constants */
#define TTY_ROW_COUNT			50
#define TTY_TAB_SPACES			8
#define TTY_VGA_BUF_SIZE		(COUNT_CRT_MAX_COL * 2)

/* TTY struct */
struct tty
{
	/* tty vga resource */
	uint32 start_row;	/* Screen range */
	uint32 row_count;
	uint32 cursor_row;	/* Cursor position */
	uint32 cursor_col;
	struct cbuf output_buf; /* Output buffer */
	struct strbuf input_buf;/* Input buffer - only record printable chars from keyboard */

	BOOL is_active;		/* Flags : indicate is this tty under operation*/
};


/* TTY functions */
void tty_init(struct tty *ptr_tty, uint32 start_row, BOOL is_active);
void tty_uninit(struct tty *ptr_tty);
void tty_process(struct tty *ptr_tty);

#endif
