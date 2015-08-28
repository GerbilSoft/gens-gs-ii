/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * byteorder.h: System byte order enumeration.                             *
 * Indicates the system byteorder as detected by TEST_BIG_ENDIAN().        *
 *                                                                         *
 * Copyright (c) 2011-2015 by David Korth.                                 *
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

#ifndef __LIBZOMG_BYTEORDER_H__
#define __LIBZOMG_BYTEORDER_H__

#include "libcompat/byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ZOMG byteorder enumeration.
 * Used for memory blocks that might be byteswapped.
 */
typedef enum _ZomgByteorder_t {
	ZOMG_BYTEORDER_8,	// 8-bit
	ZOMG_BYTEORDER_16LE,	// 16-bit LE
	ZOMG_BYTEORDER_16BE,	// 16-bit BE
	ZOMG_BYTEORDER_32LE,	// 32-bit LE
	ZOMG_BYTEORDER_32BE,	// 32-bit BE

#if SYS_BYTEORDER == SYS_LIL_ENDIAN
	ZOMG_BYTEORDER_16H = ZOMG_BYTEORDER_16LE,	// 16-bit host-endian
	ZOMG_BYTEORDER_32H = ZOMG_BYTEORDER_32LE,	// 32-bit host-endian
#else /* SYS_BYTEORDER == SYS_BIG_ENDIAN */
	ZOMG_BYTEORDER_16H = ZOMG_BYTEORDER_16BE,	// 16-bit host-endian
	ZOMG_BYTEORDER_32H = ZOMG_BYTEORDER_32BE,	// 32-bit host-endian
#endif
} ZomgByteorder_t;

#ifdef __cplusplus
}
#endif

#endif /* __LIBZOMG_BYTEORDER_H__ */
