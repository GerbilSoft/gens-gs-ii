/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * zomg_vdp.h: ZOMG save definitions for the Video Display Processor.      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
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

#ifndef __LIBGENS_SAVE_ZOMG_ZOMG_VDP_H__
#define __LIBGENS_SAVE_ZOMG_ZOMG_VDP_H__

#include "zomg_common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// VDP save struct.
// NOTE: Byteswapping is done in Zomg.cpp when saving/loading.
#pragma pack(1)
typedef struct PACKED _Zomg_VdpSave_t
{
	/**
	 * vdp_reg: VDP registers.
	 * File: common/vdp_reg.bin
	 */
	union _vdp_reg
	{
		uint8_t tms9918[8];	// TMS9918: 0x00 - 0x07
		uint8_t sms[11];	// SMS/GG: 0x00 - 0x0A
		uint8_t md[24];		// MD: 0x00 - 0x17
	} vdp_reg;
	
	/**
	 * VRam: Video RAM.
	 * File: common/VRam.bin
	 */
	union _VRam
	{
		uint8_t sms[16384];	// TMS9918/SMS/GG
		uint16_t md[32768];	// MD
	} VRam;
	
	/**
	 * CRam: Color RAM.
	 * File: common/CRam.bin
	 */
	union _CRam
	{
		uint8_t sms[32];	// SMS only
		uint16_t gg[32];	// GG (little-endian)
		uint16_t md[64];	// MD (big-endian)
	} CRam;
	
	// TODO: Other common VDP stuff.
	
	/** MD-specific. **/
	
	/**
	 * MD_VSRam: Vertical Scroll RAM.
	 * File: MD/VSRam.bin
	 */
	uint16_t MD_VSRam[40];
} Zomg_VdpSave_t;
#pragma pack(0)

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_SAVE_ZOMG_ZOMG_VDP_H__ */
