/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * M68K_Mem.cpp: Main 68000 memory handler.                                *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#include "M68K_Mem.hpp"
#include "MD/VdpIo.hpp"

// Z80 CPU emulator and memory space.
#include "Z80.hpp"
#include "Z80_MD_Mem.hpp"

// C includes.
#include <string.h>

// MD emulator.
// Required to access controller I/O ports.
// TODO: SMS/GG?
#include "MD/EmuMD.hpp"

// Sound Manager.
#include "sound/SoundMgr.hpp"

// TODO: Starscream accesses Ram_68k directly.
// Move Ram_68k back to M68K once Starscream is updated.
Ram_68k_t Ram_68k;

// EmuContext
#include "../EmuContext.hpp"

// C wrapper functions for Starscream.
#ifdef __cplusplus
extern "C" {
#endif

uint8_t Gens_M68K_RB(uint32_t address)
{
	/** WORKAROUND for Starscream not properly saving ecx/edx. **/
	return LibGens::M68K_Mem::M68K_RB(address);
}
uint16_t Gens_M68K_RW(uint32_t address)
{
	/** WORKAROUND for Starscream not properly saving ecx/edx. **/
	return LibGens::M68K_Mem::M68K_RW(address);
}
void Gens_M68K_WB(uint32_t address, uint8_t data)
{
	/** WORKAROUND for Starscream not properly saving ecx/edx. **/
	LibGens::M68K_Mem::M68K_WB(address, data);
}
void Gens_M68K_WW(uint32_t address, uint16_t data)
{
	/** WORKAROUND for Starscream not properly saving ecx/edx. **/
	LibGens::M68K_Mem::M68K_WW(address, data);
}

#ifdef __cplusplus
}
#endif

namespace LibGens
{

/** ROM and RAM variables. **/
//M68K_Mem::Ram_68k_t M68K_Mem::Ram_68k;	// TODO: Fix Starscream!
M68K_Mem::Rom_Data_t M68K_Mem::Rom_Data;
unsigned int M68K_Mem::Rom_Size = 0;

/** Z80/M68K cycle table. **/
int M68K_Mem::Z80_M68K_Cycle_Tab[512];

// M68K static variables.
// TODO: Improve some of these, especially the cycle counters!
unsigned int M68K_Mem::Z80_State;
int M68K_Mem::Last_BUS_REQ_Cnt;
int M68K_Mem::Last_BUS_REQ_St;
int M68K_Mem::Bank_M68K;
int M68K_Mem::Fake_Fetch;

int M68K_Mem::CPL_M68K;
int M68K_Mem::CPL_Z80;
int M68K_Mem::Cycles_M68K;
int M68K_Mem::Cycles_Z80;

// TODO: Move ms_Region somewhere else?
SysVersion M68K_Mem::ms_SysVersion;

uint8_t M68K_Mem::ms_SSF2_BankState[8];
uint8_t *M68K_Mem::ms_RomData_ptrs[10];


void M68K_Mem::Init(void)
{
	// Initialize the Z80/M68K cycle table.
	for (int x = 0; x < 512; x++)
		Z80_M68K_Cycle_Tab[x] = (int)((double) x * 7.0 / 15.0);
	
	// Initialize the SSF2 bankswitching state.
	memset(ms_SSF2_BankState, 0xFF, sizeof(ms_SSF2_BankState));
	
	// Initialize the ROM data pointers.
	for (int i = 0; i < 10; i++)
		ms_RomData_ptrs[i] = &Rom_Data.u8[i * 0x80000];
}


void M68K_Mem::End(void)
{
}


/** Read Byte functions. **/


/**
 * M68K_Read_Byte_Rom(): Read a byte from ROM.
 * TODO: XOR by 1 on little-endian systems only.
 * @param bank ROM bank number.
 * @param address Address.
 * @return Byte from ROM.
 */
inline uint8_t M68K_Mem::M68K_Read_Byte_Rom(uint32_t address)
{
	const uint8_t bank = (address >> 19) & 0x07;
	address &= 0x7FFFF;
	address ^= 1;	// TODO: LE only!
	return ms_RomData_ptrs[bank][address];
}
// TODO: Add banks C, D, E, and F for 8 MB ROM support.
// For now, they will return 0x00.


/**
 * T_M68K_Read_Byte_RomX_SRam(): Read a byte from ROM bank X or SRam/EEPRom.
 * TODO: Verify that this works for 0x300000/0x380000.
 * TODO: XOR by 1 on little-endian systems only.
 * @param bank ROM bank number.
 * @param address Address.
 * @return Byte from ROM or SRam/EEPRom.
 */
template<uint8_t bank>
inline uint8_t M68K_Mem::T_M68K_Read_Byte_RomX_SRam(uint32_t address)
{
	// Check if this is a save data request.
	if (EmuContext::GetSaveDataEnable())
	{
		// Temporarily needed because M68K_Mem is static.
		EEPRom *eeprom = EmuContext::GetEEPRom();
		SRam *sram = EmuContext::GetSRam();
		
		if (eeprom->isEEPRomTypeSet())
		{
			// EEPRom is enabled.
			if (eeprom->isReadBytePort(address))
			{
				// EEPRom read port.
				return eeprom->readByte(address);
			}
		}
		else if (sram->canRead() && sram->isAddressInRange(address))
		{
			// SRam data request.
			// Return the byte from SRam.
			// NOTE: SRam is NOT byteswapped.
			// TODO: Check boundaries.
			// TODO: Should start/end addressing be handled here or in SRam?
			return sram->readByte(address);
		}
	}
	
	address &= 0x7FFFF;
	address ^= ((bank << 19) | 1);	// TODO: LE only!
	return Rom_Data.u8[address];
}


/**
 * M68K_Read_Byte_Ram(): Read a byte from RAM. (0xE00000 - 0xFFFFFF)
 * RAM is 64 KB, mirrored throughout the entire range.
 * @param address Address.
 * @return Byte from RAM.
 */
inline uint8_t M68K_Mem::M68K_Read_Byte_Ram(uint32_t address)
{
	address &= 0xFFFF;
	address ^= 1;	// TODO: LE only!
	return Ram_68k.u8[address];
}


/**
 * M68K_Read_Byte_Misc(): Read a byte from the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @return Miscellaneous data byte.
 */
inline uint8_t M68K_Mem::M68K_Read_Byte_Misc(uint32_t address)
{
	if (address <= 0xA0FFFF)
	{
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET))
		{
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			return 0;
		}
		
