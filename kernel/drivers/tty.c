#include "buffer.h"
#include "drivers/keyboard.h"
#include "drivers/tty.h"
#include "drivers/vga.h"

/*
 # Initialize TTY
 */
void tty_init(
	struct tty *ptr_tty,
	uint32 start_row,
	BOOL is_active
)
{
	/* Check TTY instance pointer */
	if (NULL == ptr_tty) {
		return;
	}

	/* Initialize */
	ptr_tty->start_row = start_row;
	ptr_tty->row_count = TTY_ROW_COUNT;
	ptr_tty->cursor_row = 0;
	ptr_tty->cursor_col = 0;

	cbuf_init(&ptr_tty->output_buf, TTY_VGA_BUF_SIZE);

	ptr_tty->is_active = is_active;
}



/*
 # Process tty requests
 @ ptr_tty : pointer to a tty struct
 */
void tty_process(struct tty *ptr_tty)
{
	/* Output related variables */
	uint32 out_data;
	uint8 char_data[2];
	struct vga_char vga_data[2];
	uint32 cursor_row;	/* Row position needed to be adjusted */
	/* Input related variables */
	uint32 in_data;


	while(1) {

		/* Handle Input Requests */
		if (IS_TRUE(ptr_tty->is_active)) {
			/* If current TTY is active, then get char from keyboard */
			while (keyboard_getchar(&in_data) == OK) {
				/* Move data from keyboard buffer to current TTY buffer */
				cbuf_write(&ptr_tty->output_buf, in_data);
			}
		}


		/* Handle Output Requests */
		/* Get one char from current TTY buffer */
		while (cbuf_read(&ptr_tty->output_buf, &out_data) == OK) {

			if (0 == (out_data & (KBMAP_UNPRINT | KBMAP_BREAK_CODE))) {
				/* Printable characters */
				char_data[0] = (uint8)(out_data & 0xff);
				char_data[1] = 0;	/* Add end mark */
				cstr_to_vga_str(vga_data, char_data);

				cursor_row = ptr_tty->cursor_row + ptr_tty->start_row;
				vga_set_cursor_location(cursor_row, ptr_tty->cursor_col);
				vga_write_screen(&cursor_row, &ptr_tty->cursor_col, vga_data, 1);
				/* Check reset cursor position */
				if (TTY_END_ROW(ptr_tty->start_row, ptr_tty->row_count) >= cursor_row) {
					ptr_tty->cursor_row = cursor_row - ptr_tty->start_row;
				} else {
					ptr_tty->cursor_row = 0;
				}

			} else {
				/* Unprintable characters - Control characters */
				if (out_data == KBC_DOWN) {
					vga_roll_down_screen();

				} else if (out_data == KBC_UP) {
					vga_roll_up_screen();
				}
			}

		}
	}

}

