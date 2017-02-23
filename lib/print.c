#include "lib.h"

#define MAX_ROW_COUNT	25
#define MAX_COL_COUNT	80

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
 # Convert 32-bit unsigned interger to string
 @ value: 32-bit unsigned integer
 @ str: pointer to a c-style char array
*/
void itoa(uint32 value, char *str)
{
	uint32 remainder;
	uint32 length = 0;
	
	do
	{
		remainder = value % 10;
		value = value / 10;
		str[length] = (char)(remainder + '0');		
		++length;
	} while(value != 0);

	str_reverse(str, length);
	str[length] = (char)0;		// Set end symbol
}

/*
 # Set cursor location
 @ row : row position (0 ~ 24)
 @ col : column position (0 ~ 79)
*/
void print_set_location(uint32 row, uint32 col)
{
	// Validate
	if (row < MAX_ROW_COUNT && col < MAX_COL_COUNT)
	{
		print_row = row;
		print_col = col;
	}
}

/*
 # Print a c-style string (end with "\0") & automatically move cursor position
 @ ptr_string : pointer to a c-style string
*/
void print_cstring(const char *ptr_string)
{
	uint32 length = 0;
	uint32 col_pos = print_col;
	uint32 row_pos = print_row;

	while(ptr_string[length] != 0)
	{
		++length;
	}	
	print_string(ptr_string, length, row_pos, col_pos);
	
	// Calculate new cursor position
	col_pos += length;
	if(col_pos >= MAX_COL_COUNT)
	{
		++row_pos;
		col_pos = col_pos % MAX_COL_COUNT;
	}
	if(row_pos >= MAX_ROW_COUNT)
	{
		// Wrap back to line 0
		row_pos = 0;
	}

	print_col = col_pos;
	print_row = row_pos;
}

/*
 # Print an 32-bit unsigned integer
 @ value: 32-bit unsigned integer
*/
void print_uint32(uint32 value)
{
	// 32-bit integer contains at most 8 chars, 9th char for '\0'.
	char value_str[9];

	itoa(value, value_str);
	print_cstring(value_str);
}