		// Call the Z80 Read Byte function.
		// TODO: CPU lockup on accessing 0x7Fxx or >=0x8000.
		return Z80_MD_Mem::Z80_ReadB(address & 0xFFFF);
	}
	else if (address == 0xA11100)
	{
		// Z80 BUSREQ. (0xA11100)
		// NOTE: Genesis Plus does BUSREQ at any even 0xA111xx...
		if (Z80_State & Z80_STATE_BUSREQ)
		{
			// Z80 is currently running.
			return 0x81;
		}
		
		// Z80 is not running.
		int odo68k = M68K::ReadOdometer();
		odo68k -= Last_BUS_REQ_Cnt;
		if (odo68k <= CYCLE_FOR_TAKE_Z80_BUS_GENESIS)
			return ((Last_BUS_REQ_St | 0x80) & 0xFF);
		else
			return 0x80;
	}
	else if (address > 0xA1001F)
	{
		// Invalid address.
		return 0;
	}
	
	// TODO: 0xA11200? (Z80 RESET)
	
	/**
	 * MD miscellaneous registers.
	 * 0xA10001: Genesis version register.
	 * 0xA10003: Control Port 1: Data.
	 * 0xA10005: Control Port 2: Data.
	 * 0xA10007: Control Port 3: Data. (EXT)
	 * 0xA10009: Control Port 1: CTRL.
	 * 0xA1000B: Control Port 2: CTRL.
	 * 0xA1000D: Control Port 3: CTRL. (EXT)
	 * 0xA1000F: Control Port 1: Serial TxData.
	 * 0xA10011: Control Port 1: Serial RxData. (READ-ONLY)
	 * 0xA10013: Control Port 1: Serial Control.
	 * 0xA10015: Control Port 2: Serial TxData.
	 * 0xA10017: Control Port 2: Serial RxData. (READ-ONLY)
	 * 0xA10019: Control Port 2: Serial Control.
	 * 0xA1001B: Control Port 3: Serial TxData.
	 * 0xA1001D: Control Port 3: Serial RxData. (READ-ONLY)
	 * 0xA1001F: Control Port 3: Serial Control.
	 */
	// TODO: Do byte reads from even addresses (e.g. 0xA10002) work?
	switch (address & 0x1E)
	{
		case 0x00:
			// 0xA10001: Genesis version register.
			return ms_SysVersion.readData();
		
		// Parallel I/O
		case 0x02:	return EmuMD::m_port1->readData();
		case 0x04:	return EmuMD::m_port2->readData();
		case 0x06:	return EmuMD::m_portE->readData();
		case 0x08:	return EmuMD::m_port1->readCtrl();
		case 0x0A:	return EmuMD::m_port2->readCtrl();
		case 0x0C:	return EmuMD::m_portE->readCtrl();
		
		// Serial I/O
		// TODO: Baud rate handling, etc.
		case 0x0E:	return EmuMD::m_port1->readSerTx();
		case 0x10:	return EmuMD::m_port1->readSerRx();
		case 0x12:	return EmuMD::m_port1->readSerCtrl();
		case 0x14:	return EmuMD::m_port2->readSerTx();
		case 0x16:	return EmuMD::m_port2->readSerRx();
		case 0x18:	return EmuMD::m_port2->readSerCtrl();
		case 0x1A:	return EmuMD::m_portE->readSerTx();
		case 0x1C:	return EmuMD::m_portE->readSerRx();
		case 0x1E:	return EmuMD::m_portE->readSerCtrl();
		
		default:
			// Unknown register.
			return 0x00;
	}
}


/**
 * M68K_Read_Byte_VDP(): Read a byte from the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @return VDP data byte.
 */
inline uint8_t M68K_Mem::M68K_Read_Byte_VDP(uint32_t address)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.
	
	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0)
	{
		// Not a valid VDP address.
		return 0x00;
	}
	
	// Check the VDP address.
	address &= 0x1F;
	if (address < 0x04)
	{
		// Invalid address.
		// NOTE: VDP Data port should still be readable...
		return 0x00;
	}
	else if (address < 0x08)
	{
		// 0xC00004 - 0xC00007: VDP Control Port.
		uint16_t vdp_status = VdpIo::Read_Status();
		if (!(address & 0x01))
		{
			// 0xC00004/0xC00006. Return the high byte.
			return ((vdp_status >> 8) & 0xFF);
		}
		else //if (address & 0x01)
		{
			// 0xC00005/0xC00007. Return the low byte.
			return (vdp_status & 0xFF);
		}
	}
	else if (address == 0x08)
	{
		// 0xC00008: V counter.
		return VdpIo::Read_V_Counter();
	}
	else if (address == 0x09)
	{
		// 0xC00009: H counter.
		return VdpIo::Read_H_Counter();
	}
	
	// Invalid VDP address.
	return 0x00;
}


/** Read Word functions. **/


/**
 * M68K_Read_Byte_RomX(): Read a word from ROM.
 * @param bank ROM bank number.
 * @param address Address.
 * @return Word from ROM.
 */
inline uint16_t M68K_Mem::M68K_Read_Word_Rom(uint32_t address)
{
	const uint8_t bank = (address >> 19) & 0x07;
	address &= 0x7FFFE;
	address >>= 1;
	
	// TODO: ms_RomData_ptrs should provide u8 and u16.
	return ((uint16_t*)ms_RomData_ptrs[bank])[address];
}
// TODO: Add banks C, D, E, and F for 8 MB ROM support.
// For now, they will return 0x00.


/**
 * T_M68K_Read_Word_RomX_SRam(): Read a word from ROM bank X or SRam/EEPRom.
 * TODO: Verify that this works for 0x300000/0x380000.
 * @param bank ROM bank number.
 * @param address Address.
 * @return Word from ROM or SRam/EEPRom.
 */
