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

#ifndef __LIBGENS_UTIL_BYTESWAP_H__
#define __LIBGENS_UTIL_BYTESWAP_H__

#include <stdint.h>

// Endianness defines ported from libsdl.
#define GENS_LIL_ENDIAN 1234
#define GENS_BIG_ENDIAN 4321
#ifndef GENS_BYTEORDER
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__powerpc__) || \
    defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__SPARC__)
#define GENS_BYTEORDER GENS_BIG_ENDIAN
#else
#define GENS_BYTEORDER GENS_LIL_ENDIAN
#endif
#endif

#define __swab16(x) (((x) << 8) | ((x) >> 8))

#define __swab32(x) \
	(((x) << 24) | ((x) >> 24) | \
		((x & 0x0000FF00UL) << 8) | \
		((x & 0x00FF0000UL) >> 8))

#if GENS_BYTEORDER == GENS_LIL_ENDIAN
	#define be16_to_cpu_array(ptr, n)	__byte_swap_16_array((ptr), (n));
	#define le16_to_cpu_array(ptr, n)
	#define be32_to_cpu_array(ptr, n)	__byte_swap_32_array((ptr), (n));
	#define le32_to_cpu_array(ptr, n)
	
	#define cpu_to_be16_array(ptr, n)	__byte_swap_16_array((ptr), (n));
	#define cpu_to_le16_array(ptr, n)
	#define cpu_to_be32_array(ptr, n)	__byte_swap_32_array((ptr), (n));
	#define cpu_to_le32_array(ptr, n)
	
	#define be16_to_cpu(x)	__swab16(x)
	#define be32_to_cpu(x)	__swab32(x)
	#define le16_to_cpu(x)	(x)
	#define le32_to_cpu(x)	(x)
	
	#define cpu_to_be16(x)	__swab16(x)
	#define cpu_to_be32(x)	__swab32(x)
	#define cpu_to_le16(x)	(x)
	#define cpu_to_le32(x)	(x)
#else /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
	#define be16_to_cpu_array(ptr, n)
	#define le16_to_cpu_array(ptr, n)	__byte_swap_16_array((ptr), (n));
	#define be32_to_cpu_array(ptr, n)
	#define le32_to_cpu_array(ptr, n)	__byte_swap_32_array((ptr), (n));
	
	#define cpu_to_be16_array(ptr, n)
	#define cpu_to_le16_array(ptr, n)	__byte_swap_16_array((ptr), (n));
	#define cpu_to_be32_array(ptr, n)
	#define cpu_to_le32_array(ptr, n)	__byte_swap_32_array((ptr), (n));
	
	#define be16_to_cpu(x)	(x)
	#define be32_to_cpu(x)	(x)
	#define le16_to_cpu(x)	__swab16(x)
	#define le32_to_cpu(x)	__swab32(x)
	
	#define cpu_to_be16(x)	(x)
	#define cpu_to_be32(x)	(x)
	#define cpu_to_le16(x)	__swab16(x)
	#define cpu_to_le32(x)	__swab32(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * __byte_swap_16_array(): 16-bit byteswap function.
 * @param Pointer to array to swap.
 * @param n Number of bytes to swap. (Must be divisible by 2; an extra odd byte will be ignored.)
 */
void __byte_swap_16_array(void *ptr, unsigned int n);

/**
 * __byte_swap_32_array(): 32-bit byteswap function.
 * @param Pointer to array to swap.
 * @param n Number of bytes to swap. (Must be divisible by 4; extra bytes will be ignored.)
 */
void __byte_swap_32_array(void *ptr, unsigned int n);

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_UTIL_BYTESWAP_H__ */
