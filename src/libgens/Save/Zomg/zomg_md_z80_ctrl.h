/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * zomg_md_z80_ctrl.h: ZOMG save definitions for the MD Z80 control logic. *
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

#ifndef __LIBGENS_SAVE_ZOMG_ZOMG_MD_Z80_CTRL_H__
#define __LIBGENS_SAVE_ZOMG_ZOMG_MD_Z80_CTRL_H__

#include "zomg_common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MD Z80 control logic struct.
 * ZOMG file: MD/Z80_ctrl.bin
 * NOTE: Byteswapping is done in Zomg.cpp when saving/loading.
 */
#pragma pack(1)
typedef struct PACKED _Zomg_MD_Z80CtrlSave_t
{
	uint8_t busreq;		// 8-bit: BUSREQ state.
				// 0 == Z80 has the bus.
				// 1 == M68K has the bus.
	
	uint8_t reset;		// 8-bit: RESET state.
				// 0 == Z80 is RESET.
				// 1 == Z80 is running.
	
	uint16_t m68k_bank;	// 16-bit BE: M68K banking register.
				// Low 9 bits of m68k_bank indicate
				// high 9 bits of M68K address space
				// for the Z80 banking space.
} Zomg_MD_Z80CtrlSave_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif /* __LIBGENS_SAVE_ZOMG_ZOMG_MD_Z80_CTRL_H__ */
