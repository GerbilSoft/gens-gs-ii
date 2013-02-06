/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * VdpSpriteMaskingTest_data.h: Sprite Masking & Overflow Test ROM.        *
 *                                                                         *
 * Copyright (c) 2011-2013 by David Korth.                                 *
 * Original ROM Copyright (c) 2009 by Nemesis.                             *
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

#ifndef __LIBGENS_TESTS_VDPSPRITEMASKINGTEST_DATA_H__
#define __LIBGENS_TESTS_VDPSPRITEMASKINGTEST_DATA_H__

// C includes.
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * VRAM for H32 (256x224) sprite masking test.
 * Contents are GZipped and must be decoded using zlib.
 */
extern const uint8_t test_spritemask_vram_h32[1452];

/**
 * VRAM for H40 (320x224) sprite masking test.
 * Contents are GZipped and must be decoded using zlib.
 */
extern const uint8_t test_spritemask_vram_h40[1476];

/**
 * CRAM for all tests. (host-endian)
 */
extern const uint16_t test_spritemask_cram[64];

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_TESTS_VDPSPRITEMASKINGTEST_DATA_H__ */
