#include "interrupt.h"
#include "io_port.h"
#include "buffer.h"
#include "drivers/keyboard.h"

/* Keymap related macros */
#define KB_BUF_CAPA		16

#define KBMAP_ROWS		0x80
#define KBMAP_COLS		3
#define KBMAP_COL_RAW		0
#define KBMAP_COL_SHIFT		1
#define KBMAP_COL_E0		2

#define KBMAP_PROCESS_PAD(keycode, charcode)		\
		case keycode:				\
			if (IS_TRUE(is_num_lock) && 	\
				IS_TRUE(is_make_code)) {\
				data = charcode;	\
			}				\
			break;				\

/* Keymap for US MF-2 keyboard.
 * NOTE: This is copied from Orange's, and small changes are made. 
 * Format : |XX XX XX XX XX XX XX | XX |
 *                  Flags          Data
 */
static uint32 keymap[KBMAP_ROWS * KBMAP_COLS] = {
/* scan-code			!Shift		Shift		E0 XX	*/
/* ==================================================================== */
/* 0x00 - none		*/	0,		0,		0,
/* 0x01 - ESC		*/	KBC_ESC,	KBC_ESC,	0,
/* 0x02 - '1'		*/	'1',		'!',		0,
/* 0x03 - '2'		*/	'2',		'@',		0,
/* 0x04 - '3'		*/	'3',		'#',		0,
/* 0x05 - '4'		*/	'4',		'$',		0,
/* 0x06 - '5'		*/	'5',		'%',		0,
/* 0x07 - '6'		*/	'6',		'^',		0,
/* 0x08 - '7'		*/	'7',		'&',		0,
/* 0x09 - '8'		*/	'8',		'*',		0,
/* 0x0A - '9'		*/	'9',		'(',		0,
/* 0x0B - '0'		*/	'0',		')',		0,
/* 0x0C - '-'		*/	'-',		'_',		0,
/* 0x0D - '='		*/	'=',		'+',		0,
/* 0x0E - BS		*/	KBC_BACKSPACE,	KBC_BACKSPACE,	0,
/* 0x0F - TAB		*/	KBC_TAB,	KBC_TAB,	0,
/* 0x10 - 'q'		*/	'q',		'Q',		0,
/* 0x11 - 'w'		*/	'w',		'W',		0,
/* 0x12 - 'e'		*/	'e',		'E',		0,
/* 0x13 - 'r'		*/	'r',		'R',		0,
/* 0x14 - 't'		*/	't',		'T',		0,
/* 0x15 - 'y'		*/	'y',		'Y',		0,
/* 0x16 - 'u'		*/	'u',		'U',		0,
/* 0x17 - 'i'		*/	'i',		'I',		0,
/* 0x18 - 'o'		*/	'o',		'O',		0,
/* 0x19 - 'p'		*/	'p',		'P',		0,
/* 0x1A - '['		*/	'[',		'{',		0,
/* 0x1B - ']'		*/	']',		'}',		0,
/* 0x1C - CR/LF		*/	KBC_ENTER,	KBC_ENTER,	KBC_PAD_ENTER,
/* 0x1D - l. Ctrl	*/	KBC_CTRL_L,	KBC_CTRL_L,	KBC_CTRL_R,
/* 0x1E - 'a'		*/	'a',		'A',		0,
/* 0x1F - 's'		*/	's',		'S',		0,
/* 0x20 - 'd'		*/	'd',		'D',		0,
/* 0x21 - 'f'		*/	'f',		'F',		0,
/* 0x22 - 'g'		*/	'g',		'G',		0,
/* 0x23 - 'h'		*/	'h',		'H',		0,
/* 0x24 - 'j'		*/	'j',		'J',		0,
/* 0x25 - 'k'		*/	'k',		'K',		0,
/* 0x26 - 'l'		*/	'l',		'L',		0,
/* 0x27 - ';'		*/	';',		':',		0,
/* 0x28 - '\''		*/	'\'',		'"',		0,
/* 0x29 - '`'		*/	'`',		'~',		0,
/* 0x2A - l. SHIFT	*/	KBC_SHIFT_L,	KBC_SHIFT_L,	0,
/* 0x2B - '\'		*/	'\\',		'|',		0,
/* 0x2C - 'z'		*/	'z',		'Z',		0,
/* 0x2D - 'x'		*/	'x',		'X',		0,
/* 0x2E - 'c'		*/	'c',		'C',		0,
/* 0x2F - 'v'		*/	'v',		'V',		0,
/* 0x30 - 'b'		*/	'b',		'B',		0,
/* 0x31 - 'n'		*/	'n',		'N',		0,
/* 0x32 - 'm'		*/	'm',		'M',		0,
/* 0x33 - ','		*/	',',		'<',		0,
/* 0x34 - '.'		*/	'.',		'>',		0,
/* 0x35 - '/'		*/	'/',		'?',		KBC_PAD_SLASH,
/* 0x36 - r. SHIFT	*/	KBC_SHIFT_R,	KBC_SHIFT_R,	0,
/* 0x37 - '*'		*/	'*',		'*',    	0,
/* 0x38 - ALT		*/	KBC_ALT_L,	KBC_ALT_L,  	KBC_ALT_R,
/* 0x39 - ' '		*/	' ',		' ',		0,
/* 0x3A - CapsLock	*/	KBC_CAPS_LOCK,	KBC_CAPS_LOCK,	0,
/* 0x3B - F1		*/	KBC_F1,		KBC_F1,		0,
/* 0x3C - F2		*/	KBC_F2,		KBC_F2,		0,
/* 0x3D - F3		*/	KBC_F3,		KBC_F3,		0,
/* 0x3E - F4		*/	KBC_F4,		KBC_F4,		0,
/* 0x3F - F5		*/	KBC_F5,		KBC_F5,		0,
/* 0x40 - F6		*/	KBC_F6,		KBC_F6,		0,
/* 0x41 - F7		*/	KBC_F7,		KBC_F7,		0,
/* 0x42 - F8		*/	KBC_F8,		KBC_F8,		0,
/* 0x43 - F9		*/	KBC_F9,		KBC_F9,		0,
/* 0x44 - F10		*/	KBC_F10,	KBC_F10,	0,
/* 0x45 - NumLock	*/	KBC_NUM_LOCK,	KBC_NUM_LOCK,	0,
/* 0x46 - ScrLock	*/	KBC_SCROLL_LOCK,KBC_SCROLL_LOCK,0,
/* 0x47 - Home		*/	KBC_PAD_HOME,	'7',		KBC_HOME,
/* 0x48 - CurUp		*/	KBC_PAD_UP,	'8',		KBC_UP,
/* 0x49 - PgUp		*/	KBC_PAD_PAGEUP,	'9',		KBC_PAGEUP,
/* 0x4A - '-'		*/	KBC_PAD_MINUS,	'-',		0,
/* 0x4B - Left		*/	KBC_PAD_LEFT,	'4',		KBC_LEFT,
/* 0x4C - MID		*/	KBC_PAD_MID,	'5',		0,
/* 0x4D - Right		*/	KBC_PAD_RIGHT,	'6',		KBC_RIGHT,
/* 0x4E - '+'		*/	KBC_PAD_PLUS,	'+',		0,
/* 0x4F - End		*/	KBC_PAD_END,	'1',		KBC_END,
/* 0x50 - Down		*/	KBC_PAD_DOWN,	'2',		KBC_DOWN,
/* 0x51 - PgDown	*/	KBC_PAD_PAGEDOWN,'3',		KBC_PAGEDOWN,
/* 0x52 - Insert	*/	KBC_PAD_INS,	'0',		KBC_INSERT,
/* 0x53 - Delete	*/	KBC_PAD_DOT,	'.',		KBC_DELETE,
/* 0x54 - Enter		*/	0,		0,		0,
/* 0x55 - ???		*/	0,		0,		0,
/* 0x56 - ???		*/	0,		0,		0,
/* 0x57 - F11		*/	KBC_F11,	KBC_F11,	0,	
/* 0x58 - F12		*/	KBC_F12,	KBC_F12,	0,	
/* 0x59 - ???		*/	0,		0,		0,	
/* 0x5A - ???		*/	0,		0,		0,	
/* 0x5B - ???		*/	0,		0,		KBC_GUI_L,	
/* 0x5C - ???		*/	0,		0,		KBC_GUI_R,	
/* 0x5D - ???		*/	0,		0,		KBC_APPS,	
/* 0x5E - ???		*/	0,		0,		0,	
/* 0x5F - ???		*/	0,		0,		0,
/* 0x60 - ???		*/	0,		0,		0,
/* 0x61 - ???		*/	0,		0,		0,	
/* 0x62 - ???		*/	0,		0,		0,	
/* 0x63 - ???		*/	0,		0,		0,	
/* 0x64 - ???		*/	0,		0,		0,	
/* 0x65 - ???		*/	0,		0,		0,	
/* 0x66 - ???		*/	0,		0,		0,	
/* 0x67 - ???		*/	0,		0,		0,	
/* 0x68 - ???		*/	0,		0,		0,	
/* 0x69 - ???		*/	0,		0,		0,	
/* 0x6A - ???		*/	0,		0,		0,	
/* 0x6B - ???		*/	0,		0,		0,	
/* 0x6C - ???		*/	0,		0,		0,	
/* 0x6D - ???		*/	0,		0,		0,	
/* 0x6E - ???		*/	0,		0,		0,	
/* 0x6F - ???		*/	0,		0,		0,	
/* 0x70 - ???		*/	0,		0,		0,	
/* 0x71 - ???		*/	0,		0,		0,	
/* 0x72 - ???		*/	0,		0,		0,	
/* 0x73 - ???		*/	0,		0,		0,	
/* 0x74 - ???		*/	0,		0,		0,	
/* 0x75 - ???		*/	0,		0,		0,	
/* 0x76 - ???		*/	0,		0,		0,	
/* 0x77 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x78 - ???		*/	0,		0,		0,	
/* 0x7A - ???		*/	0,		0,		0,	
/* 0x7B - ???		*/	0,		0,		0,	
/* 0x7C - ???		*/	0,		0,		0,	
/* 0x7D - ???		*/	0,		0,		0,	
/* 0x7E - ???		*/	0,		0,		0,
/* 0x7F - ???		*/	0,		0,		0
};