template<uint8_t bank>
inline uint16_t M68K_Mem::T_M68K_Read_Word_RomX_SRam(uint32_t address)
{
	// Check if this is a save data request.
	if (EmuContext::GetSaveDataEnable())
	{
		// Temporarily needed because M68K_Mem is static.
		EEPRom *eeprom = EmuContext::GetEEPRom();
		SRam *sram = EmuContext::GetSRam();
		
		if (eeprom->isEEPRomTypeSet())
		{
			// EEPRom is enabled.
			if (eeprom->isReadBytePort(address))
			{
				// EEPRom read port.
				return eeprom->readWord(address);
			}
		}
		else if (sram->canRead() && sram->isAddressInRange(address))
		{
			// SRam data request.
			// Return the byte from SRam.
			// NOTE: SRam is NOT byteswapped.
			// TODO: Check boundaries.
			// TODO: Should start/end addressing be handled here or in SRam?
			return sram->readWord(address);
		}
	}
	
	// ROM data request.
	address &= 0x7FFFE;
	address |= (bank << 19);
	address >>= 1;
	return Rom_Data.u16[address];
}


/**
 * M68K_Read_Word_Ram(): Read a word from RAM. (0xE00000 - 0xFFFFFF)
 * RAM is 64 KB, mirrored throughout the entire range.
 * @param address Address.
 * @return Word from RAM.
 */
inline uint16_t M68K_Mem::M68K_Read_Word_Ram(uint32_t address)
{
	address &= 0xFFFE;
	return Ram_68k.u16[address >> 1];
}


/**
 * M68K_Read_Word_Misc(): Read a word from the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @return Miscellaneous data word.
 */
inline uint16_t M68K_Mem::M68K_Read_Word_Misc(uint32_t address)
{
	if (address <= 0xA0FFFF)
	{
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET))
		{
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			return 0;
		}
		
		// Call the Z80 Read Byte function.
		// TODO: CPU lockup on accessing 0x7Fxx or >=0x8000.
		// Genesis Plus duplicates the byte in both halves of the M68K word.
		uint8_t ret = Z80_MD_Mem::Z80_ReadB(address & 0xFFFF);
		return (ret | (ret << 8));
	}
	else if (address == 0xA11100)
	{
		// Z80 BUSREQ. (0xA11100)
		// NOTE: Genesis Plus does BUSREQ at any even 0xA111xx...
		if (Z80_State & Z80_STATE_BUSREQ)
		{
			// Z80 is currently running.
			// NOTE: Low byte is supposed to be from
			// the next fetched instruction.
			Fake_Fetch ^= 0xFF;	// Fake the next fetched instruction. ("random")
			return (0x8100 | (Fake_Fetch & 0xFF));
		}
		
		// Z80 is not running.
		int odo68k = M68K::ReadOdometer();
		odo68k -= Last_BUS_REQ_Cnt;
		if (odo68k <= CYCLE_FOR_TAKE_Z80_BUS_GENESIS)
		{
			// bus not taken yet
			uint16_t ret;
			Fake_Fetch ^= 0xFF;	// Fake the next fetched instruction. ("random")
			ret = (Fake_Fetch & 0xFF);
			ret |= ((Last_BUS_REQ_St & 0xFF) << 8);
			ret += 0x8000;
			return ret;
		}
		else
		{
			// bus taken
			uint16_t ret;
			Fake_Fetch ^= 0xFF;	// Fake the next fetched instruction. ("random")
			ret = (Fake_Fetch & 0xFF) | 0x8000;
			return ret;
		}
	}
	else if (address > 0xA1001F)
	{
		// Invalid address.
		return 0;
	}
	
	// TODO: 0xA11200? (Z80 RESET)
	
	/**
	 * MD miscellaneous registers.
	 * 0xA10001: Genesis version register.
	 * 0xA10003: Control Port 1: Data.
	 * 0xA10005: Control Port 2: Data.
	 * 0xA10007: Control Port 3: Data. (EXT)
	 * 0xA10009: Control Port 1: CTRL.
	 * 0xA1000B: Control Port 2: CTRL.
	 * 0xA1000D: Control Port 3: CTRL. (EXT)
	 * 0xA1000F: Control Port 1: Serial TxData.
	 * 0xA10011: Control Port 1: Serial RxData. (READ-ONLY)
	 * 0xA10013: Control Port 1: Serial Control.
	 * 0xA10015: Control Port 2: Serial TxData.
	 * 0xA10017: Control Port 2: Serial RxData. (READ-ONLY)
	 * 0xA10019: Control Port 2: Serial Control.
	 * 0xA1001B: Control Port 3: Serial TxData.
	 * 0xA1001D: Control Port 3: Serial RxData. (READ-ONLY)
	 * 0xA1001F: Control Port 3: Serial Control.
	 */
	switch (address & 0x1E)
	{
		case 0x00:
			// 0xA10001: Genesis version register.
			return ms_SysVersion.readData();
		
		// Parallel I/O
		case 0x02:	return EmuMD::m_port1->readData();
		case 0x04:	return EmuMD::m_port2->readData();
		case 0x06:	return EmuMD::m_portE->readData();
		case 0x08:	return EmuMD::m_port1->readCtrl();
		case 0x0A:	return EmuMD::m_port2->readCtrl();
		case 0x0C:	return EmuMD::m_portE->readCtrl();
		
		// Serial I/O
		// TODO: Baud rate handling, etc.
		case 0x0E:	return EmuMD::m_port1->readSerTx();
		case 0x10:	return EmuMD::m_port1->readSerRx();
		case 0x12:	return EmuMD::m_port1->readSerCtrl();
		case 0x14:	return EmuMD::m_port2->readSerTx();
		case 0x16:	return EmuMD::m_port2->readSerRx();
		case 0x18:	return EmuMD::m_port2->readSerCtrl();
		case 0x1A:	return EmuMD::m_portE->readSerTx();
		case 0x1C:	return EmuMD::m_portE->readSerRx();
		case 0x1E:	return EmuMD::m_portE->readSerCtrl();
		
		default:
			// Unknown register.
			return 0x0000;
	}
}


