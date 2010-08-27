/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * zomg_ym2612.h: ZOMG save definitions for the Yamaha YM2612 emulator.    *
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

#ifndef __LIBGENS_SAVE_ZOMG_ZOMG_YM2612_H__
#define __LIBGENS_SAVE_ZOMG_ZOMG_YM2612_H__

#include "zomg_common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// YM2612 save struct.
// NOTE: Byteswapping is done in Zomg.cpp when saving/loading.
#pragma pack(1)
typedef struct PACKED _Zomg_Ym2612Save_t
{
	/**
	 * reg[][]: YM2612 registers.
	 * Consists of two banks of 256 registers.
	 */
	uint8_t reg[2][0x100];
	
	// TODO: YM timers.
	// TODO: Other stuff to save.
} Zomg_Ym2612Save_t;
#pragma pack(0)

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_SAVE_ZOMG_ZOMG_YM2612_H__ */