/* Internal Variables */
static struct cbuf keyboard_buffer;	/* Scan & Make code buffer */
static BOOL is_shift_r = FALSE;		/* Shift-Right flag */
static BOOL is_shift_l = FALSE;		/* Shift-Left flag */
static BOOL is_ctrl_r = FALSE;		/* Ctrl-Right flag */
static BOOL is_ctrl_l = FALSE;		/* Ctrl-Left flag */
static BOOL is_alt_r = FALSE;		/* Alt-Right flag */
static BOOL is_alt_l = FALSE;		/* Alt-Left flag */
static BOOL is_caps_lock = FALSE;	/* Caps Lock flag */
static BOOL is_num_lock = FALSE;	/* Number Lock flag */
static BOOL is_scroll_lock = FALSE;	/* Scroll Lock flag */
static BOOL is_e0 = FALSE;		/* Keycode E0 flag */
static BOOL is_e1 = FALSE;		/* Keycode E1 flag */


/*
 # Initialize Keyboard Driver
 */
void keyboard_init(void)
{
	cbuf_init(&keyboard_buffer, KB_BUF_CAPA);
}


/*
 # Keyboard Interrupt Handler
 */
void keyboard_interrupt_handler(void)
{
	uint8 keycode;

	/* Read one code and save it to the buffer */
	io_in_byte(PORT_8042_BUF_R, &keycode);
	cbuf_write(&keyboard_buffer, (uint32)keycode);
}


