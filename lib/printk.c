#include "lib.h"
#include "drivers/tty.h"
#include "drivers/vga.h"

#define 	STRBUF_LEN		40
#define		DEFAULT_COLOR		0x0f

/*
 # Format Print (Base version)
 @ ptr_row    : pointer to absolute row postion
 @ ptr_col    : pointer to absolute column position
 @ print_base : pointer to print format string
 */
void printb(uint32 *ptr_row, uint32 *ptr_col, const char **print_base)
{
	const char *format = *print_base;
	uint32 *base_arg = ((uint32 *)(print_base)) + 1;
	const char *ptr_char;
	const int32 *ptr_int32;
	const uint32 *ptr_uint32;
	char str_buf[STRBUF_LEN];
	struct vga_char vga_str_buf[STRBUF_LEN];
	uint32 index = 0;	/* index for 'format' */
	uint32 str_index = 0;	/* index for %s */
	uint32 str_len = 0;
	uint32 i;

	while(format[index] != (char)0) {

		switch (format[index]) {

		case '%':
			++index;
			if (format[index] != (char)0) {
				switch (format[index]) {

				case 's':
					ptr_char = (const char *)(*base_arg);
					for(str_index = 0; 
					    ptr_char[str_index] != 0;
					    ++str_index) {
						vga_str_buf[0].data = ptr_char[str_index];
						vga_str_buf[0].color.data = DEFAULT_COLOR;
						vga_write_screen(ptr_row, ptr_col, vga_str_buf, 1);
					}

					++base_arg;
					break;

				case 'd':
					ptr_int32 = (const int32 *)(base_arg);
					if (*ptr_int32 < 0) {
						vga_str_buf[0].data = '-';
						vga_str_buf[0].color.data = DEFAULT_COLOR;
						itoa(abs(*ptr_int32), str_buf, 10);
						cstr_to_vga_str(&vga_str_buf[1], str_buf);
						str_len = 1 + strlen(str_buf);
					} else {
						itoa(*ptr_int32, str_buf, 10);
						cstr_to_vga_str(vga_str_buf, str_buf);
						str_len = strlen(str_buf);
					}

					vga_write_screen(ptr_row, ptr_col, vga_str_buf, str_len);
					++base_arg;
					break;

				case 'u':
					ptr_uint32 = (const uint32 *)(base_arg);

					itoa(*ptr_uint32, str_buf, 10);
					cstr_to_vga_str(vga_str_buf, str_buf);
					str_len = strlen(str_buf);

					vga_write_screen(ptr_row, ptr_col, vga_str_buf, str_len);
					++base_arg;
					break;

				case 'x':
					ptr_uint32 = (const uint32 *)(base_arg);

					itoa(*ptr_uint32, str_buf, 16);
					cstr_to_vga_str(vga_str_buf, str_buf);
					str_len = strlen(str_buf);

					vga_write_screen(ptr_row, ptr_col, vga_str_buf, str_len);
					++base_arg;
					break;

				default:
					vga_str_buf[0].data = '%';
					vga_str_buf[0].color.data = DEFAULT_COLOR;
					vga_write_screen(ptr_row, ptr_col, vga_str_buf, 1);
					--index;
				}
			}
			break;

		case '\n':
			*ptr_row += 1;
			*ptr_col = 0;
			break;

		case '\t':
			for(i = 0; i < TTY_TAB_SPACES; ++i) {
				vga_str_buf[0].data = ' ';
				vga_str_buf[0].color.data = DEFAULT_COLOR;
				vga_write_screen(ptr_row, ptr_col, vga_str_buf, 1);
			}
			break;

		default:
			vga_str_buf[0].data = format[index];
			vga_str_buf[0].color.data = DEFAULT_COLOR;
			vga_write_screen(ptr_row, ptr_col, vga_str_buf, 1);
		}

		++index;
	}

}


/*
 # Format Print (User version)
 @ fmt : print format string
 @ ... : other arguments
 */
void printf(const char *fmt, ...)
{
	static uint32 row = 0;
	static uint32 col = 0;
	printb(&row, &col, &fmt);
}


/*
 # Format Print (Kernel version)
 @ fMt : print format string
 @ ... : other arguments
 */
void printk(const char *fmt, ...)
{
	static uint32 row = 0;
	static uint32 col = 0;
	printb(&row, &col, &fmt);
}


