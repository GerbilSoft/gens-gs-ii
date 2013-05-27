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

// ROM cartridge.
#include "Cartridge/RomCartridgeMD.hpp"

// C includes. (C++ namespace)
#include <cstring>

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

// ROM cartridge.
RomCartridgeMD *M68K_Mem::ms_RomCartridge = nullptr;

// TMSS ROM.
M68K_Mem::MD_TMSS_Rom_t M68K_Mem::MD_TMSS_Rom;

// TMSS registers.
TmssReg M68K_Mem::tmss_reg;

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
 * M68K bank type identifiers.
 * These type identifiers indicate what's mapped to each virtual bank.
 * Banks are 2 MB each, for a total of 8 banks.
 */
uint8_t M68K_Mem::ms_M68KBank_Type[8];

/**
 * Default M68K bank type IDs for MD.
 */
const uint8_t M68K_Mem::msc_M68KBank_Def_MD[8] =
{
	// $000000 - $9FFFFF: ROM cartridge.
	M68K_BANK_CARTRIDGE, M68K_BANK_CARTRIDGE,
	M68K_BANK_CARTRIDGE, M68K_BANK_CARTRIDGE,
	M68K_BANK_CARTRIDGE,

	// $A00000 - $BFFFFF: I/O area.
	M68K_BANK_IO,

	// $C00000 - $DFFFFF: VDP. (specialized mirroring)
	M68K_BANK_VDP,

	// $E00000 - $FFFFFF: RAM. (64K mirroring)
	M68K_BANK_RAM
};


void M68K_Mem::Init(void)
{
	// Initialize the Z80/M68K cycle table.
	for (int x = 0; x < 512; x++)
		Z80_M68K_Cycle_Tab[x] = (int)((double) x * 7.0 / 15.0);
}


void M68K_Mem::End(void)
{ }


/** Read Byte functions. **/

/**
 * Address inversion flags for byteswapped addressing.
 * - U16DATA_U8_INVERT: Access U8 data in host-endian 16-bit data.
 * - U32DATA_U8_INVERT: Access U8 data in host-endian 32-bit data.
 * - U32DATA_U16_INVERT: Access U16 data in host-endian 32-bit data.
 */
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
#define U16DATA_U8_INVERT 1
#define U32DATA_U8_INVERT 3
#define U32DATA_U16_INVERT 1
#else /* GENS_BYTEORDER = GENS_BIG_ENDIAN */
#define U16DATA_U8_INVERT 0
#define U32DATA_U8_INVERT 0
#define U32DATA_U16_INVERT 0
#endif

/**
 * Read a byte from RAM. (0xE00000 - 0xFFFFFF)
 * RAM is 64 KB, mirrored throughout the entire range.
 * @param address Address.
 * @return Byte from RAM.
 */
inline uint8_t M68K_Mem::M68K_Read_Byte_Ram(uint32_t address)
{
	address &= 0xFFFF;
	address ^= U16DATA_U8_INVERT;
	return Ram_68k.u8[address];
}

/**
 * Read a byte from the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @return Miscellaneous data byte.
 */
