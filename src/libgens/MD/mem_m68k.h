/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * mem_m68k.h: Main 68000 memory handler.                                  *
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

#ifndef __GENS_MEM_M68K_H__
#define __GENS_MEM_M68K_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union
{
	uint8_t  u8[64*1024];
	uint16_t u16[(64*1024)>>1];
	uint32_t u32[(64*1024)>>2];
} Ram_68k_t;
extern Ram_68k_t Ram_68k;
typedef union
{
	uint8_t  u8[6*1024*1024];
	uint16_t u16[(6*1024*1024)>>1];
	uint32_t u32[(6*1024*1024)>>2];
} Rom_Data_t;
extern Rom_Data_t Rom_Data;

// Genesis TMSS ROM.
extern unsigned char MD_TMSS_Rom[2 * 1024];

// Main MC68000 read/write functions.
typedef uint8_t  (*M68K_Read_Byte_fn)(uint32_t address);
typedef uint16_t (*M68K_Read_Word_fn)(uint32_t address);
typedef void     (*M68K_Write_Byte_fn)(uint32_t address, uint8_t data);
typedef void     (*M68K_Write_Word_fn)(uint32_t address, uint8_t data);

// Main M68K function tables. (512 KB pages; 0x20 entries.)
extern M68K_Read_Byte_fn M68K_Read_Byte_Table[0x20];
extern M68K_Read_Word_fn M68K_Read_Word_Table[0x20];
extern M68K_Write_Byte_fn M68K_Write_Byte_Table[0x20];
extern M68K_Write_Word_fn M68K_Write_Word_Table[0x20];

// Default M68K function tables for MD.
extern const M68K_Read_Byte_fn MD_M68K_Read_Byte_Table[0x20];
extern const M68K_Read_Word_fn MD_M68K_Read_Word_Table[0x20];
extern const M68K_Write_Byte_fn MD_M68K_Write_Byte_Table[0x20];
extern const M68K_Write_Word_fn MD_M68K_Write_Word_Table[0x20];

// SRam.
extern uint8_t SRam[64 * 1024];
extern uint32_t SRam_Start;
extern uint32_t SRam_End;

typedef struct
{
	uint32_t on      : 1;
	uint32_t write   : 1;
	uint32_t custom  : 1;
	uint32_t enabled : 1;
} SRam_State_t;
extern SRam_State_t SRam_State;

extern unsigned int Rom_Size;

// Z80/M68K cycle table.
extern int Z80_M68K_Cycle_Tab[512];

/** Z80 state. **/
#define Z80_STATE_ENABLED	(1 << 0)
#define Z80_STATE_BUSREQ	(1 << 1)
#define Z80_STATE_RESET		(1 << 2)

extern unsigned int Z80_State;
extern int Last_BUS_REQ_Cnt;
extern int Last_BUS_REQ_St;
extern int Bank_M68K;
extern int Fake_Fetch;

extern int CPL_M68K;
extern int CPL_Z80;
extern int Cycles_M68K;
extern int Cycles_Z80;

extern int Game_Mode;
extern int CPU_Mode;
extern int Gen_Mode;

void Init_Memory_M68K(int System_ID);
unsigned char M68K_RB(unsigned int Adr);
unsigned short M68K_RW(unsigned int Adr);
void M68K_WB(unsigned int Adr, unsigned char Data);
void M68K_WW(unsigned int Adr, unsigned short Data);

#ifdef __cplusplus
}
#endif

#endif /* __GENS_MEM_M68K_H__ */
