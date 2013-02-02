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
#include "Vdp/Vdp.hpp"

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

// Miscellaneous.
#include "../Util/byteswap.h"
#include "../macros/log_msg.h"

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

/**
 * ms_SSF2_BankState[]: SSF2 bankswitching state.
 * TODO: Make a helper class for this?
 */
uint8_t M68K_Mem::ms_SSF2_BankState[8];

/**
 * ms_M68KBank_Type[]: M68K bank type identifiers.
 * These type identifiers indicate what's mapped to each virtual bank.
 * Banks are 512 KB each, for a total of 32 banks.
 */
uint8_t M68K_Mem::ms_M68KBank_Type[32];
		
/**
 * msc_M68KBank_Def_MD[]: Default M68K bank type IDs for MD.
 */
const uint8_t M68K_Mem::msc_M68KBank_Def_MD[32] =
{
	// $000000 - $3FFFFF: ROM banks.
	M68K_BANK_ROM_0, M68K_BANK_ROM_1, M68K_BANK_ROM_2, M68K_BANK_ROM_3,
	M68K_BANK_ROM_4, M68K_BANK_ROM_5, M68K_BANK_ROM_6, M68K_BANK_ROM_7,
	
	// $400000 - $7FFFFF: Unused. (Sega CD)
	// Xin Qi Gai Wangzi (Beggar Prince) has SRAM at $400000 - $40FFFF.
	M68K_BANK_SRAM, M68K_BANK_UNUSED, M68K_BANK_UNUSED, M68K_BANK_UNUSED,
	M68K_BANK_UNUSED, M68K_BANK_UNUSED, M68K_BANK_UNUSED, M68K_BANK_UNUSED,
	
	// $800000 - $9FFFFF: Unused. (Sega 32X)
	M68K_BANK_UNUSED, M68K_BANK_UNUSED, M68K_BANK_UNUSED, M68K_BANK_UNUSED,
	
	// $A00000 - $A7FFFF: I/O area.
	// $A80000 - $BFFFFF: Unused.
	M68K_BANK_IO, M68K_BANK_UNUSED, M68K_BANK_UNUSED, M68K_BANK_UNUSED,
	
	// $C00000 - $DFFFFF: VDP. (specialized mirroring)
	M68K_BANK_VDP, M68K_BANK_VDP, M68K_BANK_VDP, M68K_BANK_VDP,
	
	// $E00000 - $FFFFFF: RAM. (64K mirroring)
	M68K_BANK_RAM, M68K_BANK_RAM, M68K_BANK_RAM, M68K_BANK_RAM
};


void M68K_Mem::Init(void)
{
	// Initialize the Z80/M68K cycle table.
	for (int x = 0; x < 512; x++)
		Z80_M68K_Cycle_Tab[x] = (int)((double) x * 7.0 / 15.0);
	
	// Initialize the SSF2 bankswitching state.
	memset(ms_SSF2_BankState, 0xFF, sizeof(ms_SSF2_BankState));
}


void M68K_Mem::End(void)
{
}


/** Read Byte functions. **/

/**
 * BYTE_ADDR_INVERT: Inversion flag for byte-addressing.
 * ROM and RAM is stored in host 16-bit endian.
 * Hence, bytewide access needs to have the LSB inverted
 * on little-endian machines.
 */
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
#define BYTE_ADDR_INVERT 1
#else /* GENS_BYTEORDER = GENS_BIG_ENDIAN */
#define BYTE_ADDR_INVERT 0
#endif

/**
 * T_M68K_Read_Byte_Rom(): Read a byte from ROM.
 * @param bank Physical ROM bank.
 * @param address Address.
 * @return Byte from ROM.
 */
template<uint8_t bank>
inline uint8_t M68K_Mem::T_M68K_Read_Byte_Rom(uint32_t address)
{
	address &= 0x7FFFF;
	address ^= ((bank << 19) | BYTE_ADDR_INVERT);
	return Rom_Data.u8[address];
}
// TODO: Add banks C, D, E, and F for 8 MB ROM support.
// For now, they will return 0x00.


/**
 * T_M68K_Read_Byte_Rom_SRam(): Read a byte from ROM or SRam/EEPRom.
 * TODO: Verify that this works for 0x300000/0x380000.
 * @param bank Physical ROM bank. (-1 for SRAM only)
 * @param address Address.
 * @return Byte from ROM or SRam/EEPRom.
 */
