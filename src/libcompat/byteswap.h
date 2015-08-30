/***************************************************************************
 * libcompat: Compatibility library.                                       *
 * byteswap.h: Byteswapping functions.                                     *
 *                                                                         *
 * Copyright (c) 2008-2015 by David Korth                                  *
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

#ifndef __LIBCOMPAT_BYTESWAP_H__
#define __LIBCOMPAT_BYTESWAP_H__

/* Get the system byte order. */
#include "byteorder.h"

#include <stdint.h>

#define __swab16(x) (((x) << 8) | ((x) >> 8))

#define __swab32(x) \
	(((x) << 24) | ((x) >> 24) | \
		(((x) & 0x0000FF00UL) << 8) | \
		(((x) & 0x00FF0000UL) >> 8))

#if SYS_BYTEORDER == SYS_LIL_ENDIAN
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
#else /* SYS_BYTEORDER == SYS_BIG_ENDIAN */
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

/**
 * Address inversion flags for byteswapped addressing.
 * - U16DATA_U8_INVERT: Access U8 data in host-endian 16-bit data.
 * - U32DATA_U8_INVERT: Access U8 data in host-endian 32-bit data.
 * - U32DATA_U16_INVERT: Access U16 data in host-endian 32-bit data.
 */
#if SYS_BYTEORDER == SYS_LIL_ENDIAN
	#define U16DATA_U8_INVERT 1
	#define U32DATA_U8_INVERT 3
	#define U32DATA_U16_INVERT 1
#else /* SYS_BYTEORDER = SYS_BIG_ENDIAN */
	#define U16DATA_U8_INVERT 0
	#define U32DATA_U8_INVERT 0
	#define U32DATA_U16_INVERT 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 16-bit byteswap function.
 * @param ptr Pointer to array to swap. (MUST be 16-bit aligned!)
 * @param n Number of bytes to swap. (Must be divisible by 2; an extra odd byte will be ignored.)
 */
void __byte_swap_16_array(uint16_t *ptr, unsigned int n);

/**
 * 32-bit byteswap function.
 * @param ptr Pointer to array to swap. (MUST be 32-bit aligned!)
 * @param n Number of bytes to swap. (Must be divisible by 4; extra bytes will be ignored.)
 */
void __byte_swap_32_array(uint32_t *ptr, unsigned int n);

#ifdef __cplusplus
}
#endif

#endif /* __LIBCOMPAT_BYTESWAP_H__ */
