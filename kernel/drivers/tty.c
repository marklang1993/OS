#include "buffer.h"
#include "drivers/keyboard.h"
#include "drivers/tty.h"
#include "drivers/vga.h"

static uint32 tty_prompt_str[] = {'>', ' '};

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

	cbuf_init(
		&ptr_tty->output_buf,
		TTY_VGA_BUF_SIZE,
		tty_prompt_str,
		sizeof(tty_prompt_str) / sizeof(uint32)
	);

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
				TTY_SET_CUR_ROW(cursor_row - ptr_tty->start_row);

			} else {
				/* Unprintable characters - Control characters */
				switch (out_data) {

				case KBC_DOWN:
					/* Down */
					vga_roll_down_screen();
					break;

				case KBC_UP:
					/* Up */
					vga_roll_up_screen();
					break;

				case KBC_ENTER:
				case KBC_PAD_ENTER:
					/* Enter */
					TTY_SET_CUR_ROW(ptr_tty->cursor_row + 1);
					ptr_tty->cursor_col = 0;

					cbuf_write(&ptr_tty->output_buf, tty_prompt_str[0]);
					cbuf_write(&ptr_tty->output_buf, tty_prompt_str[1]);
					break;

				case KBC_BACKSPACE:
					/* Delete */
					break;

				case KBC_TAB:
					/* Delete */
					break;
				}
			}

		}
	}

}

