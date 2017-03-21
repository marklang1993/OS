#include "interrupt.h"
#include "io_port.h"
#include "lib.h"
#include "drivers/vga.h"

/* VGA Ports */
#define PORT_VGA_CRT_CTRL_ADDR                  0x3d4
#define PORT_VGA_CRT_CTRL_DATA                  0x3d5

#define IDX_VGA_CRT_CTRL_ADDR_START_ADDR_H      0x0c
#define IDX_VGA_CRT_CTRL_ADDR_START_ADDR_L      0x0d
#define IDX_VGA_CRT_CTRL_ADDR_CUR_LOC_H         0x0e
#define IDX_VGA_CRT_CTRL_ADDR_CUR_LOC_L         0x0f

/* VGA Memory */
#define VGA_MEMORY_ADDR				0xb8000
#define VGA_MEMORY_LIMIT			0x8000


/*
 # VGA Set Cursor Location
 @ row:	cursor row location
 @ col: cursor column location
 */
void vga_set_cursor_location(uint32 row, uint32 col)
{
	/* Convert row & col to actual registers' values */
	uint32 pos = VGA_GET_POS(row, col);
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
 # VGA set current screen to indicated row
 @ row : target row position
 */
void vga_set_screen_row(uint32 row)
{
	/* Convert row & col to actual registers' values */
	uint32 pos = VGA_GET_POS(row, 0);
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
 # VGA get current screen row
 @ ptr_row : pointer to current row position
 */
void vga_get_screen_row(uint32 *ptr_row)
{
	/* Convert row & col to actual registers' values */
	uint32 pos;
	uint8 pos_high;
	uint8 pos_low;

	/* Set registers */
	cli();
	io_out_byte(PORT_VGA_CRT_CTRL_ADDR, IDX_VGA_CRT_CTRL_ADDR_START_ADDR_L);
	io_in_byte(PORT_VGA_CRT_CTRL_DATA, &pos_low);
	io_out_byte(PORT_VGA_CRT_CTRL_ADDR, IDX_VGA_CRT_CTRL_ADDR_START_ADDR_H);
	io_in_byte(PORT_VGA_CRT_CTRL_DATA, &pos_high);
	sti();

	/* Calculate */
	pos = (((uint32)pos_high) << 8) + pos_low;
	*ptr_row = VGA_GET_ROW(pos);
}


/*
 # VGA roll up current screen
 */
void vga_roll_up_screen(void)
{
	uint32 current_row;

	vga_get_screen_row(&current_row);
	if (current_row != 0) {
		current_row -= 1;
		vga_set_screen_row(current_row);
	}
}


/*
 # VGA roll down current screen
 */
void vga_roll_down_screen(void)
{
	uint32 current_row;

	vga_get_screen_row(&current_row);
	if (current_row != 50) {
		current_row += 1;
		vga_set_screen_row(current_row);
	}
}

/*
 # VGA memory opeartion parameters calculation
 @ ptr_row	: pointer to absolute row position (in/out)
 @ ptr_col	: pointer to absolute col position (in/out)
 @ count	: count of characters
 @ ptr_mem_pos	: pointer to mem_pos
 @ ptr_mem_len	: pointer to mem_len
 */
static void vga_mem_calculate(
	uint32 *ptr_row,
	uint32 *ptr_col,
	uint32 count,
	uint32 *ptr_mem_pos,
	uint32 *ptr_mem_len
)
{
	uint32 mem_pos = VGA_GET_POS(*ptr_row, *ptr_col) * 2 + VGA_MEMORY_ADDR;	/* Upper bound */
	uint32 mem_limit = mem_pos + count * 2;					/* Lower bound */
	uint32 mem_video_limit = VGA_MEMORY_ADDR + VGA_MEMORY_LIMIT;
	uint32 mem_len = count * 2;
	uint32 new_pos;

	/* Memory boundary check */
	if (mem_pos > mem_video_limit) {
		/* Upper bound address is beyond the boundary */
		mem_len = 0;

	} else if (mem_limit > mem_video_limit) {
		/* Lower bound address is beyond the boundary */
		mem_len = mem_video_limit - mem_pos;
	}

	/* Return values */
	*ptr_mem_pos = mem_pos;
	*ptr_mem_len = mem_len;

	/* New position */
	new_pos = VGA_GET_POS(*ptr_row, *ptr_col) + (mem_len / 2);
	*ptr_row = VGA_GET_ROW(new_pos);
	*ptr_col = VGA_GET_COL(new_pos);
}


/*
 # VGA write video memory
 @ ptr_row	: pointer to absolute row position (in/out)
 @ ptr_col	: pounter to absolute col position (in/out)
 @ ptr_buf	: raw data to be written
 @ count	: count of characters
 * RETURN	: count of characters have been written
 */
uint32 vga_write_screen(
	uint32 *ptr_row,
	uint32 *ptr_col,
	const struct vga_char *ptr_buf,
	uint32 count
)
{
	uint32 mem_pos, mem_len;
	vga_mem_calculate(ptr_row, ptr_col, count, &mem_pos, &mem_len);

	memcpy((void *)mem_pos, (const void *)ptr_buf, mem_len);
	return (mem_len / 2);
}


/*
 # VGA read video memory
 @ row		: absolute row position
 @ col		: absolute col position
 @ ptr_buf	: place to store raw data
 @ count	: count of characters
 * RETURN	: count of characters have been read
 */
uint32 vga_read_screen(
	uint32 row,
	uint32 col,
	struct vga_char *ptr_buf,
	uint32 count
)
{
	uint32 mem_pos, mem_len;
	vga_mem_calculate(&row, &col, count, &mem_pos, &mem_len);

	memcpy((void *)ptr_buf, (const void *)mem_pos, mem_len);
	return (mem_len / 2);
}


/*
 # Convert c-style string to vga string
 @ vga_str	: Output vga string
 @ cstr		: Input c-style string
 * RETURN	: Count of converted characters
 */
uint32 cstr_to_vga_str(struct vga_char *vga_str, const char *cstr)
{
	uint32 i = 0;

	// Convert
	while (*(cstr + i) != 0) {
		(vga_str + i)->data = *(cstr + i);
		(vga_str + i)->color.data = 0x0f;	/* Back: Black; Char: White */
		++i;
	}

	// Add end mark
	(vga_str + i)->data = 0;
	(vga_str + i)->color.data = 0;

	return i;
}


/*
 # Convert vga string to c-style string
 @ cstr		: Output c-style string
 @ vga_str	: Input vga string
 * RETURN	: Count of converted characters
 */
uint32 vga_str_to_cstr(char *cstr, const struct vga_char *vga_str)
{
	uint32 i = 0;

	// Convert
	while ((vga_str + i)->data != 0) {
		*(cstr + i) = (vga_str + i)->data;
		++i;
	}

	// Add end mark
	*(cstr + i) = 0;
	
	return i;
}