/**
 * M68K_Read_Word_VDP(): Read a word from the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @return VDP data byte.
 */
inline uint16_t M68K_Mem::M68K_Read_Word_VDP(uint32_t address)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.
	
	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0)
	{
		// Not a valid VDP address.
		return 0x00;
	}
	
	// Check the VDP address.
	address &= 0x1F;
	if (address < 0x04)
	{
		// 0xC00000 - 0xC00003: VDP Data Port.
		return VdpIo::Read_Data();
	}
	else if (address < 0x08)
	{
		// 0xC00004 - 0xC00007: VDP Control port.
		return VdpIo::Read_Status();
	}
	else if (address < 0x0A)
	{
		// 0xC00008 - 0xC00009: HV counter.
		return ((VdpIo::Read_V_Counter() << 8) | VdpIo::Read_H_Counter());
	}
	
	// Invalid VDP address.
	return 0x00;
}


/** Write Byte functions. **/


/**
 * M68K_Write_Byte_SRam(): Write a byte to SRam.
 * @param address Address.
 * @param data Byte to write.
 */
inline void M68K_Mem::M68K_Write_Byte_SRam(uint32_t address, uint8_t data)
{
	if (!EmuContext::GetSaveDataEnable())
	{
		// Save data is disabled.
		return;
	}
	
	// Temporarily needed because M68K_Mem is static.
	EEPRom *eeprom = EmuContext::GetEEPRom();
	SRam *sram = EmuContext::GetSRam();
	
	if (eeprom->isEEPRomTypeSet())
	{
		// EEPRom is enabled.
		if (eeprom->isWriteBytePort(address))
		{
			// EEPRom write port.
			return eeprom->writeByte(address, data);
		
		}
	}
	else if (sram->canWrite() && sram->isAddressInRange(address))
	{
		// SRam data request.
		
		// Write the byte to SRam.
		// NOTE: SRam is NOT byteswapped.
		// TODO: Check boundaries.
		// TODO: Should start/end addressing be handled here or in SRam?
		sram->writeByte(address, data);
	}
}


/**
 * M68K_Write_Byte_Ram(): Write a byte to RAM. (0xE00000 - 0xFFFFFF)
 * RAM is 64 KB, mirrored throughout the entire range.
 * @param address Address.
 * @param data Byte to write.
 */
inline void M68K_Mem::M68K_Write_Byte_Ram(uint32_t address, uint8_t data)
{
	address &= 0xFFFF;
	address ^= 1;	// TODO: LE only!
	Ram_68k.u8[address] = data;
}


/**
 * M68K_Write_Byte_Misc(): Write a byte to the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @param data Byte to write.
 */