template<int8_t bank>
inline uint8_t M68K_Mem::T_M68K_Read_Byte_Rom_SRam(uint32_t address)
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
	
	// Not SRam/EEPRom. Return the ROM data.
	if (bank >= 0)
		return T_M68K_Read_Byte_Rom<bank>(address);
	return 0xFF;
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
	switch (address & 0x1E) {
		case 0x00: {
			// 0xA10001: Genesis version register.
			EmuContext *context = EmuContext::Instance();
			if (!context)
				return 0;
			return context->readVersionRegister_MD();
		}

		// Parallel I/O
		case 0x02:	return EmuMD::m_ioManager->readDataMD(IoManager::PHYSPORT_1);
		case 0x04:	return EmuMD::m_ioManager->readDataMD(IoManager::PHYSPORT_2);
		case 0x06:	return EmuMD::m_ioManager->readDataMD(IoManager::PHYSPORT_EXT);
		case 0x08:	return EmuMD::m_ioManager->readCtrlMD(IoManager::PHYSPORT_1);
		case 0x0A:	return EmuMD::m_ioManager->readCtrlMD(IoManager::PHYSPORT_2);
		case 0x0C:	return EmuMD::m_ioManager->readCtrlMD(IoManager::PHYSPORT_EXT);

		// Serial I/O
		// TODO: Baud rate handling, etc.
		case 0x0E:	return EmuMD::m_ioManager->readSerTx(IoManager::PHYSPORT_1);
		case 0x10:	return EmuMD::m_ioManager->readSerRx(IoManager::PHYSPORT_1);
		case 0x12:	return EmuMD::m_ioManager->readSerCtrl(IoManager::PHYSPORT_1);
		case 0x14:	return EmuMD::m_ioManager->readSerTx(IoManager::PHYSPORT_2);
		case 0x16:	return EmuMD::m_ioManager->readSerRx(IoManager::PHYSPORT_2);
		case 0x18:	return EmuMD::m_ioManager->readSerCtrl(IoManager::PHYSPORT_2);
		case 0x1A:	return EmuMD::m_ioManager->readSerTx(IoManager::PHYSPORT_EXT);
		case 0x1C:	return EmuMD::m_ioManager->readSerRx(IoManager::PHYSPORT_EXT);
		case 0x1E:	return EmuMD::m_ioManager->readSerCtrl(IoManager::PHYSPORT_EXT);

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
	
	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return 0;
	
	// Check the VDP address.
	switch (address & 0x1F)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
			// VDP data port.
			// FIXME: Gens doesn't read the data port here.
			// It should still be readable...
			return 0x00;
		
		case 0x04: case 0x06:
		{
			// VDP control port. (high byte)
			uint16_t vdp_status = context->m_vdp->Read_Status();
			return ((vdp_status >> 8) & 0xFF);
		}
		
		case 0x05: case 0x07:
		{
			// VDP control port. (low byte)
			uint16_t vdp_status = context->m_vdp->Read_Status();
			return (vdp_status & 0xFF);
		}
		
		case 0x08:
			// V counter.
			return context->m_vdp->Read_V_Counter();
		
		case 0x09:
			// H counter.
			return context->m_vdp->Read_H_Counter();
		
		default:
			// Invalid or unsupported VDP port.
			return 0x00;
	}
	
	// Should not get here...
	return 0x00;
}


/** Read Word functions. **/


/**
 * T_M68K_Read_Word_Rom(): Read a word from ROM.
 * @param bank Physical ROM bank.
 * @param address Address.
 * @return Word from ROM.
 */
template<uint8_t bank>
inline uint16_t M68K_Mem::T_M68K_Read_Word_Rom(uint32_t address)
{
	address &= 0x7FFFF;
	address |= (bank << 19);
	return Rom_Data.u16[address >> 1];
}
// TODO: Add banks C, D, E, and F for 8 MB ROM support.
// For now, they will return 0x00.


/**
 * T_M68K_Read_Word_Rom_SRam(): Read a word from ROM or SRam/EEPRom.
 * TODO: Verify that this works for 0x300000/0x380000.
 * @param bank Physical ROM bank. (-1 for SRAM only)
 * @param address Address.
 * @return Word from ROM or SRam/EEPRom.
 */
