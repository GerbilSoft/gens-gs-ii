/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpIo.cpp: VDP class: I/O functions.                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include <config.libgens.h>

#include "Vdp.hpp"
#include "Vdp_p.hpp"

// LOG_MSG() subsystem.
#include "macros/log_msg.h"

// Byteswapping macros.
#include "libcompat/byteswap.h"

// M68K CPU.
#include "cpu/star_68k.h"
#include "cpu/M68K_Mem.hpp"
#include "Cartridge/RomCartridgeMD.hpp"

// Emulation Context.
#include "EmuContext/EmuContext.hpp"

// C wrapper functions for Starscream.
#ifdef __cplusplus
extern "C" {
#endif

uint8_t VDP_Int_Ack(void)
{
	// TODO: This won't work with multiple contexts...
	LibGens::EmuContext *instance = LibGens::EmuContext::Instance();
	if (instance != nullptr)
		return instance->m_vdp->Int_Ack();

	// TODO: What should we return here?
	return 0;
}

#ifdef __cplusplus
}
#endif

namespace LibGens {

/**
 * Acknowledge an interrupt.
 * @return ???
 */
uint8_t Vdp::Int_Ack(void)
{
	if ((d->VDP_Reg.m5.Set2 & VDP_REG_M5_SET2_IE0) && (d->VDP_Int & 0x08))
	{
		// VBlank interrupt acknowledge.
		d->VDP_Int &= ~0x08;
		// VINT HAPPENED bit is cleared *here*,
		// not on control port read.
		d->Reg_Status.setBit(VdpStatus::VDP_STATUS_F, false);

		uint8_t rval_mask = d->VDP_Reg.m5.Set1;
		rval_mask &= 0x10;
		rval_mask >>= 2;

		return (d->VDP_Int & rval_mask);
	}

	// Reset the interrupt counter.
	d->VDP_Int = 0;
	return 0;
}

/**
 * Update the IRQ line.
 * @param interrupt Interrupt that just occurred. (If 0, no interrupt occurred.)
 */
void Vdp::updateIRQLine(int interrupt)
{
	// 'interrupt' contains a new interrupt value.
	d->VDP_Int |= interrupt;

	// TODO: HBlank interrupt should take priority over VBlank interrupt.
	if ((d->VDP_Reg.m5.Set2 & VDP_REG_M5_SET2_IE0) && (d->VDP_Int & 0x08)) {
		// VBlank interrupt.
		M68K::Interrupt(6, -1);
		return;
	} else if ((d->VDP_Reg.m5.Set1 & VDP_REG_M5_SET1_IE1) && (d->VDP_Int & 0x04)) {
		// HBlank interrupt.
		M68K::Interrupt(4, -1);
		return;
	}

	// No VDP interrupts.
	// TODO: Move to M68K class.
#ifdef GENS_ENABLE_EMULATION
	main68k_context.interrupts[0] &= 0xF0;
#endif /* GENS_ENABLE_EMULATION */
}

/**
 * Read the H Counter.
 * @return H Counter.
 */
uint8_t Vdp::readHCounter(void)
{
	unsigned int odo_68K = M68K::ReadOdometer();
	odo_68K -= (M68K_Mem::Cycles_M68K - M68K_Mem::CPL_M68K);
	odo_68K &= 0x1FF;

	// H_Counter_Table[][0] == H32.
	// H_Counter_Table[][1] == H40.

	if (d->isH40())
		return d->H_Counter_Table[odo_68K][1];
	else
		return d->H_Counter_Table[odo_68K][0];
}

/**
 * Read the V Counter.
 * @return V Counter.
 */
uint8_t Vdp::readVCounter(void)
{
	unsigned int odo_68K = M68K::ReadOdometer();
	odo_68K -= (M68K_Mem::Cycles_M68K - M68K_Mem::CPL_M68K);
	odo_68K &= 0x1FF;

	unsigned int H_Counter;
	uint8_t bl, bh;		// TODO: Figure out what this actually means.

	if (d->isH40()) {
		// H40
		H_Counter = d->H_Counter_Table[odo_68K][1];
		bl = 0xA4;
	} else {
		// H32
		H_Counter = d->H_Counter_Table[odo_68K][0];
		bl = 0x84;
	}

	bh = ((H_Counter <= 0xE0) ? 1 : 0);
	bl = ((H_Counter >= bl) ? 1 : 0);
	bl &= bh;

	int V_Counter = VDP_Lines.currentLine;
	V_Counter += (bl ? 1 : 0);

	// TODO: Some of these values are wrong.
	// Rewrite HV handling to match Genesis Plus.

	// V_Counter_Overflow depends on PAL/NTSC status.
	if (d->Reg_Status.isPal()) {
		// PAL.
		if (V_Counter >= 0x103) {
			// Overflow.
			V_Counter -= 56;
		}
	} else {
		// NTSC.
		if (V_Counter >= 0xEB) {
			// Overflow.
			V_Counter -= 6;
		}
	}

	// Check for Interlaced Mode 2. (2x resolution)
	// NOTE: enableInterlacedMode does NOT affect this function.
	if (d->isIM2()) {
		// Interlaced mode is enabled.
		uint8_t vc_tmp = (V_Counter & 0xFF);
		vc_tmp = (vc_tmp << 1) | (vc_tmp >> 7);
		return vc_tmp;
	}

	// Interlaced mode is not enabled.
	return (uint8_t)(V_Counter & 0xFF);
}

/**
 * Read the HV Counter.
 * Convenience function for MD.
 * @return HV Counter.
 */
uint16_t Vdp::readHVCounterMD(void)
{
	return ((readVCounter() << 8) | readHCounter());
}

/**
 * Write the test register. ($C0001C)
 * TODO: Not implemented.
 * @param data Test register value.
 */
void Vdp::writeTestRegMD(uint16_t data)
{
	// TODO
	d->testReg = data;
}

/**
 * Write the test register. ($C0001C)
 * Convenience function. This function doubles the bytes
 * into both halves of a word, then calls writeTestRegMD().
 * @param data 8-bit test register value.
 */
void Vdp::writeTestRegMD_8(uint8_t data)
{
	writeTestRegMD(data | (data << 8));
}

/**
 * Read the VDP control port. (M5)
 * This returns the status register.
 * @return VDP status register.
 */
uint16_t Vdp::readCtrlMD(void)
{
	const uint16_t status = d->Reg_Status.read();

	// Reading the control port clears the control word latch.
	d->VDP_Ctrl.ctrl_latch = 0;

	// If the Display is disabled, set the VBlank flag.
	if (d->VDP_Reg.m5.Set2 & VDP_REG_M5_SET2_DISP)
		return status;
	else
		return (status | VdpStatus::VDP_STATUS_VBLANK);
}

/**
 * Read the VDP data port. (M5)
 * This returns the requested data as set by the control word.
 * TODO: Implement the one-word cache.
 * @return Data.
 */
uint16_t Vdp::readDataMD(void)
{
	// TODO: Test this function.
	// Soleil (crusader of Centry) reads from VRam.
	LOG_MSG(vdp_io, LOG_MSG_LEVEL_DEBUG2,
		"VDP_Ctrl.code == %02X, VDP_Ctrl.address == %04X",
		d->VDP_Ctrl.code, d->VDP_Ctrl.address);

	// Reading the data port clears the control word latch.
	d->VDP_Ctrl.ctrl_latch = 0;

	// NOTE: volatile is needed due to an optimization issue caused by
	// -ftree-pre on gcc-4.4.2. (It also breaks on gcc-3.4.5, but that
	// flag doesn't exist on gcc-3.4.5...)
	// An example of the issue can be seen in Soleil (Crusader oF Centry).
	// The onscreen text is partially corrupted when scrolling.
	// TODO: Report this as a bug to the gcc developers.
	volatile uint16_t data;

	// Check the destination.
	switch (d->VDP_Ctrl.code & VdpTypes::CD_DEST_MODE_CD4_MASK) {
		case VdpTypes::CD_DEST_VRAM_READ:
			// VRam Read.
			data = d->VRam.u16[(d->VDP_Ctrl.address & d->VRam_Mask) >> 1];
			break;

		case VdpTypes::CD_DEST_CRAM_READ:
			// CRam Read.
			// FIXME: Missing bits should come from the FIFO.
			data = d->palette.readCRam_16(d->VDP_Ctrl.address & 0x7E);
			break;

		case VdpTypes::CD_DEST_VSRAM_READ:
			// VSRam Read.
			// FIXME: Missing bits should come from the FIFO.
			// FIXME: MD1 and MD2 only have 80 bytes. (Genesis 3 has 128.)
			data = d->VSRam.u16[(d->VDP_Ctrl.address & 0x7E) >> 1];
			break;

		case VdpTypes::CD_DEST_VRAM_8BIT:
			// VRam Read. (8-bit; undocumented)
			// Low byte is from VRAM, with inverted LSB.
			// High byte is the high byte of the next FIFO entry. (TODO)
			data = d->VRam.u8[(d->VDP_Ctrl.address & d->VRam_Mask) ^ 1 ^ U16DATA_U8_INVERT];
			break;

		default:
			// Invalid read specification.
			// NOTE: The address register should be incremented anyway.
			data = 0;
			break;
	}

	d->VDP_Ctrl.address += d->VDP_Reg.m5.Auto_Inc;
	return data;
}


/**
 * Write to the VDP data port. (M5, 8-bit)
 * Convenience function. This function doubles the bytes
 * into both halves of a word, then calls writeDataMD().
 * @param data 8-bit data.
 */
void Vdp::writeDataMD_8(uint8_t data)
{
	/**
	 * NOTE: In Mega Drive mode, the VDP requires 16-bit data.
	 * 8-bit writes will result in the data byte being mirrored
	 * for both high-byte and low-byte.
	 *
	 * The following two instructions are equivalent.
	 * move.b   #$58, ($C00000)
	 * move.w #$5858, ($C00000)
	 */

	writeDataMD(data | (data << 8));
}

/**
 * Read to the VDP data port. (M5)
 * This writes to the target memory as set by the control word.
 * TODO: Implement the FIFO.
 * @param data 16-bit data.
 */
void Vdp::writeDataMD(uint16_t data)
{
	LOG_MSG(vdp_io, LOG_MSG_LEVEL_DEBUG2,
		"VDP_Ctrl.code == %02X, VDP_Ctrl.address == %04X, data == %04X",
		d->VDP_Ctrl.code, d->VDP_Ctrl.address, data);

	// Writing to the data port clears the control word latch.
	d->VDP_Ctrl.ctrl_latch = 0;

	if (d->VDP_Ctrl.code & VdpTypes::CD_MODE_WRITE)
		d->vdpDataWrite_int(data);

	// Check for DMA FILL.
	if ((d->VDP_Ctrl.code & VdpTypes::CD_DMA_ENABLE) &&
	    (d->VDP_Ctrl.DMA_Mode == 0x80))
	{
		// DMA Fill operation is in progress.
		d->DMA_Fill(data);
	}
}

/**
 * Internal VDP data write function.
 * Used by Vdp::writeDataMD() and DMA.
 * @param data Data word.
 */
void VdpPrivate::vdpDataWrite_int(uint16_t data)
{
	// Check the destination.
	uint32_t address = VDP_Ctrl.address;
	switch (VDP_Ctrl.code & VdpTypes::CD_DEST_MASK) {
		case VdpTypes::CD_DEST_VRAM: {
			// VRam Write.
			address &= VRam_Mask;
			uint16_t tmp_data;
			if (address & 0x0001) {
				// Odd address.
				// This results in the data bytes being swapped
				// before writing to (address & ~1).
				tmp_data = (data << 8 | data >> 8);
			} else {
				// Even address.
				// Data is written normally.
				tmp_data = data;
			}

			if (VRam.u16[address>>1] != tmp_data) {
				VRam.u16[address>>1] = tmp_data;
				if ((address & Spr_Tbl_Mask) == Spr_Tbl_Addr) {
					// Sprite Attribute Table.
					SprAttrTbl_m5.w[(address & ~Spr_Tbl_Mask) >> 1] = tmp_data;
				}
				// Mark the address as dirty in the pattern cache.
				cache.mark_dirty(address);
			}
			break;
		}

		case VdpTypes::CD_DEST_CRAM_INT_W:
		case VdpTypes::CD_DEST_CRAM_INT_R:	// TODO: Not needed?
			// CRam Write.
			// TODO: According to the Genesis Software Manual, writing at
			// odd addresses results in "interesting side effects".
			// Those side effects aren't listed, so we're just going to
			// mask the LSB for now.
			// FIXME: "The Adventures of Batman and Robin" has problems if
			// the address wraps around. Ignore addresses over 0x80 for now.
			// Note that this breaks a test in VDPFIFOTesting:
			// #22. DMA Transfer to CRAM Wrapping

			// TODO: Don't do anything if the new value is the
			// same as the old value.

			// Write the word to CRam.
			// CRam is 128 bytes. (64 words)
			if (address < 0x80) {
				palette.writeCRam_16((address & 0x7E), data);
			}
			break;

		case VdpTypes::CD_DEST_VSRAM:
			// VSRam Write.
			// TODO: The Genesis Software Manual doesn't mention what happens
			// with regards to odd address writes for VSRam.

			// Write the word to VSRam.
			// VSRam is 80 bytes. (40 words)
			// TODO: VSRam is 80 bytes, but we're allowing a maximum of 128 bytes here...
			// TODO: Mask off high bits? (Only 10/11 bits are present.)
			VSRam.u16[(address & 0x7E) >> 1] = data;
			break;

		default:
			// Invalid destination.
			break;
	}

	// Increment the address register.
	VDP_Ctrl.address += VDP_Reg.m5.Auto_Inc;
	VDP_Ctrl.address &= VRam_Mask;
}

/**
 * Write to the VDP control port. (M5, 8-bit)
 * Convenience function. This function doubles the bytes
 * into both halves of a word, then calls writeCtrlMD().
 * @param ctrl 8-bit control word.
 */
void Vdp::writeCtrlMD_8(uint8_t ctrl)
{
	/**
	 * NOTE: In Mega Drive mode, the VDP requires 16-bit data.
	 * 8-bit writes will result in the data byte being mirrored
	 * for both high-byte and low-byte.
	 *
	 * The following two instructions are equivalent.
	 * move.b   #$58, ($C00004)
	 * move.w #$5858, ($C00004)
	 */

	writeCtrlMD(ctrl | (ctrl << 8));
}

/**
 * Write to the VDP control port. (M5)
 * @param ctrl Control word.
 */
void Vdp::writeCtrlMD(uint16_t ctrl)
{
	// TODO: Check endianness with regards to the control words. (Wordswapping!)

	// Check if this is the first or second control word.
	if (!d->VDP_Ctrl.ctrl_latch) {
		/**
		 * First control word.
		 *
		 * Format:
		 * [CD1 CD0 A13 A12 A11 A10 A09 A08] (D15-D8)
		 * [A07 A06 A05 A04 A03 A02 A01 A00] (D7-D0)
		 *
		 * CD = access code
		 *  A = address
		 *
		 * NOTE: CD5-CD2 and A15-A14 are left intact after the
		 * first control word is processed. They are replaced
		 * when the second control word is processed.
		 */

		// Update the VDP address counter.
		// NOTE: High bits are reloaded from the latch.
		// Reference: http://gendev.spritesmind.net/forum/viewtopic.php?t=1277&p=17430#17430
		d->VDP_Ctrl.address = (ctrl & 0x3FFF);
		d->VDP_Ctrl.address |= d->VDP_Ctrl.addr_hi_latch;

		// Update the VDP access code register.
		d->VDP_Ctrl.code &= ~0x03;
		d->VDP_Ctrl.code |= ((ctrl >> 14) & 0x03);

		// Check if this is a register write
		if ((ctrl & 0xC000) == 0x8000) {
			/**
			 * Register write.
			 *
			 * Format:
			 * [  1   0   x R04 R03 R02 R01 R00] (D15-D8)
			 * [D07 D06 D05 D04 D03 D02 D01 D00] (D7-D0)
			 *
			 * R = register number
			 * D = data
			 */
			const int reg = (ctrl >> 8) & 0x1F;
			d->setReg(reg, (ctrl & 0xFF));
		} else {
			// First control word.
			d->VDP_Ctrl.ctrl_latch = 1;
		}

		// We're done here.
		return;
	}

	/**
	 * Second control word.
	 *
	 * Format:
	 * [  x   x   x   x   x   x   x   x] (D15-D8)
	 * [CD5 CD4 CD3 CD2   x A16 A15 A14] (D7-D0)
	 *
	 * CD = access code
	 *  A = address
	 *
	 * NOTE: CD1-CD0 and A13-A01 are left intact after the
	 * first control word is processed. They are replaced
	 * when the second control word is processed.
	 *
	 * NOTE 2: CD5 is only updated if M1 (DMA) == 1.
	 * (VDP_Reg.m5.Set2 & VDP_REG_M5_SET2_M1)
	 *
	 * NOTE 3: A16 is only used if 128 KB mode is enabled.
	 */
	d->VDP_Ctrl.ctrl_latch = 0;	// Clear the control word latch.

	// Update the VDP address high bits latch.
	// TODO: Cache the mask here?
	const uint16_t ctrl_mask = (d->is128KB() ? 0x0007 : 0x0003);
	d->VDP_Ctrl.addr_hi_latch = ((uint32_t)(ctrl & ctrl_mask) << 14);
	// Update the VDP address counter.
	d->VDP_Ctrl.address &= ~0x1C000;
	d->VDP_Ctrl.address |= d->VDP_Ctrl.addr_hi_latch;

	// Update the VDP access code register: CD(4..2)
	d->VDP_Ctrl.code &= ~0x1C;
	d->VDP_Ctrl.code |= ((ctrl >> 2) & 0x1C);
	if (d->VDP_Reg.m5.Set2 & VDP_REG_M5_SET2_M1) {
		// DMA is enabled. Update CD5.
		d->VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;
		d->VDP_Ctrl.code |= ((ctrl >> 2) & VdpTypes::CD_DMA_ENABLE);
	}

	// If CD5 is not set, DMA is disabled,
	// so we're done here.
	if (!(d->VDP_Ctrl.code & VdpTypes::CD_DMA_ENABLE))
		return;

	// Process the DMA control write.
	d->processDmaCtrlWrite();
}

}