inline void M68K_Mem::M68K_Write_Byte_Misc(uint32_t address, uint8_t data)
{
	if (address <= 0xA0FFFF)
	{
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET))
		{
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			return;
		}
		
		// Call the Z80 Write Byte function.
		// TODO: CPU lockup on accessing 0x7Fxx or >=0x8000.
		Z80_MD_Mem::Z80_WriteB(address & 0xFFFF, data);
		return;
	}
	else if (address == 0xA11100)
	{
		// Z80 BUSREQ. (0xA11100)
		// NOTE: Genesis Plus does BUSREQ at any even 0xA111xx...
		// TODO: Combine with Byte Write version?
		
		// Zero the controller counters and delays.
		// NOTE: Genesis Plus doesn't do this, and it doesn't make sense.
		// The controller counters and delays are located in the controller,
		// not in the system.
#if 0
		xor	ecx, ecx
		mov	dword [SYM(Controller_1_Counter)], ecx
		mov	dword [SYM(Controller_1_Delay)], ecx
		mov	dword [SYM(Controller_2_Counter)], ecx
		mov	dword [SYM(Controller_2_Delay)], ecx
#endif
		
		if (data & 0x01)
		{
			// M68K requests the bus.
			// Disable the Z80.
			Last_BUS_REQ_Cnt = M68K::ReadOdometer();
			Last_BUS_REQ_St = (Z80_State & Z80_STATE_BUSREQ);
			
			if (Z80_State & Z80_STATE_BUSREQ)
			{
				// Z80 is running. Disable it.
				Z80_State &= ~Z80_STATE_BUSREQ;
				
				// TODO: Rework this.
				int ebx = (Cycles_M68K - Last_BUS_REQ_Cnt);
				ebx = Z80_M68K_Cycle_Tab[ebx];
				
				int edx = Cycles_Z80;
				edx -= ebx;
				Z80::Exec(edx);
			}
		}
		else
		{
			// M68K releases the bus.
			// Enable the Z80.
			if (!(Z80_State & Z80_STATE_BUSREQ))
			{
				// Z80 is stopped. Enable it.
				Z80_State |= Z80_STATE_BUSREQ;
				
				// TODO: Rework this.
				int ebx = Cycles_M68K;
				ebx -= M68K::ReadOdometer();
				
				int edx = Cycles_Z80;
				ebx = Z80_M68K_Cycle_Tab[ebx];
				edx -= ebx;
				
				// Set the Z80 odometer.
				Z80::SetOdometer((unsigned int)edx);
			}
		}
		
		return;
	}
	else if (address == 0xA11200)
	{
		// Z80 RESET. (0xA11200)
		// NOTE: Genesis Plus does RESET at any even 0xA112xx...
		if (data & 0x01)
		{
			// RESET is high. Start the Z80.
			Z80_State &= ~Z80_STATE_RESET;
		}
		else
		{
			// RESET is low. Stop the Z80.
			Z80::Reset();
			Z80_State |= Z80_STATE_RESET;
			
			// YM2612's RESET line is tied to the Z80's RESET line.
			SoundMgr::ms_Ym2612.reset();
		}
		return;
	}
	else if (address == 0xA130F1)
	{
		// SRam control register. (0xA130F1)
		
		// Temporarily needed because M68K_Mem is static.
		SRam *sram = EmuContext::GetSRam();
		
		sram->writeCtrl(data);
		return;
	}
	else if (address >= 0xA130F2 && address <= 0xA130FF)
	{
		// Super Street Fighter II (SSF2) bankswitching system.
		// TODO: Port to integrated dispatch function.
#if 0
		// TODO: Starscream doesn't use this for instruction fetch!
		// TODO: Use a helper class?
		// TODO: Only banks 0-9 are supported right now...
		unsigned int phys_bank = (address & 0xF) >> 1;
		unsigned int virt_bank = (data & 0x1F);
		
		if (virt_bank > 9)
		{
			// TODO: We're ignoring banks over bank 9.
			virt_bank = phys_bank;
			ms_SSF2_BankState[phys_bank] = 0xFF;	// default bank
		}
		else
		{
			// Save the bank number.
			ms_SSF2_BankState[phys_bank] = virt_bank;
		}
		
		// Set the banking in the read byte/word tables.
		M68K_Read_Byte_Table[phys_bank] = MD_M68K_Read_Byte_Table[virt_bank];
		M68K_Read_Word_Table[phys_bank] = MD_M68K_Read_Word_Table[virt_bank];
#endif
		return;
	}
	else if (address > 0xA1001F)
	{
		// Invalid address.
		return;
	}
	
	/**
	 * MD miscellaneous registers.
	 * 0xA10001: Genesis version register.
	 * 0xA10003: Control Port 1: Data.
	 * 0xA10005: Control Port 2: Data.
	 * 0xA10007: Control Port 3: Data. (EXT)
	 * 0xA10009: Control Port 1: CTRL.
	 * 0xA1000B: Control Port 2: CTRL.
	 * 0xA1000D: Control Port 3: CTRL. (EXT)
	 * 0xA1000F: Control Port 1: Serial TxData.
	 * 0xA10011: Control Port 1: Serial RxData. (READ-ONLY)
	 * 0xA10013: Control Port 1: Serial Control.
	 * 0xA10015: Control Port 2: Serial TxData.
	 * 0xA10017: Control Port 2: Serial RxData. (READ-ONLY)
	 * 0xA10019: Control Port 2: Serial Control.
	 * 0xA1001B: Control Port 3: Serial TxData.
	 * 0xA1001D: Control Port 3: Serial RxData. (READ-ONLY)
	 * 0xA1001F: Control Port 3: Serial Control.
	 */
	// TODO: Do byte writes to even addresses (e.g. 0xA10002) work?
	switch (address & 0x1E)
	{
		case 0x00: /// 0xA10001: Genesis version register.
		default:
			break;
		
		// Parallel I/O
		case 0x02:	EmuMD::m_port1->writeData(data); break;
		case 0x04:	EmuMD::m_port2->writeData(data); break;
		case 0x06:	EmuMD::m_portE->writeData(data); break;
		case 0x08:	EmuMD::m_port1->writeCtrl(data); break;
		case 0x0A:	EmuMD::m_port2->writeCtrl(data); break;
		case 0x0C:	EmuMD::m_portE->writeCtrl(data); break;
		
		// Serial I/O
		// TODO: Baud rate handling, etc.
		case 0x0E:	EmuMD::m_port1->writeSerTx(data); break;
		case 0x10:	break; // READ-ONLY
		case 0x12:	EmuMD::m_port1->writeSerCtrl(data); break;
		case 0x14:	EmuMD::m_port2->writeSerTx(data); break;
		case 0x16:	break; // READ-ONLY
		case 0x18:	EmuMD::m_port2->writeSerCtrl(data); break;
		case 0x1A:	EmuMD::m_portE->writeSerTx(data); break;
		case 0x1C:	break; // READ-ONLY
		case 0x1E:	EmuMD::m_portE->writeSerCtrl(data); break;
	}
}


/**
 * M68K_Write_Byte_VDP(): Write a byte to the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @param data Byte to write.
 */
inline void M68K_Mem::M68K_Write_Byte_VDP(uint32_t address, uint8_t data)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.
	
	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0)
	{
		// Not a valid VDP address.
		return;
	}
	
	// Check the VDP address.
	address &= 0x1F;
	if (address < 0x04)
	{
		// 0xC00000 - 0xC00003: VDP Data Port.
		VdpIo::Write_Data_Byte(data);
	}
	else if (address < 0x08)
	{
		// 0xC00004 - 0xC00007: VDP Control Port.
		// TODO: This should still be writable.
		// Gens' mem_m68k.asm doesn't implement this.
	}
	else if (address == 0x11)
	{
		// 0xC00011: PSG control port.
		SoundMgr::ms_Psg.write(data);
	}
}


/** Write Word functions. **/


/**
 * M68K_Write_Byte_SRam(): Write a word to SRam.
 * @param address Address.
 * @param data Word to write.
 */
inline void M68K_Mem::M68K_Write_Word_SRam(uint32_t address, uint16_t data)
{
	if (!EmuContext::GetSaveDataEnable())
	{
		// Save data is disabled.
		return;
	}
	
	// Temporarily needed because M68K_Mem is static.
	EEPRom *eeprom = EmuContext::GetEEPRom();
	SRam *sram = EmuContext::GetSRam();
	
	if (eeprom->isEEPRomTypeSet())
	{
		// EEPRom is enabled.
		if (eeprom->isWriteWordPort(address))
		{
			// EEPRom write port.
			return eeprom->writeWord(address, data);
		}
	}
	else if (sram->canWrite() && sram->isAddressInRange(address))
	{
		// SRam data request.
		
		// Write the word to SRam.
		// NOTE: SRam is NOT byteswapped.
		// TODO: Check boundaries.
		// TODO: Should start/end addressing be handled here or in SRam?
		sram->writeWord(address, data);
	}
}


/**
 * M68K_Write_Word_Ram(): Write a word to RAM. (0xE00000 - 0xFFFFFF)
 * RAM is 64 KB, mirrored throughout the entire range.
 * @param address Address.
 * @param data Word to write.
 */
inline void M68K_Mem::M68K_Write_Word_Ram(uint32_t address, uint16_t data)
{
	address &= 0xFFFE;
	Ram_68k.u16[address >> 1] = data;
}


/**
 * M68K_Write_Word_Misc(): Write a word to the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @param data Word to write.
 */