template<int8_t bank>
inline uint16_t M68K_Mem::T_M68K_Read_Word_Rom_SRam(uint32_t address)
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
	
	// Not SRam/EEPRom. Return the ROM data.
	if (bank >= 0)
		return T_M68K_Read_Word_Rom<bank>(address);
	return 0xFFFF;
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
	switch (address & 0x1E) {
		case 0x00: {
			// 0xA10001: Genesis version register.
			EmuContext *context = EmuContext::Instance();
			if (!context)
				return 0;
			return context->readVersionRegister_MD();
		}

		// Parallel I/O
		case 0x02:	return EmuMD::m_ioManager->readDataMD(IoManager::PHYSPORT_1);
		case 0x04:	return EmuMD::m_ioManager->readDataMD(IoManager::PHYSPORT_2);
		case 0x06:	return EmuMD::m_ioManager->readDataMD(IoManager::PHYSPORT_EXT);
		case 0x08:	return EmuMD::m_ioManager->readCtrlMD(IoManager::PHYSPORT_1);
		case 0x0A:	return EmuMD::m_ioManager->readCtrlMD(IoManager::PHYSPORT_2);
		case 0x0C:	return EmuMD::m_ioManager->readCtrlMD(IoManager::PHYSPORT_EXT);

		// Serial I/O
		// TODO: Baud rate handling, etc.
		case 0x0E:	return EmuMD::m_ioManager->readSerTx(IoManager::PHYSPORT_1);
		case 0x10:	return EmuMD::m_ioManager->readSerRx(IoManager::PHYSPORT_1);
		case 0x12:	return EmuMD::m_ioManager->readSerCtrl(IoManager::PHYSPORT_1);
		case 0x14:	return EmuMD::m_ioManager->readSerTx(IoManager::PHYSPORT_2);
		case 0x16:	return EmuMD::m_ioManager->readSerRx(IoManager::PHYSPORT_2);
		case 0x18:	return EmuMD::m_ioManager->readSerCtrl(IoManager::PHYSPORT_2);
		case 0x1A:	return EmuMD::m_ioManager->readSerTx(IoManager::PHYSPORT_EXT);
		case 0x1C:	return EmuMD::m_ioManager->readSerRx(IoManager::PHYSPORT_EXT);
		case 0x1E:	return EmuMD::m_ioManager->readSerCtrl(IoManager::PHYSPORT_EXT);

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
		return 0x0000;
	}
	
	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return 0;
	
	// Check the VDP address.
	switch (address & 0x1E)
	{
		case 0x00: case 0x02:
			// VDP data port.
			return context->m_vdp->Read_Data();
		
		case 0x04: case 0x06:
			// VDP control port.
			return context->m_vdp->Read_Status();
		
		case 0x08:
			// HV counter.
			return ((context->m_vdp->Read_V_Counter() << 8) | context->m_vdp->Read_H_Counter());
		
		default:
			// Invalid or unsupported VDP port.
			return 0x0000;
	}
	
	// Should not get here...
	return 0x0000;
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
			Z80::SoftReset();
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
		// TODO: Starscream doesn't use this for instruction fetch!
		// TODO: Use a helper class?
		const uint8_t phys_bank = (address & 0xF) >> 1;
		uint8_t virt_bank = (data & 0x1F);
		
		// SSF2_NUM_BANKS == number of SSF2 banks supported by Gens/GS II's implementation.
		if (virt_bank >= SSF2_NUM_BANKS)
		{
			// Bank isn't supported.
			virt_bank = phys_bank;
			ms_SSF2_BankState[phys_bank] = 0xFF;	// default bank
		}
		else
		{
			// Save the bank number.
			ms_SSF2_BankState[phys_bank] = virt_bank;
		}
		
		// Update the M68K bank type identifiers.
		ms_M68KBank_Type[phys_bank] = (M68K_BANK_ROM_0 + virt_bank);
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
		case 0x02:	EmuMD::m_ioManager->writeDataMD(IoManager::PHYSPORT_1, data); break;
		case 0x04:	EmuMD::m_ioManager->writeDataMD(IoManager::PHYSPORT_2, data); break;
		case 0x06:	EmuMD::m_ioManager->writeDataMD(IoManager::PHYSPORT_EXT, data); break;
		case 0x08:	EmuMD::m_ioManager->writeCtrlMD(IoManager::PHYSPORT_1, data); break;
		case 0x0A:	EmuMD::m_ioManager->writeCtrlMD(IoManager::PHYSPORT_2, data); break;
		case 0x0C:	EmuMD::m_ioManager->writeCtrlMD(IoManager::PHYSPORT_EXT, data); break;

		// Serial I/O
		// TODO: Baud rate handling, etc.
		case 0x0E:	EmuMD::m_ioManager->writeSerTx(IoManager::PHYSPORT_1, data); break;
		case 0x10:	break; // READ-ONLY
		case 0x12:	EmuMD::m_ioManager->writeSerCtrl(IoManager::PHYSPORT_1, data); break;
		case 0x14:	EmuMD::m_ioManager->writeSerTx(IoManager::PHYSPORT_2, data); break;
		case 0x16:	break; // READ-ONLY
		case 0x18:	EmuMD::m_ioManager->writeSerCtrl(IoManager::PHYSPORT_2, data); break;
		case 0x1A:	EmuMD::m_ioManager->writeSerTx(IoManager::PHYSPORT_EXT, data); break;
		case 0x1C:	break; // READ-ONLY
		case 0x1E:	EmuMD::m_ioManager->writeSerCtrl(IoManager::PHYSPORT_EXT, data); break;
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
	
	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return;
	
	// Check the VDP address.
	switch (address & 0x1F)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
			// VDP data port.
			context->m_vdp->Write_Data_Byte(data);
			break;
		
		case 0x04: case 0x05: case 0x06: case 0x07:
			// VDP control port.
			// TODO: This should still be writable.
			// Gens' mem_m68k.asm doesn't implement this.
			break;
		
		case 0x11:
			// PSG control port.
			SoundMgr::ms_Psg.write(data);
			break;
		
		default:
			// Invalid or unsupported VDP port.
			break;
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
			Z80::SoftReset();
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
		// TODO: Starscream doesn't use this for instruction fetch!
		// TODO: Use a helper class?
		const uint8_t phys_bank = (address & 0xF) >> 1;
		uint8_t virt_bank = (data & 0x1F);
		
		// SSF2_NUM_BANKS == number of SSF2 banks supported by Gens/GS II's implementation.
		if (virt_bank >= SSF2_NUM_BANKS)
		{
			// Bank isn't supported.
			virt_bank = phys_bank;
			ms_SSF2_BankState[phys_bank] = 0xFF;	// default bank
		}
		else
		{
			// Save the bank number.
			ms_SSF2_BankState[phys_bank] = virt_bank;
		}
		
		// Update the M68K bank type identifiers.
		ms_M68KBank_Type[phys_bank] = (M68K_BANK_ROM_0 + virt_bank);
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
		case 0x02:	EmuMD::m_ioManager->writeDataMD(IoManager::PHYSPORT_1, data); break;
		case 0x04:	EmuMD::m_ioManager->writeDataMD(IoManager::PHYSPORT_2, data); break;
		case 0x06:	EmuMD::m_ioManager->writeDataMD(IoManager::PHYSPORT_EXT, data); break;
		case 0x08:	EmuMD::m_ioManager->writeCtrlMD(IoManager::PHYSPORT_1, data); break;
		case 0x0A:	EmuMD::m_ioManager->writeCtrlMD(IoManager::PHYSPORT_2, data); break;
		case 0x0C:	EmuMD::m_ioManager->writeCtrlMD(IoManager::PHYSPORT_EXT, data); break;

		// Serial I/O
		// TODO: Baud rate handling, etc.
		case 0x0E:	EmuMD::m_ioManager->writeSerTx(IoManager::PHYSPORT_1, data); break;
		case 0x10:	break; // READ-ONLY
		case 0x12:	EmuMD::m_ioManager->writeSerCtrl(IoManager::PHYSPORT_1, data); break;
		case 0x14:	EmuMD::m_ioManager->writeSerTx(IoManager::PHYSPORT_2, data); break;
		case 0x16:	break; // READ-ONLY
		case 0x18:	EmuMD::m_ioManager->writeSerCtrl(IoManager::PHYSPORT_2, data); break;
		case 0x1A:	EmuMD::m_ioManager->writeSerTx(IoManager::PHYSPORT_EXT, data); break;
		case 0x1C:	break; // READ-ONLY
		case 0x1E:	EmuMD::m_ioManager->writeSerCtrl(IoManager::PHYSPORT_EXT, data); break;
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
	
	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return;
	
	// Check the VDP address.
	switch (address & 0x1E)
	{
		case 0x00: case 0x02:
			// VDP data port.
			context->m_vdp->Write_Data_Word(data);
			break;
		
		case 0x04: case 0x06:
			// VDP control port.
			context->m_vdp->Write_Ctrl(data);
			break;
		
		case 0x10:
			// PSG control port.
			// TODO: mem_m68k.asm doesn't support this for word writes...
			//SoundMgr::ms_Psg.write(data);
			break;
		
		default:
			// Invalid or unsupported VDP port.
			break;
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
	
	// Initialize the M68K bank type identifiers.
	switch (system)
	{
		case M68K::SYSID_MD:
			memcpy(ms_M68KBank_Type, msc_M68KBank_Def_MD, sizeof(ms_M68KBank_Type));
			break;
		
		default:
			// Unknown system ID.
			LOG_MSG(68k, LOG_MSG_LEVEL_ERROR,
				"Unknwon system ID: %d", system);
			memset(ms_M68KBank_Type, 0x00, sizeof(ms_M68KBank_Type));
			break;
	}
}


