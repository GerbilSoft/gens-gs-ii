/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * gens_iconv.c: iconv() wrapper.                                          *
 *                                                                         *
 * Copyright (c) 2009-2010 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "gens_iconv.h"

// C includes.
#include <stdlib.h>

// iconv includes.
#include <iconv.h>


/**
 * gens_iconv(): Convert a string from one character set to another.
 * @param src 		[in] Source string.
 * @param src_bytes_len [in] Source length, in bytes.
 * @param src_charset	[in] Source character set.
 * @param dest_charset	[in] Destination character set.
 * @return malloc()'d UTF-8 string, or NULL on error.
 */
char *gens_iconv(const char *src, size_t src_bytes_len,
		 const char *src_charset, const char *dest_charset)
{
	if (!src || src_bytes_len == 0)
		return "";
	
	if (!src_charset)
		src_charset = "";
	if (!dest_charset)
		dest_charset = "";
	
	// Based on examples from:
	// * http://www.delorie.com/gnu/docs/glibc/libc_101.html
	// * http://www.codase.com/search/call?name=iconv
	
	// Open an iconv descriptor.
	iconv_t cd;
	cd = iconv_open(dest_charset, src_charset);
	if (cd == (iconv_t)(-1))
	{
		// Error opening iconv.
		return NULL;
	}
	
	// Allocate the output buffer.
	// UTF-8 is variable length, and the largest UTF-8 character is 4 bytes long.
	const size_t out_bytes_len = (src_bytes_len * 4) + 1;
	size_t out_bytes_remaining = out_bytes_len;
	char *outbuf = (char*)malloc(out_bytes_len);
	
	// Input and output pointers.
	char *inptr = (char*)(src);	// Input pointer.
	char *outptr = &outbuf[0];	// Output pointer.
	
	int success = 1;
	
	while (src_bytes_len > 0)
	{
		if (iconv(cd, &inptr, &src_bytes_len, &outptr, &out_bytes_remaining) == (size_t)(-1))
		{
			// An error occurred while converting the string.
			success = 0;
			break;
		}
	}
	
	// Close the iconv descriptor.
	iconv_close(cd);
	
	if (success)
	{
		// The string was converted successfully.
		
		// Make sure the string is null-terminated.
		if (out_bytes_remaining > 0)
			*outptr = 0x00;
		
		// Return the output buffer.
		return outbuf;
	}
	
	// The string was not converted successfully.
	free(outbuf);
	return NULL;
}