inline void M68K_Mem::M68K_Write_Word_Misc(uint32_t address, uint16_t data)
{
	if (address <= 0xA0FFFF)
	{
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET))
		{
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			return;
		}
		
		// Call the Z80 Write Byte function.
		// TODO: CPU lockup on accessing 0x7Fxx or >=0x8000.
		// Genesis Plus writes the high byte of the M68K word.
		// NOTE: Gunstar Heroes uses word write access to the Z80 area on startup.
		Z80_MD_Mem::Z80_WriteB(address & 0xFFFF, (data >> 8) & 0xFF);
		return;
	}
	else if (address == 0xA11100)
	{
		// Z80 BUSREQ. (0xA11100)
		// NOTE: Genesis Plus does BUSREQ at any even 0xA111xx...
		// NOTE: We're doing it at odd addresses!
		// TODO: Combine with Byte Write version?
		
		// Zero the controller counters and delays.
		// NOTE: Genesis Plus doesn't do this, and it doesn't make sense.
		// The controller counters and delays are located in the controller,
		// not in the system.
#if 0
		xor	ecx, ecx
		mov	al, [SYM(Z80_State)]
		mov	dword [SYM(Controller_1_Counter)], ecx
		test	ah, 1	; TODO: Should this be al, Z80_STATE_ENABLED ?
		mov	dword [SYM(Controller_1_Delay)], ecx
		mov	dword [SYM(Controller_2_Counter)], ecx
		mov	dword [SYM(Controller_2_Delay)], ecx
#endif
		
		// NOTE: Test data against 0x0100, since 68000 is big-endian.
		if (data & 0x0100)
		{
			// M68K requests the bus.
			// Disable the Z80.
			Last_BUS_REQ_Cnt = M68K::ReadOdometer();
			Last_BUS_REQ_St = (Z80_State & Z80_STATE_BUSREQ);
			
			if (Z80_State & Z80_STATE_BUSREQ)
			{
				// Z80 is running. Disable it.
				Z80_State &= ~Z80_STATE_BUSREQ;
				
				// TODO: Rework this.
				int ebx = (Cycles_M68K - Last_BUS_REQ_Cnt);
				ebx = Z80_M68K_Cycle_Tab[ebx];
				
				int edx = Cycles_Z80;
				edx -= ebx;
				Z80::Exec(edx);
			}
		}
		else
		{
			// M68K releases the bus.
			// Enable the Z80.
			if (!(Z80_State & Z80_STATE_BUSREQ))
			{
				// Z80 is stopped. Enable it.
				Z80_State |= Z80_STATE_BUSREQ;
				
				// TODO: Rework this.
				int ebx = Cycles_M68K;
				ebx -= M68K::ReadOdometer();
				
				int edx = Cycles_Z80;
				ebx = Z80_M68K_Cycle_Tab[ebx];
				edx -= ebx;
				
				// Set the Z80 odometer.
				Z80::SetOdometer((unsigned int)edx);
			}
		}
		
		return;
	}
	else if (address == 0xA11200)
	{
		// Z80 RESET. (0xA11200)
		// NOTE: Genesis Plus does RESET at any even 0xA112xx...
		// NOTE: We're doing it at odd addresses!
		
		// NOTE: Test data against 0x0100, since 68000 is big-endian.
		if (data & 0x0100)
		{
			// RESET is high. Start the Z80.
			Z80_State &= ~Z80_STATE_RESET;
		}
		else
		{
			// RESET is low. Stop the Z80.
			Z80::Reset();
			Z80_State |= Z80_STATE_RESET;
			
			// YM2612's RESET line is tied to the Z80's RESET line.
			SoundMgr::ms_Ym2612.reset();
		}
		
		return;
	}
	else if (address == 0xA130F0)
	{
		// SRam control register. (0xA130F0/0xA130F1)
		// NOTE: NOT 0xA130F1 - this is a word write.
		
		// Temporarily needed because M68K_Mem is static.
		SRam *sram = EmuContext::GetSRam();
		
		sram->writeCtrl(data);
		return;
	}
	else if (address >= 0xA130F2 && address <= 0xA130FF)
	{
		// Super Street Fighter II (SSF2) bankswitching system.
		// TODO: Port to integrated dispatch function.
#if 0
		// TODO: Starscream doesn't use this for instruction fetch!
		// TODO: Use a helper class?
		// TODO: Only banks 0-9 are supported right now...
		unsigned int phys_bank = (address & 0xF) >> 1;
		unsigned int virt_bank = (data & 0x1F);
		
		if (virt_bank > 9)
		{
			// TODO: We're ignoring banks over bank 9.
			virt_bank = phys_bank;
			ms_SSF2_BankState[phys_bank] = 0xFF;	// default bank
		}
		else
		{
			// Save the bank number.
			ms_SSF2_BankState[phys_bank] = virt_bank;
		}
		
		// Set the banking in the read byte/word tables.
		M68K_Read_Byte_Table[phys_bank] = MD_M68K_Read_Byte_Table[virt_bank];
		M68K_Read_Word_Table[phys_bank] = MD_M68K_Read_Word_Table[virt_bank];
#endif
		return;
	}
	else if (address > 0xA1001F)
	{
		// Invalid address.
		return;
	}
	
	/**
	 * MD miscellaneous registers.
	 * 0xA10001: Genesis version register.
	 * 0xA10003: Control Port 1: Data.
	 * 0xA10005: Control Port 2: Data.
	 * 0xA10007: Control Port 3: Data. (EXT)
	 * 0xA10009: Control Port 1: CTRL.
	 * 0xA1000B: Control Port 2: CTRL.
	 * 0xA1000D: Control Port 3: CTRL. (EXT)
	 * 0xA1000F: Control Port 1: Serial TxData.
	 * 0xA10011: Control Port 1: Serial RxData. (READ-ONLY)
	 * 0xA10013: Control Port 1: Serial Control.
	 * 0xA10015: Control Port 2: Serial TxData.
	 * 0xA10017: Control Port 2: Serial RxData. (READ-ONLY)
	 * 0xA10019: Control Port 2: Serial Control.
	 * 0xA1001B: Control Port 3: Serial TxData.
	 * 0xA1001D: Control Port 3: Serial RxData. (READ-ONLY)
	 * 0xA1001F: Control Port 3: Serial Control.
	 */
	switch (address & 0x1E)
	{
		case 0x00: /// 0xA10001: Genesis version register.
		default:
			break;
		
		// Parallel I/O
		case 0x02:	EmuMD::m_port1->writeData(data & 0xFF); break;
		case 0x04:	EmuMD::m_port2->writeData(data & 0xFF); break;
		case 0x06:	EmuMD::m_portE->writeData(data & 0xFF); break;
		case 0x08:	EmuMD::m_port1->writeCtrl(data & 0xFF); break;
		case 0x0A:	EmuMD::m_port2->writeCtrl(data & 0xFF); break;
		case 0x0C:	EmuMD::m_portE->writeCtrl(data & 0xFF); break;
		
		// Serial I/O
		// TODO: Baud rate handling, etc.
		case 0x0E:	EmuMD::m_port1->writeSerTx(data & 0xFF); break;
		case 0x10:	break; // READ-ONLY
		case 0x12:	EmuMD::m_port1->writeSerCtrl(data & 0xFF); break;
		case 0x14:	EmuMD::m_port2->writeSerTx(data & 0xFF); break;
		case 0x16:	break; // READ-ONLY
		case 0x18:	EmuMD::m_port2->writeSerCtrl(data & 0xFF); break;
		case 0x1A:	EmuMD::m_portE->writeSerTx(data & 0xFF); break;
		case 0x1C:	break; // READ-ONLY
		case 0x1E:	EmuMD::m_portE->writeSerCtrl(data & 0xFF); break;
	}
}