/*
 # Parse KeyCode
 */
rtc keyboard_getchar(uint32 *ptr_data)
{
	uint32 keycode;
	uint32 data_index, data;
	BOOL is_make_code;
	BOOL is_shift;
	rtc ret;
	
	/* # Read cbuf */
	ret = cbuf_read(&keyboard_buffer, &keycode);
	if (EBUFEMP == ret) {
		return ret;
	}

	/* # Parse */
	/* Less than 0x80 -> make code */
	is_make_code = (keycode < KBMAP_BREAK_CODE) ? TRUE : FALSE;

	/* #1 Key code == E0 or E1 */
	if (0xe0 == keycode) {
		is_e0 = TRUE;
		return EINVIDX;

	} else if (0xe1 == keycode) {
		is_e1 = TRUE;
		return EINVIDX;

	}

	/* #2 Key code == E0 or E1 pressed */
	if (IS_TRUE(is_e0)) {
		/* Process key data */
		data_index = (keycode & 0x7f) * KBMAP_COLS + KBMAP_COL_E0;
		data = keymap[data_index];

		if(0 == (data & KBMAP_UNPRINT)) {
			/* Printable characters */
			if (IS_TRUE(is_make_code)) {
				data = data & 0xff;

			} else {
				/* Do not print break code */
				data = (data & 0xff) | KBMAP_BREAK_CODE;
			}

		} else {

			/* Unprintable characters */
			switch (data) {

			case KBC_CTRL_R:
				is_ctrl_r = IS_TRUE(is_make_code) ? TRUE : FALSE;
				break;

			case KBC_ALT_R:
				is_alt_r = IS_TRUE(is_make_code) ? TRUE : FALSE;
				break;

			case KBC_PAD_SLASH:
				if (IS_TRUE(is_make_code)) {
					data = '/';
				}
				break;

			}

			/* Process break code */
			data |= IS_TRUE(is_make_code) ? 0 : KBMAP_BREAK_CODE;
		}

		is_e0 = FALSE;
		*ptr_data = data;
		return OK;

	} else if (IS_TRUE(is_e1)) {

		is_e1 = FALSE;
		return EINVIDX;
	}

	/* #3 Other key code */
	/* Shift & Caps Lock */
	is_shift = is_shift_r | is_shift_l;
	is_shift = IS_TRUE(is_caps_lock) ? NOT(is_shift) : is_shift;
	/* Process key data */
	data_index = (keycode & 0x7f) * KBMAP_COLS +
		(IS_TRUE(is_shift) ? KBMAP_COL_SHIFT : KBMAP_COL_RAW);
	data = keymap[data_index];

	if(0 == (data & KBMAP_UNPRINT)) {
		/* Printable characters */
		if (IS_TRUE(is_make_code)) {
			data = data & 0xff;

		} else {
			/* Do not print break code */
			data = (data & 0xff) | KBMAP_BREAK_CODE;
		}

	} else {
		/* Unprintable characters */
		switch (data) {

		case KBC_SHIFT_R:
			is_shift_r = IS_TRUE(is_make_code) ? TRUE : FALSE;
			break;

		case KBC_SHIFT_L:
			is_shift_l = IS_TRUE(is_make_code) ? TRUE : FALSE;
			break;

		case KBC_CAPS_LOCK:
			if (IS_TRUE(NOT(is_make_code))) {
				/* Change flag once the key pops up */
				is_caps_lock = NOT(is_caps_lock);
			}
			break;

		case KBC_NUM_LOCK:
			if (IS_TRUE(NOT(is_make_code))) {
				/* Change flag once the key pops up */
				is_num_lock = NOT(is_num_lock);
			}
			break;

		case KBC_SCROLL_LOCK:
			if (IS_TRUE(NOT(is_make_code))) {
				/* Change flag once the key pops up */
				is_scroll_lock = NOT(is_scroll_lock);
			}
			break;

		case KBC_CTRL_L:
			is_ctrl_l = IS_TRUE(is_make_code) ? TRUE : FALSE;
			break;

		case KBC_ALT_L:
			is_alt_l = IS_TRUE(is_make_code) ? TRUE : FALSE;
			break;

		KBMAP_PROCESS_PAD(KBC_PAD_HOME, '7')
		KBMAP_PROCESS_PAD(KBC_PAD_UP, '8')
		KBMAP_PROCESS_PAD(KBC_PAD_PAGEUP, '9')
		KBMAP_PROCESS_PAD(KBC_PAD_MINUS, '-')
		KBMAP_PROCESS_PAD(KBC_PAD_LEFT, '4')
		KBMAP_PROCESS_PAD(KBC_PAD_MID, '5')
		KBMAP_PROCESS_PAD(KBC_PAD_RIGHT, '6')
		KBMAP_PROCESS_PAD(KBC_PAD_PLUS, '+')
		KBMAP_PROCESS_PAD(KBC_PAD_END, '1')
		KBMAP_PROCESS_PAD(KBC_PAD_DOWN, '2')
		KBMAP_PROCESS_PAD(KBC_PAD_PAGEDOWN, '3')
		KBMAP_PROCESS_PAD(KBC_PAD_INS, '0')
		KBMAP_PROCESS_PAD(KBC_PAD_DOT, '.')
		}

		/* Process break code */
		data |= IS_TRUE(is_make_code) ? 0 : KBMAP_BREAK_CODE;
	}

	*ptr_data = data;
	return OK;
}