inline uint8_t M68K_Mem::M68K_Read_Byte_Misc(uint32_t address)
{
	if (address <= 0xA0FFFF) {
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET)) {
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			// TODO: Fake Fetch?
			return 0xFF;
		}

		// Call the Z80 Read Byte function.
		// TODO: CPU lockup on accessing 0x7Fxx or >=0x8000.
		return Z80_MD_Mem::Z80_ReadB(address & 0xFFFF);
	} else if (address >= 0xA20000) {
		// Invalid address.
		// TODO: Fake Fetch?
		return 0xFF;
	}

	// Check the second address byte to determine what to do.
	switch ((address >> 8) & 0xFF) {
		default:
			// Invalid address.
			// TODO: Fake Fetch?
			return 0xFF;

		case 0x11: {
			// 0xA11100: Z80 BUSREQ.
			// NOTE: Genesis Plus does BUSREQ at any even 0xA111xx...
			if (address & 1) {
				// FAKE FETCH.
				Fake_Fetch ^= 0xFF;
				return Fake_Fetch;
			}

			if (Z80_State & Z80_STATE_BUSREQ) {
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

		case 0x12:
			// 0xA11200: Z80 RESET.
			// NOTE: Not readable in Gens...
			// TODO: Fake Fetch?
			return 0xFF;

		case 0x30:
			// 0xA130xx: /TIME registers.
			return ms_RomCartridge->readByte_TIME(address & 0xFF);

		case 0x40: {
			// 0xA14000: TMSS ('SEGA' register)
			if (!tmss_reg.tmss_en) {
				// TMSS is disabled.
				// TODO: Fake Fetch?
				return 0xFF;
			}

			// Verify that this is a valid TMSS address.
			if ((address & 0xFF) > 0x03) {
				// Invalid TMSS address.
				// TODO: Fake Fetch?
				return 0xFF;
			}

			// 'SEGA' register.
			return tmss_reg.a14000.b[(address & 3) ^ U32DATA_U8_INVERT];
		}

		case 0x41: {
			// 0xA14101: TMSS (!CART_CE register)
			if (!tmss_reg.tmss_en) {
				// TMSS is disabled.
				// TODO: Fake Fetch?
				return 0xFF;
			}

			// Verify that this is a valid TMSS address.
			if ((address & 0xFF) != 0x01) {
				// Invalid TMSS address.
				// TODO: Fake Fetch?
				return 0xFF;
			}

			// !CART_CE register.
			return (tmss_reg.cart_ce & 1);
		}

		case 0x00: {
			// 0xA100xx: I/O registers.

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
			// NOTE: Reads from even addresses are handled the same as odd addresses.
			// (Least-significant bit is ignored.)
			uint8_t ret = 0xFF;
			const LibGens::IoManager *const ioManager = EmuMD::m_ioManager;
			switch (address & 0x1E) {
				case 0x00: {
					// 0xA10001: Genesis version register.
					EmuContext *context = EmuContext::Instance();
					if (context)
						ret = context->readVersionRegister_MD();
					break;
				}

				// Parallel I/O
				case 0x02:	ret = ioManager->readDataMD(IoManager::PHYSPORT_1); break;
				case 0x04:	ret = ioManager->readDataMD(IoManager::PHYSPORT_2); break;
				case 0x06:	ret = ioManager->readDataMD(IoManager::PHYSPORT_EXT); break;
				case 0x08:	ret = ioManager->readCtrlMD(IoManager::PHYSPORT_1); break;
				case 0x0A:	ret = ioManager->readCtrlMD(IoManager::PHYSPORT_2); break;
				case 0x0C:	ret = ioManager->readCtrlMD(IoManager::PHYSPORT_EXT); break;

				// Serial I/O
				// TODO: Baud rate handling, etc.
				case 0x0E:	ret = ioManager->readSerTx(IoManager::PHYSPORT_1); break;
				case 0x10:	ret = ioManager->readSerRx(IoManager::PHYSPORT_1); break;
				case 0x12:	ret = ioManager->readSerCtrl(IoManager::PHYSPORT_1); break;
				case 0x14:	ret = ioManager->readSerTx(IoManager::PHYSPORT_2); break;
				case 0x16:	ret = ioManager->readSerRx(IoManager::PHYSPORT_2); break;
				case 0x18:	ret = ioManager->readSerCtrl(IoManager::PHYSPORT_2); break;
				case 0x1A:	ret = ioManager->readSerTx(IoManager::PHYSPORT_EXT); break;
				case 0x1C:	ret = ioManager->readSerRx(IoManager::PHYSPORT_EXT); break;
				case 0x1E:	ret = ioManager->readSerCtrl(IoManager::PHYSPORT_EXT); break;

				default:
					// Unknown register.
					// TODO: Fake Fetch?
					break;
			}

			return ret;
		}
	}

	// Should not get here...
	// TODO: Fake Fetch?
	return 0xFF;
}


/**
 * Read a byte from the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @return VDP data byte.
 */
inline uint8_t M68K_Mem::M68K_Read_Byte_VDP(uint32_t address)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.

	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0) {
		// Not a valid VDP address.
		return 0x00;
	}

	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return 0;

	// Check the VDP address.
	switch (address & 0x1F) {
		case 0x00: case 0x01: case 0x02: case 0x03:
			// VDP data port.
			// FIXME: Gens doesn't read the data port here.
			// It should still be readable...
			return 0x00;

		case 0x04: case 0x06: {
			// VDP control port. (high byte)
			uint16_t vdp_status = context->m_vdp->Read_Status();
			return ((vdp_status >> 8) & 0xFF);
		}

		case 0x05: case 0x07: {
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


/**
 * Read a byte from the TMSS ROM. (0x000000 - 0x3FFFFF)
 * TMSS ROM is 2 KB, mirrored throughout the entire range.
 * @param address Address.
 * @return Byte from the TMSS ROM.
 */
inline uint8_t M68K_Mem::M68K_Read_Byte_TMSS_Rom(uint32_t address)
{
	address &= 0x7FF;
	address ^= U16DATA_U8_INVERT;
	return MD_TMSS_Rom.u8[address];
}


/** Read Word functions. **/


/**
 * Read a word from RAM. (0xE00000 - 0xFFFFFF)
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
 * Read a word from the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @return Miscellaneous data word.
 */
inline uint16_t M68K_Mem::M68K_Read_Word_Misc(uint32_t address)
{
	if (address <= 0xA0FFFF) {
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET)) {
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			// TODO: Fake Fetch?
			return 0xFFFF;
		}

		// Call the Z80 Read Byte function.
		// TODO: CPU lockup on accessing 0x7Fxx or >=0x8000.
		// Genesis Plus duplicates the byte in both halves of the M68K word.
		uint8_t ret = Z80_MD_Mem::Z80_ReadB(address & 0xFFFF);
		return (ret | (ret << 8));
	} else if (address >= 0xA20000) {
		// Invalid address.
		// TODO: Fake Fetch?
		return 0xFFFF;
	}

	// Check the second address byte to determine what to do.
	switch ((address >> 8) & 0xFF) {
		default:
			// Invalid address.
			// TODO: Fake Fetch?
			return 0xFFFF;

		case 0x11: {
			// 0xA11100: Z80 BUSREQ.
			// NOTE: Genesis Plus does BUSREQ at any even 0xA111xx...
			if (Z80_State & Z80_STATE_BUSREQ) {
				// Z80 is currently running.
				// NOTE: Low byte is supposed to be from
				// the next fetched instruction.
				Fake_Fetch ^= 0xFF;	// Fake the next fetched instruction. ("random")
				return (0x8100 | (Fake_Fetch & 0xFF));
			}

			// Z80 is not running.
			int odo68k = M68K::ReadOdometer();
			odo68k -= Last_BUS_REQ_Cnt;
			if (odo68k <= CYCLE_FOR_TAKE_Z80_BUS_GENESIS) {
				// bus not taken yet
				uint16_t ret;
				Fake_Fetch ^= 0xFF;	// Fake the next fetched instruction. ("random")
				ret = (Fake_Fetch & 0xFF);
				ret |= ((Last_BUS_REQ_St & 0xFF) << 8);
				ret += 0x8000;
				return ret;
			} else {
				// bus taken
				uint16_t ret;
				Fake_Fetch ^= 0xFF;	// Fake the next fetched instruction. ("random")
				ret = (Fake_Fetch & 0xFF) | 0x8000;
				return ret;
			}
		}

		case 0x12:
			// 0xA11200: Z80 RESET.
			// NOTE: Not readable in Gens...
			// TODO: Fake Fetch?
			return 0xFFFF;

		case 0x30:
			// 0xA130xx: /TIME registers.
			return ms_RomCartridge->readWord_TIME(address & 0xFF);

		case 0x40: {
			// 0xA14101: TMSS ('SEGA' register)
			if (!tmss_reg.tmss_en) {
				// TMSS is disabled.
				// TODO: Fake Fetch?
				return 0xFFFF;
			}

			// Verify that this is a valid TMSS address.
			if ((address & 0xFE) > 0x03) {
				// Invalid TMSS address.
				// TODO: Fake Fetch?
				return 0xFFFF;
			}

			// 'SEGA' register.
			return tmss_reg.a14000.w[((address & 2) >> 1) ^ U32DATA_U16_INVERT];
		}

		case 0x41: {
			// 0xA14101: TMSS (!CART_CE register)
			if (!tmss_reg.tmss_en) {
				// TMSS is disabled.
				// TODO: Fake Fetch?
				return 0xFFFF;
			}

			// Verify that this is a valid TMSS address.
			if ((address & 0xFE) != 0x00) {
				// Invalid TMSS address.
				// TODO: Fake Fetch?
				return 0xFFFF;
			}

			// !CART_CE register.
			uint16_t ret = (tmss_reg.cart_ce & 1);
			Fake_Fetch ^= 0xFF;	// Fake the next fetched instruction. ("random")
			ret |= ((Fake_Fetch & 0xFF) << 8);
		}

		case 0x00: {
			// 0xA100xx: I/O registers.

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
			uint8_t ret = 0xFF;
			const LibGens::IoManager *const ioManager = EmuMD::m_ioManager;
			switch (address & 0x1E) {
				case 0x00: {
					// 0xA10001: Genesis version register.
					EmuContext *context = EmuContext::Instance();
					if (context)
						ret = context->readVersionRegister_MD();
					break;
				}

				// Parallel I/O
				case 0x02:	ret = ioManager->readDataMD(IoManager::PHYSPORT_1); break;
				case 0x04:	ret = ioManager->readDataMD(IoManager::PHYSPORT_2); break;
				case 0x06:	ret = ioManager->readDataMD(IoManager::PHYSPORT_EXT); break;
				case 0x08:	ret = ioManager->readCtrlMD(IoManager::PHYSPORT_1); break;
				case 0x0A:	ret = ioManager->readCtrlMD(IoManager::PHYSPORT_2); break;
				case 0x0C:	ret = ioManager->readCtrlMD(IoManager::PHYSPORT_EXT); break;

				// Serial I/O
				// TODO: Baud rate handling, etc.
				case 0x0E:	ret = ioManager->readSerTx(IoManager::PHYSPORT_1); break;
				case 0x10:	ret = ioManager->readSerRx(IoManager::PHYSPORT_1); break;
				case 0x12:	ret = ioManager->readSerCtrl(IoManager::PHYSPORT_1); break;
				case 0x14:	ret = ioManager->readSerTx(IoManager::PHYSPORT_2); break;
				case 0x16:	ret = ioManager->readSerRx(IoManager::PHYSPORT_2); break;
				case 0x18:	ret = ioManager->readSerCtrl(IoManager::PHYSPORT_2); break;
				case 0x1A:	ret = ioManager->readSerTx(IoManager::PHYSPORT_EXT); break;
				case 0x1C:	ret = ioManager->readSerRx(IoManager::PHYSPORT_EXT); break;
				case 0x1E:	ret = ioManager->readSerCtrl(IoManager::PHYSPORT_EXT); break;

				default:
					// Unknown register.
					// TODO: Fake Fetch?
					break;
			}

			// NOTE: Word reads to the $A100xx registers result in the
			// register value being duplicated for both MSB and LSB.
			return (ret | (ret << 8));
		}
	}

	// Should not get here...
	// TODO: Fake Fetch?
	return 0xFFFF;
}