/**
 * M68K_Write_Word_VDP(): Write a word to the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @param data Word to write.
 */
inline void M68K_Mem::M68K_Write_Word_VDP(uint32_t address, uint16_t data)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.
	
	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0)
	{
		// Not a valid VDP address.
		return;
	}
	
	// Check the VDP address.
	address &= 0x1F;
	if (address < 0x04)
	{
		// 0xC00000 - 0xC00003: VDP Data Port.
		VdpIo::Write_Data_Word(data);
	}
	else if (address < 0x08)
	{
		// 0xC00004 - 0xC00007: VDP Control Port.
		VdpIo::Write_Ctrl(data);
	}
	else if (address == 0x11)
	{
		// 0xC00011: PSG control port.
		// TODO: mem_m68k.asm doesn't support this for word writes...
		//m_Psg.write(data);
	}
}


/** Public init and read/write functions. **/


/**
 * M68K_Mem::InitSys(): Initialize the M68K memory handler.
 * @param system System ID.
 */
void M68K_Mem::InitSys(M68K::SysID system)
{
	// Initialize the SSF2 bankswitching state.
	memset(ms_SSF2_BankState, 0xFF, sizeof(ms_SSF2_BankState));
}


/**
 * M68K_Mem::M68K_RB(): Read a byte from the M68K address space.
 * @param address Address.
 * @return Byte from the M68K address space.
 */
uint8_t M68K_Mem::M68K_RB(uint32_t address)
{
	// TODO: This is MD only. Add MCD/32X later.
	// TODO: SSF2 bankswitching.
	address &= 0xFFFFFF;
	const uint8_t page = (address >> 19);
	
	switch (page & 0x1F)
	{
		case 0x00:	return M68K_Read_Byte_Rom(address);
		case 0x01:	return M68K_Read_Byte_Rom(address);
		case 0x02:	return M68K_Read_Byte_Rom(address);
		case 0x03:	return M68K_Read_Byte_Rom(address);
		
		case 0x04:	return T_M68K_Read_Byte_RomX_SRam<0x4>(address);
		case 0x05:	return M68K_Read_Byte_Rom(address);
		case 0x06:	return T_M68K_Read_Byte_RomX_SRam<0x6>(address);
		case 0x07:	return T_M68K_Read_Byte_RomX_SRam<0x7>(address);
		
		case 0x08:	return 0xFF;
		case 0x09:	return 0xFF;
		case 0x0A:	return 0xFF;
		case 0x0B:	return 0xFF;
		
		case 0x0C:	return 0xFF;
		case 0x0D:	return 0xFF;
		case 0x0E:	return 0xFF;
		case 0x0F:	return 0xFF;
		
		case 0x10:	return 0xFF;
		case 0x11:	return 0xFF;
		case 0x12:	return 0xFF;
		case 0x13:	return 0xFF;
		
		case 0x14:	return M68K_Read_Byte_Misc(address);
		case 0x15:	return 0xFF;
		case 0x16:	return 0xFF;
		case 0x17:	return 0xFF;
		
		case 0x18:	return M68K_Read_Byte_VDP(address);
		case 0x19:	return M68K_Read_Byte_VDP(address);
		case 0x1A:	return M68K_Read_Byte_VDP(address);
		case 0x1B:	return M68K_Read_Byte_VDP(address);
		
		case 0x1C:	return M68K_Read_Byte_Ram(address);
		case 0x1D:	return M68K_Read_Byte_Ram(address);
		case 0x1E:	return M68K_Read_Byte_Ram(address);
		case 0x1F:	return M68K_Read_Byte_Ram(address);
	}
	
	// Should not get here...
	return 0xFF;
}


/**
 * M68K_Mem::M68K_RW(): Read a word from the M68K address space.
 * @param address Address.
 * @return Word from the M68K address space.
 */
