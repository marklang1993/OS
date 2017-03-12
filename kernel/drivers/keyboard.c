#include "interrupt.h"
#include "io_port.h"
#include "buffer.h"
#include "lib.h"
#include "drivers/keyboard.h"

// Keymap related macros
#define KBMAP_ROWS		0x80
#define KBMAP_COLS		3
#define KBMAP_COL_RAW		0
#define KBMAP_COL_SHIFT		1
#define KBMAP_COL_E0		2

// Masks
#define KBMAP_BREAK_CODE	0x80
#define KBMAP_UNPRINT		0x100

// Unprintable keycode
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
#define KBC_INSERT		(45 | KBMAP_UNPRINT)
#define KBC_PAD_DOT		(46 | KBMAP_UNPRINT)
#define KBC_DELETE		(47 | KBMAP_UNPRINT)
#define KBC_F11			(48 | KBMAP_UNPRINT)
#define KBC_F12			(49 | KBMAP_UNPRINT)
#define KBC_GUI_L		(50 | KBMAP_UNPRINT)
#define KBC_GUI_R		(51 | KBMAP_UNPRINT)
#define KBC_APPS		(52 | KBMAP_UNPRINT)

// Keymap for US MF-2 keyboard.
// NOTE: This copies from Orange's, and small changes are made. 
// Format : |XX XX XX XX XX XX XX | XX |
//                  Flags          Data
uint32 keymap[KBMAP_ROWS * KBMAP_COLS] = {
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

// Scan & Make Code Buffer 
struct cbuf keyboard_buffer;

/*
 # Initialize Keyboard Driver
 */
void keyboard_init(void)
{
	cbuf_init(&keyboard_buffer);
}


/*
 # Keyboard Interrupt Handler
 */
void keyboard_interrupt_handler(void)
{
	uint8 data;

	// Read one code and save it to the buffer	
	io_in_byte(PORT_8042_BUF_R, &data);
	cbuf_write(&keyboard_buffer, data);
}

/*
 # Keyboard Re-enter Failed Interrupt Handler
 */
void keyboard_failed_interrupt_handler(void)
{
	uint8 status, data;
	
	do
	{
		// At least 1 scan code needs to be read
		io_in_byte(PORT_8042_BUF_R, &data);
		
		// Check status
		io_in_byte(PORT_8042_STAT_R, &status);
		status &= MASK_8042_OUT_BUF;
	}
	while(status != 0);	
		
	print_cstring_pos("*", 2, 8);
}

/*
 # Parse KeyCode
 */
rtc keyboard_getchar(char *ptr_data)
{
	uint8 keycode, status;
	uint32 data;
	rtc ret;
	
	// Read cbuf
	ret = cbuf_read(&keyboard_buffer, &keycode);
	if (EBUFEMP == ret)
	{
		/*
		// Check status
		io_in_byte(PORT_8042_STAT_R, &status);
		status &= MASK_8042_OUT_BUF;
		if (status != 0)
		{
			// Try to read 8042 again
			keyboard_interrupt_handler();
			ret = cbuf_read(&keyboard_buffer, &keycode);
			if (EBUFEMP == ret)
			{
				return ret;
			}
		}	
		*/
		return ret;
	}

	// Parse
	if (keycode < 0x80)
	{
		data = keymap[keycode * KBMAP_COLS];
		if(0 == (data & KBMAP_UNPRINT))
		{
			*ptr_data = (char)(data & 0xff);
			return OK;
		}
	}
	return EINVIDX;
}