/**
 * Read a word from the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @return VDP data byte.
 */
inline uint16_t M68K_Mem::M68K_Read_Word_VDP(uint32_t address)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.

	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0) {
		// Not a valid VDP address.
		return 0x0000;
	}

	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return 0;

	// Check the VDP address.
	switch (address & 0x1E) {
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


/**
 * Read a word from the TMSS ROM. (0x000000 - 0x3FFFFF)
 * TMSS ROM is 2 KB, mirrored throughout the entire range.
 * @param address Address.
 * @return Word from the TMSS ROM.
 */
inline uint16_t M68K_Mem::M68K_Read_Word_TMSS_Rom(uint32_t address)
{
	address &= 0x7FE;
	return MD_TMSS_Rom.u16[address >> 1];
}


/** Write Byte functions. **/


/**
 * Write a byte to RAM. (0xE00000 - 0xFFFFFF)
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
 * Write a byte to the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @param data Byte to write.
 */
inline void M68K_Mem::M68K_Write_Byte_Misc(uint32_t address, uint8_t data)
{
	if (address <= 0xA0FFFF) {
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET)) {
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			return;
		}

		// Call the Z80 Write Byte function.
		// TODO: CPU lockup on accessing 0x7Fxx or >=0x8000.
		Z80_MD_Mem::Z80_WriteB(address & 0xFFFF, data);
		return;
	} else if (address >= 0xA20000) {
		// Invalid address.
		return;
	}

	// Check the second address byte to determine what to do.
	switch ((address >> 8) & 0xFF) {
		default:
			// Invalid address.
			break;

		case 0x11:
			// 0xA11100: Z80 BUSREQ.
			// NOTE: Genesis Plus does BUSREQ at any even 0xA111xx...
			// TODO: Combine with Byte Write version?
			if (address & 1)
				break;

			if (data & 0x01) {
				// M68K requests the bus.
				// Disable the Z80.
				Last_BUS_REQ_Cnt = M68K::ReadOdometer();
				Last_BUS_REQ_St = (Z80_State & Z80_STATE_BUSREQ);

				if (Z80_State & Z80_STATE_BUSREQ) {
					// Z80 is running. Disable it.
					Z80_State &= ~Z80_STATE_BUSREQ;
					
					// TODO: Rework this.
					int ebx = (Cycles_M68K - Last_BUS_REQ_Cnt);
					ebx = Z80_M68K_Cycle_Tab[ebx];
					
					int edx = Cycles_Z80;
					edx -= ebx;
					Z80::Exec(edx);
				}
			} else {
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

			break;

		case 0x12:
			// 0xA11200: Z80 RESET.
			// NOTE: Genesis Plus does RESET at any even 0xA112xx...
			if (address & 1)
				break;

			if (data & 0x01) {
				// RESET is high. Start the Z80.
				Z80_State &= ~Z80_STATE_RESET;
			} else {
				// RESET is low. Stop the Z80.
				Z80::SoftReset();
				Z80_State |= Z80_STATE_RESET;

				// YM2612's RESET line is tied to the Z80's RESET line.
				SoundMgr::ms_Ym2612.reset();
			}
			break;

		case 0x30:
			// 0xA130xx: /TIME registers.
			ms_RomCartridge->writeByte_TIME(address & 0xFF, data);
			break;

		case 0x40: {
			// 0xA14000: TMSS ('SEGA' register)
			if (!tmss_reg.tmss_en) {
				// TMSS is disabled.
				break;
			}

			// Verify that this is a valid TMSS address.
			if ((address & 0xFF) > 0x03)
				break;

			// 'SEGA' register.
			tmss_reg.a14000.b[(address & 3) ^ U32DATA_U8_INVERT] = data;
			break;
		}

		case 0x41: {
			// 0xA14101: TMSS (!CART_CE register)
			if (!tmss_reg.tmss_en) {
				// TMSS is disabled.
				break;
			}

			// Verify that this is a valid TMSS address.
			if ((address & 0xFF) != 0x01)
				break;

			// !CART_CE register.
			tmss_reg.cart_ce = (data & 1);

			// Update TMSS mapping.
			UpdateTmssMapping();
			break;
		}

		case 0x00: {
			// 0xA100xx: I/O registers.

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
			LibGens::IoManager *const ioManager = EmuMD::m_ioManager;
			switch (address & 0x1E) {
				default:
				case 0x00: /// 0xA10001: Genesis version register.
					break;

				// Parallel I/O
				case 0x02:	ioManager->writeDataMD(IoManager::PHYSPORT_1, data); break;
				case 0x04:	ioManager->writeDataMD(IoManager::PHYSPORT_2, data); break;
				case 0x06:	ioManager->writeDataMD(IoManager::PHYSPORT_EXT, data); break;
				case 0x08:	ioManager->writeCtrlMD(IoManager::PHYSPORT_1, data); break;
				case 0x0A:	ioManager->writeCtrlMD(IoManager::PHYSPORT_2, data); break;
				case 0x0C:	ioManager->writeCtrlMD(IoManager::PHYSPORT_EXT, data); break;

				// Serial I/O
				// TODO: Baud rate handling, etc.
				case 0x0E:	ioManager->writeSerTx(IoManager::PHYSPORT_1, data); break;
				case 0x10:	break; // READ-ONLY
				case 0x12:	ioManager->writeSerCtrl(IoManager::PHYSPORT_1, data); break;
				case 0x14:	ioManager->writeSerTx(IoManager::PHYSPORT_2, data); break;
				case 0x16:	break; // READ-ONLY
				case 0x18:	ioManager->writeSerCtrl(IoManager::PHYSPORT_2, data); break;
				case 0x1A:	ioManager->writeSerTx(IoManager::PHYSPORT_EXT, data); break;
				case 0x1C:	break; // READ-ONLY
				case 0x1E:	ioManager->writeSerCtrl(IoManager::PHYSPORT_EXT, data); break;
			}

			break;
		}
	}
}


