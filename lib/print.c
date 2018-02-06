#include "kheap.h"
#include "lib.h"
#include "drivers/vga.h"

// Cursor position (25*80)
static uint32 print_row = 0;
static uint32 print_col = 0;


/*
 # Reverse a string
 @ str: pointer to a char array
 @ length : size of this char array
*/
static void str_reverse(char *str, uint32 length)
{
	uint32 start = 0;
	uint32 end = length - 1;
	char tmp;
	
	while(start < end)
	{
		/* swap by xor:
		   A = A ^ B
		   B = A' ^ B = (A ^ B) ^ B = A ^ 0 = A
		   A = A' ^ B' = (A ^ B) ^ A = B ^ 0 = B
		*/
		//str[start] = (char)(str[start] ^ str[end]);
		//str[end] = (char)(str[start] ^ str[end]);
		//str[start] = (char)(str[start] ^ str[end]);
		
		tmp = str[start];
		str[start] = str[end];
		str[end] = tmp;

		++start;
		--end;
	}
}


/*
 # Get absolute value of a 32-bit signed integer
 @ n    : input number
 RETURN : absolute value of given signed integer
 */
uint32 abs(int32 n)
{
	uint32 abs_val = (uint32)n;

	if (n & 0x80000000) {
		abs_val = ~abs_val;
		++abs_val;
	}

	return abs_val;
}

/*
 # Convert 32-bit unsigned interger to string
 @ value : 32-bit unsigned integer
 @ str   : pointer to a c-style char array
 @ base  : base of value
*/
void itoa(uint32 value, char *str, int base)
{
	uint32 remainder;
	uint32 length = 0;
	
	do
	{
		remainder = value % base;
		value = value / base;
		if (remainder < 10) {
			str[length] = (char)(remainder + '0');

		} else {
			remainder -= 10;
			str[length] = (char)(remainder + 'a');
		}
		++length;
	} while(value != 0);

	str_reverse(str, length);
	str[length] = (char)0;		// Set end symbol
}

/*
 # Calculate the length of a c-style string
 @ str: pointer to a c-style char array
 */
uint32 strlen(const char *str)
{
	uint32 length = 0;
	while(str[length] != 0)
	{
		++length;
	}

	return length;
}

/*
 # Compare 2 c-style strings
 @ str1: string 1
 @ str2: string 2
 @ RETURN: 0 is same, others are different
 */
int32 strcmp(const char *str1, const char *str2)
{
	uint32 length1 = strlen(str1);
	uint32 length2 = strlen(str2);
	int32 diff = ((int32)length1) - ((int32)length2);
	int32 i;

	if (0 == diff) {
		/* same length */
		for (i = 0; i < length1; ++i)
		{
			if (str1[i] == str2[i]) {
				/* same character at i-th */
				continue;
			} else {
				/* different character at i-th */
				return ((int32)str1[i]) - ((int32)str2[i]);
			}
		}
		return 0;

	} else {
		/* different */
		return diff;
	}
}

/*
 # Copy a c-style string
 @ destination: dest. string
 @ source: src. string
 */
void strcpy(char *destination, const char *source)
{
	uint32 length = strlen(source);
	memcpy(destination, source, length);
}

/*
 # Find char in a string
 @ str      : src. string
 @ character: target character
 # RETURN   : pointer to the 1st occurance of that char
 */
const char* strchr(const char *str, int character)
{
	uint32 length = strlen(str);
	uint32 i;
	char a;

	for (i = 0; i < length; ++i)
	{
		a = *(str + i);
		if (a == character)
		{
			return (str + i);
		}
	}
	return NULL;
}

/*
 # Find char in a string
 @ str      : src. string
 @ character: target character
 # RETURN   : pointer to the last occurance of that char
 */
const char* strrchr(const char *str, int character)
{
	uint32 length = strlen(str);
	int32 i; /* i will be -1 in the following for-loop */
	char a;

	for (i = length - 1; i >= 0; --i)
	{
		a = *(str + i);
		if (a == character)
		{
			return (str + i);
		}
	}
	return NULL;
}

/*
 # Set cursor location
 @ row : row position (0 ~ 24)
 @ col : column position (0 ~ 79)
 */
void print_set_location(uint32 row, uint32 col)
{
	// Validate
	if (row < COUNT_CRT_MAX_ROW && col < COUNT_CRT_MAX_COL)
	{
		print_row = row;
		print_col = col;
	}
}

/*
 # Print a c-style string (end with "\0") with cursor position
 # NOTE: This is a THREAD-SAFE function.
 @ ptr_string : pointer to a c-style string
 @ row : row position (0 ~ 24)
 @ col : column position (0 ~ 79)
*/
void print_cstring_pos(const char *ptr_string, uint32 row, uint32 col)
{
	uint32 pos = 0;
	struct vga_char vga_str[1];

	while (ptr_string[pos] != 0) {
		vga_str[0].data = ptr_string[pos];
		vga_str[0].color.data = 0x0f;
		vga_write_screen(&row, &col, vga_str, 1);
		++pos;
	}
}

/*
 # Print a c-style string (end with "\0") & automatically move cursor position
 @ ptr_string : pointer to a c-style string
*/
void print_cstring(const char *ptr_string)
{
	uint32 pos = 0;
	struct vga_char vga_str[1];

	while (ptr_string[pos] != 0) {
		vga_str[0].data = ptr_string[pos];
		vga_str[0].color.data = 0x0f;
		vga_write_screen(&print_row, &print_col, vga_str, 1);
		++pos;
	}
}

/*
 # Print a 32-bit unsigned integer with cursor position
 # NOTE: This is a THREAD-SAFE function.
 @ value: 32-bit unsigned integer
 @ row : row position (0 ~ 24)
 @ col : column position (0 ~ 79)
*/
void print_uint32_pos(uint32 value, uint32 row, uint32 col)
{
	// 32-bit integer contains at most 8 chars, 9th char for '\0'.
	char value_str[9];

	itoa(value, value_str, 10);
	print_cstring_pos(value_str, row, col);
}

/*
 # Print a 32-bit unsigned integer
 @ value: 32-bit unsigned integer
*/
void print_uint32(uint32 value)
{
	// 32-bit integer contains at most 8 chars, 9th char for '\0'.
	char value_str[9];

	itoa(value, value_str, 10);
	print_cstring(value_str);
}


