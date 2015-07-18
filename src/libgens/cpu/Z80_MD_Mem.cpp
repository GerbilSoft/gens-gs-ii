/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Z80_MD_Mem.cpp: Z80 memory handler. (Mega Drive mode)                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2015 by David Korth                                  *
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

#include "Z80_MD_Mem.hpp"

// Unused parameter macro.
// TODO: Move somewhere else?
#define UNUSED(x) ((void)x)

// LibGens includes.
#include "M68K_Mem.hpp"
#include "Vdp/Vdp.hpp"

// Sound Manager.
#include "sound/SoundMgr.hpp"

// EmuContext
#include "EmuContext/EmuContext.hpp"

#if defined(__APPLE__) && defined(__i386__) || defined(__amd64__)
// Mac OS X requires 16-byte aligned stacks.
// Otherwise, the program will randomly crash in
// __dyld_misaligned_stack_error().
// (The crash might not even be immediately after
// calling the C function!)
#define FORCE_STACK_ALIGNMENT
#endif
#define FORCE_STACK_ALIGNMENT

// TODO: mdZ80 accesses Ram_Z80 directly.
// Move Ram_Z80 back to Z80_MD_Mem once mdZ80 is updated.
uint8_t Ram_Z80[8 * 1024];

namespace LibGens
{

// Static class variables.
int Z80_MD_Mem::Bank_Z80;

void Z80_MD_Mem::Init(void)
{
	// TODO
}

void Z80_MD_Mem::End(void)
{
	// TODO
}

/** Z80 Read Byte functions. **/

/**
 * Read a byte from the YM2612.
 * @param address Address to read from.
 * @return YM2612 register.
 */
inline uint8_t Z80_MD_Mem::Z80_ReadB_YM2612(uint32_t address)
{
	// According to the Genesis Software Manual, all four addresses return
	// the same value for YM2612_Read().
	UNUSED(address);
	
	// The YM2612's RESET line is tied to the Z80's RESET line.
	// TODO: Determine the correct return value.
	if (M68K_Mem::Z80_State & Z80_STATE_RESET)
		return 0xFF;
	
	// Return the YM2612 status register.
	return SoundMgr::ms_Ym2612.read();
}

/**
 * Read a byte from the VDP.
 * @param address Address to read from.
 * @return VDP register.
 */
inline uint8_t Z80_MD_Mem::Z80_ReadB_VDP(uint32_t address)
{
	if (address < 0x7F00) {
		// Not in VDP range.
		// Ignore this read.
		return 0;
	}

	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return 0;

	Vdp *vdp = context->m_vdp;
	uint8_t ret = 0; // TODO: Default to 0xFF?
	switch (address & 0xFD) {
		case 0x00:
			// VDP data port. (high byte)
			// NOTE: Gens doesn't read the data port here,
			// but it should still be readable...
			ret = ((vdp->readDataMD() >> 8) & 0xFF);
			break;

		case 0x01:
			// VDP data port. (low byte)
			// NOTE: Gens doesn't read the data port here,
			// but it should still be readable...
			ret = (vdp->readDataMD() & 0xFF);
			break;

		case 0x04:
			// VDP control port. (high byte)
			ret = ((vdp->readCtrlMD() >> 8) & 0xFF);
			break;

		case 0x05:
			// VDP control port. (low byte)
			// FIXME: Unused bits return prefetch data.
			ret = (vdp->readCtrlMD() & 0xFF);
			break;

		case 0x08: case 0x0C:
			// V counter.
			return vdp->readVCounter();

		case 0x09: case 0x0D:
			// H counter.
			return vdp->readHCounter();

		case 0x18: case 0x19:
		case 0x1C: case 0x1D:
			// Unused read address.
			// This address is valid, so a lockup
			// should not occur.
			break;

		default:
			// Invalid VDP port.
			// (PSG is not readable.)
			// TODO: Z80 should lock up.
			break;
	}

	return ret;
}

/**
 * Read a byte from MC68000 ROM.
 * @param address Address to read from.
 * @return Byte from MC68000 ROM.
 */
inline uint8_t Z80_MD_Mem::Z80_ReadB_68K_Rom(uint32_t address)
{
	// Z80 cannot read from M68K RAM.
	// If this is attempted, 0xFF will be returned.
	// Reference: http://gendev.spritesmind.net/forum/viewtopic.php?t=985
	if (Bank_Z80 >= 0xE00000)
		return 0xFF;
	
	address &= 0x7FFF;
	address |= Bank_Z80;
	return M68K_Mem::M68K_RB(address);
}

/** Z80 Write Byte functions. **/

/**
 * Shift a bit into the Z80's 68K ROM banking register.
 * @param address Address to write to.
 * @param data Byte to write.
 */
inline void Z80_MD_Mem::Z80_WriteB_Bank(uint32_t address, uint8_t data)
{
	if (address > 0x60FF) {
		// TODO: Invalid address. This should do something.
		return;
	}

	uint32_t bank_address = ((Bank_Z80 & 0xFF0000) >> 1);
	bank_address |= ((data & 1) << 23);
	Bank_Z80 = bank_address;
}

/**
 * Write a byte to the YM2612.
 * @param address Address to write to.
 * @param data Byte to write.
 */
inline void Z80_MD_Mem::Z80_WriteB_YM2612(uint32_t address, uint8_t data)
{
	// The YM2612's RESET line is tied to the Z80's RESET line.
	if (M68K_Mem::Z80_State & Z80_STATE_RESET)
		return;
	
	// Write to the YM2612.
	SoundMgr::ms_Ym2612.write(address & 0x03, data);
}

/**
 * Write a byte to the VDP.
 * @param address Address to write to.
 * @param data Byte to write.
 */
inline void Z80_MD_Mem::Z80_WriteB_VDP(uint32_t address, uint8_t data)
{
	if (address < 0x7F00) {
		// Not in VDP range.
		// Ignore this read.
		return;
	}

	// TODO: Don't use EmuContext here...
	EmuContext *context = EmuContext::Instance();
	if (!context)
		return;

	Vdp *vdp = context->m_vdp;
	switch (address & 0xFC) {
		case 0x00:
			// VDP data port.
			vdp->writeDataMD_8(data);
			break;
		case 0x04:
			// VDP control port.
			vdp->writeCtrlMD_8(data);
			break;
		case 0x10: case 0x14:
			// PSG control port. (Odd addresses only)
			if (address & 1) {
				SoundMgr::ms_Psg.write(data);
			}
			break;
		case 0x18:
			// Unused write address.
			// This address is valid, so a lockup
			// should not occur.
			break;
		case 0x1C:
			// VDP test register.
			vdp->writeTestRegMD_8(data);
			break;
		default:
			// Invalid VDP port.
			// TODO: Z80 should lock up.
			break;
	}
}

/**
 * Write a byte to MC68000 ROM.
 * @param address Address to write to.
 * @param data Byte to write.
 */
inline void Z80_MD_Mem::Z80_WriteB_68K_Rom(uint32_t address, uint8_t data)
{
	// NOTE: Z80 writes to M68K RAM are allowed.
	// Reference: http://gendev.spritesmind.net/forum/viewtopic.php?t=985
	
	address &= 0x7FFF;
	address |= Bank_Z80;
	M68K_Mem::M68K_WB(address, data);
}

/** Z80 General Read/Write functions. **/

/**
 * Read a byte from the Z80 address space.
 * @param address Address to read from.
 * @return Byte from the Z80 address space.
 */
uint8_t Z80_MD_Mem::Z80_ReadB(uint32_t address)
{
	const uint8_t page = ((address >> 12) & 0x0F);
	switch (page & 0x0F) {
		case 0x00: case 0x01:
		case 0x02: case 0x03:
			// 0x0000-0x1FFF: Z80 RAM.
			// 0x2000-0x3FFF: Z80 RAM. (mirror)
			return Ram_Z80[address & 0x1FFF];

		case 0x04: case 0x05:
			// 0x4000-0x5FFF: YM2612.
			return Z80_ReadB_YM2612(address);

		case 0x06:
			// 0x6000-0x6FFF: Bank.
			// NOTE: Reading from the bank register is undefined...
			return 0xFF;

		case 0x07:
			// 0x7000-0x7FFF: VDP.
			return Z80_ReadB_VDP(address);

		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E: case 0x0F:
			// 0x8000-0xFFFF: 68K ROM bank.
			return Z80_ReadB_68K_Rom(address);
	}

	// Should not get here...
	return 0xFF;
}

/**
 * Write a byte to the Z80 address space.
 * @param address Address to write to.
 * @param data Byte to write to the Z80 address space.
 */
void Z80_MD_Mem::Z80_WriteB(uint32_t address, uint8_t data)
{
	const uint8_t page = ((address >> 12) & 0x0F);
	switch (page & 0x0F) {
		case 0x00: case 0x01:
		case 0x02: case 0x03:
			// 0x0000-0x1FFF: Z80 RAM.
			// 0x2000-0x3FFF: Z80 RAM. (mirror)
			Ram_Z80[address & 0x1FFF] = data;
			break;

		case 0x04: case 0x05:
			// 0x4000-0x5FFF: YM2612.
			Z80_WriteB_YM2612(address, data);
			break;

		case 0x06:
			// 0x6000-0x6FFF: Bank.
			Z80_WriteB_Bank(address, data);
			break;

		case 0x07:
			// 0x7000-0x7FFF: VDP.
			Z80_WriteB_VDP(address, data);
			break;

		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E: case 0x0F:
			// 0x8000-0xFFFF: 68K ROM bank.
			Z80_WriteB_68K_Rom(address, data);
			break;
	}
}

}