uint16_t M68K_Mem::M68K_RW(uint32_t address)
{
	// TODO: This is MD only. Add MCD/32X later.
	// TODO: SSF2 bankswitching.
	address &= 0xFFFFFF;
	const uint8_t page = (address >> 19);
	
	switch (page & 0x1F)
	{
		case 0x00:	return M68K_Read_Word_Rom(address);
		case 0x01:	return M68K_Read_Word_Rom(address);
		case 0x02:	return M68K_Read_Word_Rom(address);
		case 0x03:	return M68K_Read_Word_Rom(address);
		
		case 0x04:	return T_M68K_Read_Word_RomX_SRam<0x4>(address);
		case 0x05:	return M68K_Read_Word_Rom(address);
		case 0x06:	return T_M68K_Read_Word_RomX_SRam<0x6>(address);
		case 0x07:	return T_M68K_Read_Word_RomX_SRam<0x7>(address);
		
		case 0x08:	return 0xFFFF;
		case 0x09:	return 0xFFFF;
		case 0x0A:	return 0xFFFF;
		case 0x0B:	return 0xFFFF;
		
		case 0x0C:	return 0xFFFF;
		case 0x0D:	return 0xFFFF;
		case 0x0E:	return 0xFFFF;
		case 0x0F:	return 0xFFFF;
		
		case 0x10:	return 0xFFFF;
		case 0x11:	return 0xFFFF;
		case 0x12:	return 0xFFFF;
		case 0x13:	return 0xFFFF;
		
		case 0x14:	return M68K_Read_Word_Misc(address);
		case 0x15:	return 0xFFFF;
		case 0x16:	return 0xFFFF;
		case 0x17:	return 0xFFFF;
		
		case 0x18:	return M68K_Read_Word_VDP(address);
		case 0x19:	return M68K_Read_Word_VDP(address);
		case 0x1A:	return M68K_Read_Word_VDP(address);
		case 0x1B:	return M68K_Read_Word_VDP(address);
		
		case 0x1C:	return M68K_Read_Word_Ram(address);
		case 0x1D:	return M68K_Read_Word_Ram(address);
		case 0x1E:	return M68K_Read_Word_Ram(address);
		case 0x1F:	return M68K_Read_Word_Ram(address);
	}
	
	// Should not get here...
	return 0xFFFF;
}


/**
 * M68K_Mem::M68K_WB(): Write a byte to the M68K address space.
 * @param address Address.
 * @param data Byte to write.
 */
void M68K_Mem::M68K_WB(uint32_t address, uint8_t data)
{
	// TODO: This is MD only. Add MCD/32X later.
	// TODO: SSF2 bankswitching.
	address &= 0xFFFFFF;
	const uint8_t page = (address >> 21);
	
	switch (page & 0x07)
	{
		case 0x00:	M68K_Write_Byte_SRam(address, data); break;
		case 0x01:	M68K_Write_Byte_SRam(address, data); break;
		case 0x02:	break;
		case 0x03:	break;
		case 0x04:	break;
		case 0x05:	M68K_Write_Byte_Misc(address, data); break;
		case 0x06:	M68K_Write_Byte_VDP(address, data); break;
		case 0x07:	M68K_Write_Byte_Ram(address, data); break;
	}
}


/**
 * M68K_Mem::M68K_WW(): Write a word to the M68K address space.
 * @param address Address.
 * @param data Word to write.
 */
void M68K_Mem::M68K_WW(uint32_t address, uint16_t data)
{
	// TODO: This is MD only. Add MCD/32X later.
	// TODO: SSF2 bankswitching.
	address &= 0xFFFFFF;
	const uint8_t page = (address >> 21);
	
	switch (page & 0x07)
	{
		case 0x00:	M68K_Write_Word_SRam(address, data); break;
		case 0x01:	M68K_Write_Word_SRam(address, data); break;
		case 0x02:	break;
		case 0x03:	break;
		case 0x04:	break;
		case 0x05:	M68K_Write_Word_Misc(address, data); break;
		case 0x06:	M68K_Write_Word_VDP(address, data); break;
		case 0x07:	M68K_Write_Word_Ram(address, data); break;
	}
}


/** ZOMG savestate functions. */


/**
 * ZomgSaveSSF2BankState(): Save the SSF2 bankswitching state.
 * @param state Zomg_MD_TimeReg_t struct to save to.
 */
void M68K_Mem::ZomgSaveSSF2BankState(Zomg_MD_TimeReg_t *state)
{
	// TODO: Only save if SSF2 bankswitching is active.
	state->SSF2_bank1 = ms_SSF2_BankState[1];
	state->SSF2_bank2 = ms_SSF2_BankState[2];
	state->SSF2_bank3 = ms_SSF2_BankState[3];
	state->SSF2_bank4 = ms_SSF2_BankState[4];
	state->SSF2_bank5 = ms_SSF2_BankState[5];
	state->SSF2_bank6 = ms_SSF2_BankState[6];
	state->SSF2_bank7 = ms_SSF2_BankState[7];
}


/**
 * ZomgRestoreSSF2BankState(): Restore the SSF2 bankswitching state.
 * @param state Zomg_MD_TimeReg_t struct to restore from.
 */
void M68K_Mem::ZomgRestoreSSF2BankState(const Zomg_MD_TimeReg_t *state)
{
	// TODO: Only load if SSF2 bankswitching is active.
	ms_SSF2_BankState[0] = 0xFF; // sanity-checking
	ms_SSF2_BankState[1] = state->SSF2_bank1;
	ms_SSF2_BankState[2] = state->SSF2_bank2;
	ms_SSF2_BankState[3] = state->SSF2_bank3;
	ms_SSF2_BankState[4] = state->SSF2_bank4;
	ms_SSF2_BankState[5] = state->SSF2_bank5;
	ms_SSF2_BankState[6] = state->SSF2_bank6;
	ms_SSF2_BankState[7] = state->SSF2_bank7;
	
	// Verify the bank states.
	// NOTE: Only banks 0-9 are supported right now.
	for (int phys_bank = 0; phys_bank < 8; phys_bank++)
	{
		unsigned int virt_bank = ms_SSF2_BankState[phys_bank];
		if (virt_bank > 9)
		{
			// TODO: We're ignoring banks over bank 9.
			// This also catches 0xFF, which indicates "default bank".
			virt_bank = phys_bank;
			ms_SSF2_BankState[phys_bank] = 0xFF;	// default bank
		}
		
		// Set the banking in the read byte/word tables.
		// TODO: Port SSF2 bankswitching to the dispatch functions.
#if 0
		M68K_Read_Byte_Table[phys_bank] = MD_M68K_Read_Byte_Table[virt_bank];
		M68K_Read_Word_Table[phys_bank] = MD_M68K_Read_Word_Table[virt_bank];
#endif
	}
}

}
