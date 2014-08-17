/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpStructs.hpp: VDP structs.                                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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

#ifndef __LIBGENS_MD_VDPSTRUCTS_HPP__
#define __LIBGENS_MD_VDPSTRUCTS_HPP__

#include <stdint.h>
#include "../macros/common.h"

// NOTE: This file is generated at compile-time.
// It's located in the binary directory.
#include "libgens/Util/byteorder.h"

namespace LibGens
{

namespace VdpStructs
{
	// Sprite attribute table entry. (M5)
	// NOTE: VRAM is 16-bit host-endian.
	#pragma pack(1)
	struct PACKED SprEntry_m5 {
		uint16_t y;	// Mask == 0x1FF (0x3FF in IM2)
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
		uint8_t link;	// Mask == 0x7F
		uint8_t sz;	// ----hhvv; units are cells
#else /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
		uint8_t sz;	// ----hhvv; units are cells
		uint8_t link;	// Mask == 0x7F
#endif
		uint16_t attr;	// pccvhnnn nnnnnnnn
		uint16_t x;	// Mask == 0x1FF
	};
	#pragma pack()

	// Sprite attribute table. (M4)
	// Note that sprite attributes aren't packed here,
	// so we have to have the entire table in one struct.
	#pragma pack(1)
	struct PACKED SprTable_m4 {
		uint8_t y[64];	// Y coordinate + 1
		uint8_t reserved[64];
		struct PACKED {
			uint8_t x;	// X coordinate
			uint8_t tile;	// Pattern index
		} xn[64];
	};
	#pragma pack()
}

}

#endif /* __LIBGENS_MD_VDPSTRUCTS_HPP__ */