/**
 * M68K_Mem::M68K_RB(): Read a byte from the M68K address space.
 * @param address Address.
 * @return Byte from the M68K address space.
 */
uint8_t M68K_Mem::M68K_RB(uint32_t address)
{
	// TODO: This is MD only. Add MCD/32X later.
	address &= 0xFFFFFF;
	const uint8_t bank = ((address >> 19) & 0x1F);
	
	// TODO: Optimize the switch using a bitwise AND.
	switch (ms_M68KBank_Type[bank])
	{
		default:
		case M68K_BANK_UNUSED:	return 0xFF;
		
		// Physical ROM bank.
		// TODO: SRam should be used for virtual banks 4, 6, and 7; not physical banks.
		case M68K_BANK_ROM_0:	return T_M68K_Read_Byte_Rom<0x00>(address);
		case M68K_BANK_ROM_1:	return T_M68K_Read_Byte_Rom<0x01>(address);
		case M68K_BANK_ROM_2:	return T_M68K_Read_Byte_Rom<0x02>(address);
		case M68K_BANK_ROM_3:	return T_M68K_Read_Byte_Rom<0x03>(address);
		case M68K_BANK_ROM_4:	return T_M68K_Read_Byte_Rom_SRam<0x04>(address);
		case M68K_BANK_ROM_5:	return T_M68K_Read_Byte_Rom<0x05>(address);
		case M68K_BANK_ROM_6:	return T_M68K_Read_Byte_Rom_SRam<0x06>(address);
		case M68K_BANK_ROM_7:	return T_M68K_Read_Byte_Rom_SRam<0x07>(address);
		case M68K_BANK_ROM_8:	return T_M68K_Read_Byte_Rom<0x08>(address);
		case M68K_BANK_ROM_9:	return T_M68K_Read_Byte_Rom<0x09>(address);
		case M68K_BANK_ROM_A:	return T_M68K_Read_Byte_Rom<0x0A>(address);
		case M68K_BANK_ROM_B:	return T_M68K_Read_Byte_Rom<0x0B>(address);
		case M68K_BANK_SRAM:	return T_M68K_Read_Byte_Rom_SRam<-1>(address);
		
		// Other MD banks.
		case M68K_BANK_IO:	return M68K_Read_Byte_Misc(address);
		case M68K_BANK_VDP:	return M68K_Read_Byte_VDP(address);
		case M68K_BANK_RAM:	return M68K_Read_Byte_Ram(address);
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
	address &= 0xFFFFFF;
	const uint8_t bank = ((address >> 19) & 0x1F);
	
	// TODO: Optimize the switch using a bitwise AND.
	switch (ms_M68KBank_Type[bank])
	{
		default:
		case M68K_BANK_UNUSED:	return 0xFFFF;
		
		// Physical ROM bank.
		// TODO: SRam should be used for virtual banks 4, 6, and 7; not physical banks.
		case M68K_BANK_ROM_0:	return T_M68K_Read_Word_Rom<0x00>(address);
		case M68K_BANK_ROM_1:	return T_M68K_Read_Word_Rom<0x01>(address);
		case M68K_BANK_ROM_2:	return T_M68K_Read_Word_Rom<0x02>(address);
		case M68K_BANK_ROM_3:	return T_M68K_Read_Word_Rom<0x03>(address);
		case M68K_BANK_ROM_4:	return T_M68K_Read_Word_Rom_SRam<0x04>(address);
		case M68K_BANK_ROM_5:	return T_M68K_Read_Word_Rom<0x05>(address);
		case M68K_BANK_ROM_6:	return T_M68K_Read_Word_Rom_SRam<0x06>(address);
		case M68K_BANK_ROM_7:	return T_M68K_Read_Word_Rom_SRam<0x07>(address);
		case M68K_BANK_ROM_8:	return T_M68K_Read_Word_Rom<0x08>(address);
		case M68K_BANK_ROM_9:	return T_M68K_Read_Word_Rom<0x09>(address);
		case M68K_BANK_ROM_A:	return T_M68K_Read_Word_Rom<0x0A>(address);
		case M68K_BANK_ROM_B:	return T_M68K_Read_Word_Rom<0x0B>(address);
		case M68K_BANK_SRAM:	return T_M68K_Read_Word_Rom_SRam<-1>(address);
		
		// Other MD banks.
		case M68K_BANK_IO:	return M68K_Read_Word_Misc(address);
		case M68K_BANK_VDP:	return M68K_Read_Word_VDP(address);
		case M68K_BANK_RAM:	return M68K_Read_Word_Ram(address);
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
	address &= 0xFFFFFF;
	const uint8_t bank = ((address >> 19) & 0x1F);
	
	// TODO: Optimize the switch using a bitwise AND.
	switch (ms_M68KBank_Type[bank])
	{
		default:
		case M68K_BANK_UNUSED:	break;
		
		// TODO: SRAM should use physical banks, not ROM banks.
		case M68K_BANK_ROM_0: case M68K_BANK_ROM_1:
		case M68K_BANK_ROM_2: case M68K_BANK_ROM_3:
		case M68K_BANK_ROM_4: case M68K_BANK_ROM_5:
		case M68K_BANK_ROM_6: case M68K_BANK_ROM_7:
		case M68K_BANK_ROM_8: case M68K_BANK_ROM_9:
		case M68K_BANK_ROM_A: case M68K_BANK_ROM_B:
		case M68K_BANK_SRAM:
			M68K_Write_Byte_SRam(address, data);
			break;
		
		// Other MD banks.
		case M68K_BANK_IO:	M68K_Write_Byte_Misc(address, data); break;
		case M68K_BANK_VDP:	M68K_Write_Byte_VDP(address, data); break;
		case M68K_BANK_RAM:	M68K_Write_Byte_Ram(address, data); break;
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
	address &= 0xFFFFFF;
	const uint8_t bank = ((address >> 19) & 0x1F);
	
	// TODO: Optimize the switch using a bitwise AND.
	switch (ms_M68KBank_Type[bank])
	{
		default:
		case M68K_BANK_UNUSED:	break;
		
		// TODO: SRAM should use physical banks, not ROM banks.
		case M68K_BANK_ROM_0: case M68K_BANK_ROM_1:
		case M68K_BANK_ROM_2: case M68K_BANK_ROM_3:
		case M68K_BANK_ROM_4: case M68K_BANK_ROM_5:
		case M68K_BANK_ROM_6: case M68K_BANK_ROM_7:
		case M68K_BANK_ROM_8: case M68K_BANK_ROM_9:
		case M68K_BANK_ROM_A: case M68K_BANK_ROM_B:
		case M68K_BANK_SRAM:
			M68K_Write_Word_SRam(address, data);
			break;
		
		// Other MD banks.
		case M68K_BANK_IO:	M68K_Write_Word_Misc(address, data); break;
		case M68K_BANK_VDP:	M68K_Write_Word_VDP(address, data); break;
		case M68K_BANK_RAM:	M68K_Write_Word_Ram(address, data); break;
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
	for (int phys_bank = 0; phys_bank < 8; phys_bank++)
	{
		uint8_t virt_bank = ms_SSF2_BankState[phys_bank];
		
		// SSF2_NUM_BANKS == number of SSF2 banks supported by Gens/GS II's implementation.
		if (virt_bank >= SSF2_NUM_BANKS)
		{
			// Bank isn't supported.
			// This also catches 0xFF, which indicates "default bank".
			virt_bank = phys_bank;
			ms_SSF2_BankState[phys_bank] = 0xFF;	// default bank
		}
		
		// Update the M68K bank type identifiers.
		ms_M68KBank_Type[phys_bank] = (M68K_BANK_ROM_0 + virt_bank);
	}
}

}
