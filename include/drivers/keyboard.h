#ifndef _DRV_KEYBOARD_H_
#define _DRV_KEYBOARD_H_

#include "errors.h"
#include "type.h"

/* Keyboard Map Masks */
#define KBMAP_BREAK_CODE	0x80
#define KBMAP_UNPRINT		0x100

/* Unprintable keycode */
#define KBC_ESC			(0 | KBMAP_UNPRINT)
#define KBC_BACKSPACE		(1 | KBMAP_UNPRINT)
#define KBC_TAB			(2 | KBMAP_UNPRINT)
#define KBC_ENTER		(3 | KBMAP_UNPRINT)
#define KBC_PAD_ENTER		(4 | KBMAP_UNPRINT)
#define KBC_CTRL_L		(5 | KBMAP_UNPRINT)
#define KBC_CTRL_R		(6 | KBMAP_UNPRINT)
#define KBC_SHIFT_L		(7 | KBMAP_UNPRINT)
#define KBC_PAD_SLASH		(8 | KBMAP_UNPRINT)
#define KBC_SHIFT_R		(9 | KBMAP_UNPRINT)
#define KBC_ALT_L		(10 | KBMAP_UNPRINT)
#define KBC_ALT_R		(11 | KBMAP_UNPRINT)
#define KBC_CAPS_LOCK		(12 | KBMAP_UNPRINT)
#define KBC_F1			(13 | KBMAP_UNPRINT)
#define KBC_F2			(14 | KBMAP_UNPRINT)
#define KBC_F3			(15 | KBMAP_UNPRINT)
#define KBC_F4			(16 | KBMAP_UNPRINT)
#define KBC_F5			(17 | KBMAP_UNPRINT)
#define KBC_F6			(18 | KBMAP_UNPRINT)
#define KBC_F7			(19 | KBMAP_UNPRINT)
#define KBC_F8			(20 | KBMAP_UNPRINT)
#define KBC_F9			(21 | KBMAP_UNPRINT)
#define KBC_F10			(22 | KBMAP_UNPRINT)
#define KBC_NUM_LOCK		(23 | KBMAP_UNPRINT)
#define KBC_SCROLL_LOCK		(24 | KBMAP_UNPRINT)
#define KBC_PAD_HOME		(25 | KBMAP_UNPRINT)
#define KBC_HOME		(26 | KBMAP_UNPRINT)
#define KBC_PAD_UP		(27 | KBMAP_UNPRINT)
#define KBC_UP			(28 | KBMAP_UNPRINT)
#define KBC_PAD_PAGEUP		(29 | KBMAP_UNPRINT)
#define KBC_PAGEUP		(30 | KBMAP_UNPRINT)
#define KBC_PAD_MINUS		(31 | KBMAP_UNPRINT)
#define KBC_PAD_LEFT		(32 | KBMAP_UNPRINT)
#define KBC_LEFT		(33 | KBMAP_UNPRINT)
#define KBC_PAD_MID		(34 | KBMAP_UNPRINT)
#define KBC_PAD_RIGHT		(35 | KBMAP_UNPRINT)
#define KBC_RIGHT		(36 | KBMAP_UNPRINT)
#define KBC_PAD_PLUS		(37 | KBMAP_UNPRINT)
#define KBC_PAD_END		(38 | KBMAP_UNPRINT)
#define KBC_END			(39 | KBMAP_UNPRINT)
#define KBC_PAD_DOWN		(40 | KBMAP_UNPRINT)
#define KBC_DOWN		(41 | KBMAP_UNPRINT)
#define KBC_PAD_PAGEDOWN	(42 | KBMAP_UNPRINT)
#define KBC_PAGEDOWN		(43 | KBMAP_UNPRINT)
#define KBC_PAD_INS		(44 | KBMAP_UNPRINT)
#define KBC_INSERT              (45 | KBMAP_UNPRINT)
#define KBC_PAD_DOT             (46 | KBMAP_UNPRINT)
#define KBC_DELETE              (47 | KBMAP_UNPRINT)
#define KBC_F11                 (48 | KBMAP_UNPRINT)
#define KBC_F12                 (49 | KBMAP_UNPRINT)
#define KBC_GUI_L               (50 | KBMAP_UNPRINT)
#define KBC_GUI_R               (51 | KBMAP_UNPRINT)
#define KBC_APPS                (52 | KBMAP_UNPRINT)

/* Keyboard Functions */
void keyboard_interrupt_handler(void);

void keyboard_init(void);
void keyboard_uninit(void);
rtc keyboard_getchar(uint32 *ptr_data);

#endif
