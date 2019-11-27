
#include <stdio.h>
#include "cstr_utils.h"

/**
 * @brief	Converts a signed integer into a string in base10.
 * @param	[in] i: Value to convert.
 * @param	[out] buf: Character array to write number string to.
 						Must be of size INT_BUF.`
 * @return	Pointer to start of number in the character array.
 *			Function writes to buffer from the last character so the 
 *			string visual fits wth how we view numbers.
 *			(largest decimal case to smallest).
 *			this means that the beginning of the buffer string may be
 *			invalid, so the returned pointer must be used to analyse
 *			the buffer data.
 * @details	This function is based on the itoa implementation found in
 			https://opensource.apple.com/source/groff/groff-10/groff/libgroff/itoa.c
 */
char *itoa(int i, char *str_buf)
{
	char* p = &str_buf[INT_BUF-1];	/* points to end of buffer '\0' */

	*p = '\0';

	if (i >= 0) {
		do {
		  *--p = '0' + (i % 10);
		  i /= 10;
		} while (i != 0);
	}
	else {			/* i < 0 */
		do {
		  *--p = '0' - (i % 10);
		  i /= 10;
		} while (i != 0);
		*--p = '-';
	}

	return p;
}

