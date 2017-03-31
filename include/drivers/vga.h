#ifndef _DRV_VGA_H_
#define _DRV_VGA_H_

#include "type.h"

/* Screen Range : 25*80 */
#define COUNT_CRT_MAX_ROW			25
#define COUNT_CRT_MAX_COL			80

/* Calculation Marco */
#define VGA_GET_POS(row, col)	(row * COUNT_CRT_MAX_COL + col)
#define VGA_GET_ROW(pos)	(pos / COUNT_CRT_MAX_COL)
#define VGA_GET_COL(pos)	(pos % COUNT_CRT_MAX_COL)

#pragma pack(push, 1)

struct vga_char {
	char data;		/* Character data */

	union {
		char data;	/* Color data*/
		uint8 char_B:1;
		uint8 char_G:1;
		uint8 char_R:1;
		uint8 char_H:1;	/* Highlight */

		uint8 back_B:1;
		uint8 back_G:1;
		uint8 back_R:1;
		uint8 back_H:1;	/* Blink */
	} color;
};

#pragma pack(pop)


/* VGA Functions */
void vga_set_cursor_location(uint32 row, uint32 col);
void vga_get_cursor_location(uint32 *ptr_row, uint32 *ptr_col);
void vga_set_screen_row(uint32 row);
void vga_get_screen_row(uint32 *ptr_row);
void vga_roll_up_screen(void);
void vga_roll_down_screen(void);

uint32 vga_write_screen(uint32 *ptr_row, uint32 *ptr_col, const struct vga_char *ptr_buf, uint32 count);
uint32 vga_read_screen(uint32 row, uint32 col, struct vga_char *ptr_buf, uint32 count);

uint32 cstr_to_vga_str(struct vga_char *vga_str, const char *cstr);
uint32 vga_str_to_cstr(char *cstr, const struct vga_char *vga_str);

#endif
