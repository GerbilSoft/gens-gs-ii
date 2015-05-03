/***************************************************************************
 * libgens/tests: Gens Emulation Library. (Test Suite)                     *
 * VdpFIFOTesting_data.h: VDP FIFO Test ROM.                               *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
 * Original ROM Copyright (c) 2013 by Nemesis.                             *
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

#ifndef __LIBGENS_TESTS_VDPFIFOTESTING_DATA_H__
#define __LIBGENS_TESTS_VDPFIFOTESTING_DATA_H__

// C includes.
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ROM for VdpFIFOTesting.
 * Original version; does NOT have the required patch to disable input.
 *
 * Data is gzipped; original size is 524,288.
 */
extern const uint8_t test_vdpfifotesting_rom[12004];
static const unsigned int test_vdpfifotesting_rom_sz = 524288;

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_TESTS_VDPFIFOTESTING_DATA_H__ */
