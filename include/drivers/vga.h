#ifndef _DRV_VGA_H_
#define _DRV_VGA_H_

#include "type.h"

/* Screen Range : 25*80 */
#define COUNT_CRT_MAX_ROW			25
#define COUNT_CRT_MAX_COL			80


/* VGA Functions */
void vga_set_cursor_location(uint32 row, uint32 col);
void vga_roll_up_screen(void);
void vga_roll_down_screen(void);

#endif
