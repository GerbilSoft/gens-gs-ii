/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Z80_MD_Mem.hpp: Z80 memory handler. (Mega Drive mode)                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __LIBGENS_CPU_Z80_MEM_HPP__
#define __LIBGENS_CPU_Z80_MEM_HPP__

#include <stdint.h>

// NOTE: mdZ80 uses the FASTCALL calling convention.
#include "macros/fastcall.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: mdZ80 accesses Ram_Z80 directly.
// Move Ram_Z80 back to Z80_MD_Mem once mdZ80 is updated.
extern uint8_t Ram_Z80[8 * 1024];

#ifdef __cplusplus
}
#endif

namespace LibGens
{

class Z80_MD_Mem
{
	public:
		static void Init(void);
		static void End(void);
		
#if 0
		// TODO: mdZ80 accesses Ram_Z80 directly.
		// Move Ram_Z80 back to Z80_MD_Mem once mdZ80 is updated.
		static uint8_t Ram_Z80[8 * 1024];
#endif
		
		// 68K ROM banking address.
		static int Bank_Z80;
		
		/** Public read/write functions. **/
		// TODO: Make these inline!
		static uint8_t Z80_ReadB(uint32_t address);
		static uint16_t Z80_ReadW(uint32_t address);
		static void Z80_WriteB(uint32_t address, uint8_t data);
		static void Z80_WriteW(uint32_t address, uint16_t data);
		
		/**
		 * NOTE: These functions *should* be marked protected,
		 * but they're needed by the Z80 class to initialize mdZ80.
		 * 
		 * TODO: Maybe move the mdZ80 initialization here?
		 */
		
		/** Read Byte functions. **/
		static uint8_t FASTCALL Z80_ReadB_Bad(uint32_t address);
		static uint8_t FASTCALL Z80_ReadB_Ram(uint32_t address);
		static uint8_t FASTCALL Z80_ReadB_Bank(uint32_t address);
		static uint8_t FASTCALL Z80_ReadB_YM2612(uint32_t address);
		static uint8_t FASTCALL Z80_ReadB_PSG(uint32_t address);
		static uint8_t FASTCALL Z80_ReadB_68K_Ram(uint32_t address);
		
		/** Read Word functions. (DEPRECATED) **/
		static uint16_t FASTCALL Z80_ReadW_Bad(uint32_t address);
		static uint16_t FASTCALL Z80_ReadW_Ram(uint32_t address);
		static uint16_t FASTCALL Z80_ReadW_Bank(uint32_t address);
		static uint16_t FASTCALL Z80_ReadW_YM2612(uint32_t address);
		static uint16_t FASTCALL Z80_ReadW_PSG(uint32_t address);
		static uint16_t FASTCALL Z80_ReadW_68K_Ram(uint32_t address);
		
		/** Write Byte functions. **/
		static void FASTCALL Z80_WriteB_Bad(uint32_t address, uint8_t data);
		static void FASTCALL Z80_WriteB_Ram(uint32_t address, uint8_t data);
		static void FASTCALL Z80_WriteB_Bank(uint32_t address, uint8_t data);
		static void FASTCALL Z80_WriteB_YM2612(uint32_t address, uint8_t data);
		static void FASTCALL Z80_WriteB_PSG(uint32_t address, uint8_t data);
		static void FASTCALL Z80_WriteB_68K_Ram(uint32_t address, uint8_t data);
		
		/** Write Word functions. (DEPRECATED) **/
		static void FASTCALL Z80_WriteW_Bad(uint32_t address, uint16_t data);
		static void FASTCALL Z80_WriteW_Bank(uint32_t address, uint16_t data);
		static void FASTCALL Z80_WriteW_Ram(uint32_t address, uint16_t data);
		static void FASTCALL Z80_WriteW_YM2612(uint32_t address, uint16_t data);
		static void FASTCALL Z80_WriteW_PSG(uint32_t address, uint16_t data);
		static void FASTCALL Z80_WriteW_68K_Ram(uint32_t address, uint16_t data);
	
	protected:
		/** Z80 read/write functions. **/
		typedef uint8_t  FASTCALL (*Z80_ReadB_fn) (uint32_t address);
		typedef uint16_t FASTCALL (*Z80_ReadW_fn) (uint32_t address); // DEPRECATED
		typedef void     FASTCALL (*Z80_WriteB_fn)(uint32_t address, uint8_t data);
		typedef void     FASTCALL (*Z80_WriteW_fn)(uint32_t address, uint16_t data); // DEPRECATED
		
		/** Z80 function tables. (4 KB pages; 16 entries.) **/
		static const Z80_ReadB_fn Z80_ReadB_Table[0x10];
		static const Z80_ReadW_fn Z80_ReadW_Table[0x10]; // DEPRECATED
		static const Z80_WriteB_fn Z80_WriteB_Table[0x10];
		static const Z80_WriteW_fn Z80_WriteW_Table[0x10]; // DEPRECATED
};

}

#endif /* __LIBGENS_CPU_Z80_MEM_HPP__ */
