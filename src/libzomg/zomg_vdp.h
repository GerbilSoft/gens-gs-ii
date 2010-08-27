/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * zomg_vdp.h: ZOMG save definitions for the Video Display Processor.      *
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

#ifndef __LIBGENS_SAVE_ZOMG_ZOMG_VDP_H__
#define __LIBGENS_SAVE_ZOMG_ZOMG_VDP_H__

#include "zomg_common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * vdp_reg: VDP registers.
 * File: common/vdp_reg.bin
 * NOTE: Byteswapping is done in Zomg.cpp when saving/loading.
 */
#pragma pack(1)
typedef union _vdp_reg
{
	uint8_t tms9918[8];	// TMS9918: 0x00 - 0x07
	uint8_t sms[11];	// SMS/GG: 0x00 - 0x0A
	uint8_t md[24];		// MD: 0x00 - 0x17
} vdp_reg;
#pragma pack()

/**
 * VRam: Video RAM.
 * File: common/VRam.bin
 * NOTE: Byteswapping is done in Zomg.cpp when saving/loading.
 */
#pragma pack(1)
typedef union _Zomg_VRam_t
{
	uint8_t sms[16384];	// TMS9918/SMS/GG
	uint16_t md[32768];	// MD
} _Zomg_VRam_t;
#pragma pack()

/**
 * CRam: Color RAM.
 * File: common/CRam.bin
 * NOTE: Byteswapping is done in Zomg.cpp when saving/loading.
 */
#pragma pack(1)
typedef union _Zomg_CRam_t
{
	uint8_t sms[32];	// SMS only
	uint16_t gg[32];	// GG (little-endian)
	uint16_t md[64];	// MD (big-endian)
} Zomg_CRam_t;
#pragma pack()

// TODO: Other common VDP stuff.

/** MD-specific. **/

#pragma pack(1)
/**
 * MD_VSRam: Vertical Scroll RAM.
 * File: MD/VSRam.bin
 */
typedef struct _Zomg_MD_VSRam_t
{
	uint16_t MD_VSRam[40];
} Zomg_MD_VSRam_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_SAVE_ZOMG_ZOMG_VDP_H__ */
