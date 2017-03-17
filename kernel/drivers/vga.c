#include "interrupt.h"
#include "io_port.h"
#include "drivers/vga.h"

/* VGA Ports */
#define PORT_VGA_CRT_CTRL_ADDR                  0x3d4
#define PORT_VGA_CRT_CTRL_DATA                  0x3d5

#define IDX_VGA_CRT_CTRL_ADDR_START_ADDR_H      0x0c
#define IDX_VGA_CRT_CTRL_ADDR_START_ADDR_L      0x0d
#define IDX_VGA_CRT_CTRL_ADDR_CUR_LOC_H         0x0e
#define IDX_VGA_CRT_CTRL_ADDR_CUR_LOC_L         0x0f

/*
 # VGA Set Cursor Location
 @ row:	cursor row location
 @ col: cursor column location
 */
void vga_set_cursor_location(uint32 row, uint32 col)
{
	/* Convert row & col to actual registers' values */
	uint32 pos = row * COUNT_CRT_MAX_COL + col;
	uint8 pos_high = (uint8)((pos >> 8) & 0xff);
	uint8 pos_low = (uint8)(pos & 0xff);

	/* Set registers */
	cli();
	io_out_byte(PORT_VGA_CRT_CTRL_ADDR, IDX_VGA_CRT_CTRL_ADDR_CUR_LOC_L);
	io_out_byte(PORT_VGA_CRT_CTRL_DATA, pos_low);
	io_out_byte(PORT_VGA_CRT_CTRL_ADDR, IDX_VGA_CRT_CTRL_ADDR_CUR_LOC_H);
	io_out_byte(PORT_VGA_CRT_CTRL_DATA, pos_high);
	sti();
}

/*
 # VGA roll up current screen to row 15
 */
void vga_roll_up_screen(void)
{
	/* Convert row & col to actual registers' values */
	uint32 pos = 0;
	uint8 pos_high = (uint8)((pos >> 8) & 0xff);
	uint8 pos_low = (uint8)(pos & 0xff);

	/* Set registers */
	cli();
	io_out_byte(PORT_VGA_CRT_CTRL_ADDR, IDX_VGA_CRT_CTRL_ADDR_START_ADDR_L);
	io_out_byte(PORT_VGA_CRT_CTRL_DATA, pos_low);
	io_out_byte(PORT_VGA_CRT_CTRL_ADDR, IDX_VGA_CRT_CTRL_ADDR_START_ADDR_H);
	io_out_byte(PORT_VGA_CRT_CTRL_DATA, pos_high);
	sti();
	
}


/*
 # VGA roll down current screen to row 0
 */
void vga_roll_down_screen(void)
{
	/* Convert row & col to actual registers' values */
	uint32 pos = 15 * COUNT_CRT_MAX_COL;
	uint8 pos_high = (uint8)((pos >> 8) & 0xff);
	uint8 pos_low = (uint8)(pos & 0xff);

	/* Set registers */
	cli();
	io_out_byte(PORT_VGA_CRT_CTRL_ADDR, IDX_VGA_CRT_CTRL_ADDR_START_ADDR_L);
	io_out_byte(PORT_VGA_CRT_CTRL_DATA, pos_low);
	io_out_byte(PORT_VGA_CRT_CTRL_ADDR, IDX_VGA_CRT_CTRL_ADDR_START_ADDR_H);
	io_out_byte(PORT_VGA_CRT_CTRL_DATA, pos_high);
	sti();
	
}

