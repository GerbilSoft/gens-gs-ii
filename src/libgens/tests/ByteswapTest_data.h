/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * ByteswapTest_data.c: ByteswapTest test data.                            *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __LIBGENS_TESTS_BYTESWAPTEST_DATA_H__
#define __LIBGENS_TESTS_BYTESWAPTEST_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Original test data.
 */
extern const uint8_t ByteswapTest_data_orig[516];

/**
 * 16-bit byteswapped test data.
 */
extern const uint8_t ByteswapTest_data_swap16[516];

/**
 * 32-bit byteswapped test data.
 */
extern const uint8_t ByteswapTest_data_swap32[516];

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_TESTS_BYTESWAPTEST_DATA_H__ */