/**
 * Write a byte to the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @param data Byte to write.
 */
inline void M68K_Mem::M68K_Write_Byte_VDP(uint32_t address, uint8_t data)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.

	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0) {
		// Not a valid VDP address.
		return;
	}

	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return;

	// Check the VDP address.
	switch (address & 0x1F) {
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
 * Write a word to RAM. (0xE00000 - 0xFFFFFF)
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
 * Write a word to the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @param data Word to write.
 */
inline void M68K_Mem::M68K_Write_Word_Misc(uint32_t address, uint16_t data)
{
	if (address <= 0xA0FFFF) {
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
	} else if (address >= 0xA20000) {
		// Invalid address.
		return;
	}

	// Check the second address byte to determine what to do.
	switch ((address >> 8) & 0xFF) {
		default:
			// Invalid address.
			break;

		case 0x11:
			// 0xA11000: Z80 BUSREQ.
			// NOTE: Genesis Plus does BUSREQ at any even 0xA111xx...
			// TODO: Combine with Byte Write version?

			// NOTE: Test data against 0x0100, since 68000 is big-endian.
			if (data & 0x0100) {
				// M68K requests the bus.
				// Disable the Z80.
				Last_BUS_REQ_Cnt = M68K::ReadOdometer();
				Last_BUS_REQ_St = (Z80_State & Z80_STATE_BUSREQ);

				if (Z80_State & Z80_STATE_BUSREQ) {
					// Z80 is running. Disable it.
					Z80_State &= ~Z80_STATE_BUSREQ;

					// TODO: Rework this.
					int ebx = (Cycles_M68K - Last_BUS_REQ_Cnt);
					ebx = Z80_M68K_Cycle_Tab[ebx];

					int edx = Cycles_Z80;
					edx -= ebx;
					Z80::Exec(edx);
				}
			} else {
				// M68K releases the bus.
				// Enable the Z80.
				if (!(Z80_State & Z80_STATE_BUSREQ)) {
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

			break;

		case 0x12:
			// 0xA11200: Z80 RESET.
			// NOTE: Genesis Plus does RESET at any even 0xA112xx...

			// NOTE: Test data against 0x0100, since 68000 is big-endian.
			if (data & 0x0100) {
				// RESET is high. Start the Z80.
				Z80_State &= ~Z80_STATE_RESET;
			} else {
				// RESET is low. Stop the Z80.
				Z80::SoftReset();
				Z80_State |= Z80_STATE_RESET;

				// YM2612's RESET line is tied to the Z80's RESET line.
				SoundMgr::ms_Ym2612.reset();
			}

			break;

		case 0x30:
			// 0xA130xx: /TIME registers.
			ms_RomCartridge->writeWord_TIME(address & 0xFF, data);
			break;

		case 0x40: {
			// 0xA14000: TMSS ('SEGA' register)
			if (!tmss_reg.tmss_en) {
				// TMSS is disabled.
				break;
			}

			// Verify that this is a valid TMSS address.
			if ((address & 0xFF) > 0x03)
				break;

			// 'SEGA' register.
			tmss_reg.a14000.w[((address & 2) >> 1) ^ U32DATA_U16_INVERT] = data;
			break;
		}

		case 0x41: {
			// 0xA14101: TMSS (!CART_CE register)
			if (!tmss_reg.tmss_en) {
				// TMSS is disabled.
				break;
			}

			// Verify that this is a valid TMSS address.
			if ((address & 0xFF) != 0x00)
				break;

			// !CART_CE register.
			tmss_reg.cart_ce = (data & 1);

			// Update TMSS mapping.
			UpdateTmssMapping();
			break;
		}

		case 0x00: {
			// 0xA100xx: I/O registers.

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
			// TODO: Is there special handling for word writes,
			// or is it just "LSB is written"?
			LibGens::IoManager *const ioManager = EmuMD::m_ioManager;
			switch (address & 0x1E) {
				default:
				case 0x00: /// 0xA10001: Genesis version register.
					break;

				// Parallel I/O
				case 0x02:	ioManager->writeDataMD(IoManager::PHYSPORT_1, data); break;
				case 0x04:	ioManager->writeDataMD(IoManager::PHYSPORT_2, data); break;
				case 0x06:	ioManager->writeDataMD(IoManager::PHYSPORT_EXT, data); break;
				case 0x08:	ioManager->writeCtrlMD(IoManager::PHYSPORT_1, data); break;
				case 0x0A:	ioManager->writeCtrlMD(IoManager::PHYSPORT_2, data); break;
				case 0x0C:	ioManager->writeCtrlMD(IoManager::PHYSPORT_EXT, data); break;

				// Serial I/O
				// TODO: Baud rate handling, etc.
				case 0x0E:	ioManager->writeSerTx(IoManager::PHYSPORT_1, data); break;
				case 0x10:	break; // READ-ONLY
				case 0x12:	ioManager->writeSerCtrl(IoManager::PHYSPORT_1, data); break;
				case 0x14:	ioManager->writeSerTx(IoManager::PHYSPORT_2, data); break;
				case 0x16:	break; // READ-ONLY
				case 0x18:	ioManager->writeSerCtrl(IoManager::PHYSPORT_2, data); break;
				case 0x1A:	ioManager->writeSerTx(IoManager::PHYSPORT_EXT, data); break;
				case 0x1C:	break; // READ-ONLY
				case 0x1E:	ioManager->writeSerCtrl(IoManager::PHYSPORT_EXT, data); break;
			}

			break;
		}
	}
}


/**
 * Write a word to the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @param data Word to write.
 */
inline void M68K_Mem::M68K_Write_Word_VDP(uint32_t address, uint16_t data)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.

	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0) {
		// Not a valid VDP address.
		return;
	}

	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return;

	// Check the VDP address.
	switch (address & 0x1E) {
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
 * Update the TMSS mapping.
 */
void M68K_Mem::UpdateTmssMapping(void)
{
	if (!tmss_reg.isTmssMapped()) {
		// TMSS is disabled, or
		// TMSS is enabled and cartridge is mapped.
		ms_M68KBank_Type[0] = M68K_BANK_CARTRIDGE;
		ms_M68KBank_Type[1] = M68K_BANK_CARTRIDGE;
	} else {
		// TMSS is enabled.
		ms_M68KBank_Type[0] = M68K_BANK_TMSS_ROM;
		ms_M68KBank_Type[1] = M68K_BANK_TMSS_ROM;
	}

	// TODO: Better way to update Starscream?
	M68K::UpdateSysBanking();
}

/**
 * Initialize the M68K memory handler.
 * @param system System ID.
 */
void M68K_Mem::InitSys(M68K::SysID system)
{
	// Reset the TMSS registers.
	tmss_reg.reset();

	// Initialize the M68K bank type identifiers.
	switch (system) {
		case M68K::SYSID_MD:
			memcpy(ms_M68KBank_Type, msc_M68KBank_Def_MD, sizeof(ms_M68KBank_Type));
			UpdateTmssMapping();
			break;

		default:
			// Unknown system ID.
			LOG_MSG(68k, LOG_MSG_LEVEL_ERROR,
				"Unknown system ID: %d", system);
			memset(ms_M68KBank_Type, 0x00, sizeof(ms_M68KBank_Type));
			break;
	}
}

/**
 * Update M68K CPU program access structs for bankswitching purposes.
 * @param M68K_Fetch Pointer to first STARSCREAM_PROGRAMREGION to update.
 * @param banks Maximum number of banks to update.
 * @return Number of banks updated.
 */
int M68K_Mem::UpdateSysBanking(STARSCREAM_PROGRAMREGION *M68K_Fetch, int banks)
{
	// Mapping depends on if TMSS is mapped.
	int cur_fetch = 0;
	if (!tmss_reg.isTmssMapped()) {
		// TMSS is not mapped.
		// Update banking using RomCartridgeMD.
		cur_fetch += ms_RomCartridge->updateSysBanking(&M68K_Fetch[cur_fetch], banks);
	} else {
		// TMSS is mapped.
		M68K_Fetch->lowaddr = 0x000000;
		M68K_Fetch->highaddr = (sizeof(MD_TMSS_Rom.u16) - 1);
		M68K_Fetch->offset = (uint32_t)&MD_TMSS_Rom.u16;
		M68K_Fetch++;
		cur_fetch++;
	}

	return cur_fetch;
}


/**
 * Read a byte from the M68K address space.
 * @param address Address.
 * @return Byte from the M68K address space.
 */
uint8_t M68K_Mem::M68K_RB(uint32_t address)
{
	// TODO: This is MD only. Add MCD/32X later.
	address &= 0xFFFFFF;
	const uint8_t bank = ((address >> 21) & 0x7);

	// TODO: Optimize the switch using a bitwise AND.
	switch (ms_M68KBank_Type[bank]) {
		default:
		case M68K_BANK_UNUSED:	return 0xFF;

		// ROM cartridge.
		case M68K_BANK_CARTRIDGE:
			return ms_RomCartridge->readByte(address);

		// Other MD banks.
		case M68K_BANK_IO:		return M68K_Read_Byte_Misc(address);
		case M68K_BANK_VDP:		return M68K_Read_Byte_VDP(address);
		case M68K_BANK_RAM:		return M68K_Read_Byte_Ram(address);
		case M68K_BANK_TMSS_ROM:	return M68K_Read_Byte_TMSS_Rom(address);
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
	const uint8_t bank = ((address >> 21) & 0x7);

	// TODO: Optimize the switch using a bitwise AND.
	switch (ms_M68KBank_Type[bank]) {
		default:
		case M68K_BANK_UNUSED:	return 0xFFFF;
		
		// ROM cartridge.
		case M68K_BANK_CARTRIDGE:
			return ms_RomCartridge->readWord(address);

		// Other MD banks.
		case M68K_BANK_IO:		return M68K_Read_Word_Misc(address);
		case M68K_BANK_VDP:		return M68K_Read_Word_VDP(address);
		case M68K_BANK_RAM:		return M68K_Read_Word_Ram(address);
		case M68K_BANK_TMSS_ROM:	return M68K_Read_Word_TMSS_Rom(address);
	}
	
	// Should not get here...
	return 0xFFFF;
}


/**
 * Write a byte to the M68K address space.
 * @param address Address.
 * @param data Byte to write.
 */
void M68K_Mem::M68K_WB(uint32_t address, uint8_t data)
{
	// TODO: This is MD only. Add MCD/32X later.
	address &= 0xFFFFFF;
	const uint8_t bank = ((address >> 21) & 0x7);

	// TODO: Optimize the switch using a bitwise AND.
	switch (ms_M68KBank_Type[bank]) {
		default:
		case M68K_BANK_UNUSED:
		case M68K_BANK_TMSS_ROM:
			break;

		// ROM cartridge.
		case M68K_BANK_CARTRIDGE:
			ms_RomCartridge->writeByte(address, data);
			break;

		// Other MD banks.
		case M68K_BANK_IO:	M68K_Write_Byte_Misc(address, data); break;
		case M68K_BANK_VDP:	M68K_Write_Byte_VDP(address, data); break;
		case M68K_BANK_RAM:	M68K_Write_Byte_Ram(address, data); break;
	}
}


/**
 * Write a word to the M68K address space.
 * @param address Address.
 * @param data Word to write.
 */
void M68K_Mem::M68K_WW(uint32_t address, uint16_t data)
{
	// TODO: This is MD only. Add MCD/32X later.
	address &= 0xFFFFFF;
	const uint8_t bank = ((address >> 21) & 0x7);

	// TODO: Optimize the switch using a bitwise AND.
	switch (ms_M68KBank_Type[bank]) {
		default:
		case M68K_BANK_UNUSED:
		case M68K_BANK_TMSS_ROM:
			break;

		// ROM cartridge.
		case M68K_BANK_CARTRIDGE:
			ms_RomCartridge->writeWord(address, data);
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
	// TODO: Move to RomCartridgeMD.
	// TODO: Rewrite using SRAM_ctrl.bin and Mapper.bin.
#if 0
	// TODO: Only save if SSF2 bankswitching is active.
	state->SSF2_bank1 = ms_SSF2_BankState[1];
	state->SSF2_bank2 = ms_SSF2_BankState[2];
	state->SSF2_bank3 = ms_SSF2_BankState[3];
	state->SSF2_bank4 = ms_SSF2_BankState[4];
	state->SSF2_bank5 = ms_SSF2_BankState[5];
	state->SSF2_bank6 = ms_SSF2_BankState[6];
	state->SSF2_bank7 = ms_SSF2_BankState[7];
#endif
}


/**
 * ZomgRestoreSSF2BankState(): Restore the SSF2 bankswitching state.
 * @param state Zomg_MD_TimeReg_t struct to restore from.
 */
void M68K_Mem::ZomgRestoreSSF2BankState(const Zomg_MD_TimeReg_t *state)
{
	// TODO: Move to RomCartridgeMD.
	// TODO: Rewrite using SRAM_ctrl.bin and Mapper.bin.
#if 0
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
#endif
}

}
