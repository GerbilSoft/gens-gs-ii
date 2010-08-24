/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * zomg_psg.h: ZOMG save definitions for the TI SN76489 (PSG) emulator.    *
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

#ifndef __LIBGENS_SAVE_ZOMG_ZOMG_PSG_H__
#define __LIBGENS_SAVE_ZOMG_ZOMG_PSG_H__

#include <stdint.h>

// Packed struct attribute.
#if !defined(PACKED)
#if defined(__GNUC__)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif /* defined(__GNUC__) */
#endif /* !defined(PACKED) */

#ifdef __cplusplus
extern "C" {
#endif

// PSG save struct.
// NOTE: Byteswapping is done in Zomg.cpp when saving/loading.
#pragma pack(1)
typedef struct PACKED _Zomg_PsgSave_t
{
	/**
	 * tone_reg[]: Tone registers.
	 * 0-2 contain tone values from 0x000 - 0x3FF.
	 * 3 contains noise value from 0x0 - 0x7.
	 */
	uint16_t tone_reg[4];
	
	/**
	 * vol_reg[]: Volume registers.
	 * Values range from 0x0 (no attenuation) to 0xF (off).
	 */
	uint8_t vol_reg[4];
	
	/**
	 * tone_ctr[]: TONE counters.
	 * Contains the current countdown until the appropriate line is toggled.
	 * If the emulator doesn't support it, set these to 0xFFFF on save.
	 */
	uint16_t tone_ctr[4];
	
	/**
	 * lfsr_state: Linear feedback register state.
	 */
	uint16_t lfsr_state;
	
	/**
	 * gg_stereo: Game Gear stereo register.
	 * Set to 0x00 for other PSGs.
	 */
	uint8_t gg_stereo;
} Zomg_PsgSave_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_SAVE_ZOMG_ZOMG_PSG_H__ */
