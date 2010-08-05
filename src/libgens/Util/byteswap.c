/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * byteswap.h: Byteswapping functions.                                     *
 *                                                                         *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#include "byteswap.h"

/**
 * __byte_swap_16_array(): 16-bit byteswap function.
 * @param Pointer to array to swap.
 * @param n Number of bytes to swap. (Must be divisible by 2; an extra odd byte will be ignored.)
 */
void __byte_swap_16_array(void *ptr, unsigned int n)
{
	unsigned char *cptr = (unsigned char*)ptr;
	unsigned char x;
	
	// Don't allow uneven lengths.
	n &= ~1;
	
	// TODO: Add an x86-optimized version, possibly using SSE.
	for (; n != 0; n -= 2, cptr += 2)
	{
		x = *cptr;
		*cptr = *(cptr + 1);
		*(cptr + 1) = x;
	}
}

/**
 * __byte_swap_32_array(): 32-bit byteswap function.
 * @param Pointer to array to swap.
 * @param n Number of bytes to swap. (Must be divisible by 4; extra bytes will be ignored.)
 */
void __byte_swap_32_array(void *ptr, unsigned int n)
{
	unsigned char *cptr = (unsigned char*)ptr;
	unsigned char x, y;
	
	// Don't allow lengths that aren't divisible by 4.
	n &= ~3;
	
	// TODO: Add an x86-optimized version using bswap and/or SSE.
	for (; n != 0; n -= 4, cptr += 4)
	{
		x = *cptr;
		y = *(cptr + 1);
		
		*cptr = (*cptr + 3);
		*(cptr + 1) = (*cptr + 2);
		*(cptr + 2) = y;
		*(cptr + 3) = x;
	}
}
