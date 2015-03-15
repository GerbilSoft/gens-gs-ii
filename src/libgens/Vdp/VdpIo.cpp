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

#include "Vdp.hpp"

// LOG_MSG() subsystem.
#include "macros/log_msg.h"

// Byteswapping macros.
#include "Util/byteswap.h"

// M68K CPU.
#include "cpu/star_68k.h"
#include "cpu/M68K_Mem.hpp"
#include "Cartridge/RomCartridgeMD.hpp"

// Emulation Context.
#include "../EmuContext.hpp"

// Vdp private class.
#include "Vdp_p.hpp"

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
	if ((d->VDP_Reg.m5.Set2 & 0x20) && (d->VDP_Int & 0x08))
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
	if ((d->VDP_Reg.m5.Set2 & 0x20) && (d->VDP_Int & 0x08)) {
		// VBlank interrupt.
		M68K::Interrupt(6, -1);
		return;
	} else if ((d->VDP_Reg.m5.Set1 & 0x10) && (d->VDP_Int & 0x04)) {
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
		H_Counter = d->H_Counter_Table[odo_68K][0];
		bl = 0xA4;
	} else {
		// H32
		H_Counter = d->H_Counter_Table[odo_68K][1];
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
	if (d->Interlaced == VdpTypes::INTERLACED_MODE_2) {
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
 * Read the test register. ($C0001C)
 * TODO: Not implemented.
 * @return Test register value.
 */
uint16_t Vdp::readTestRegMD(void) const
{
	return d->testReg;
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
	if (d->VDP_Reg.m5.Set2 & 0x40)
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
			data = d->VRam.u16[(d->VDP_Ctrl.address & 0xFFFF) >> 1];
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
			data = d->VRam.u8[(d->VDP_Ctrl.address & 0xFFFF) ^ 1 ^ U16DATA_U8_INVERT];
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
 * Update the DMA state.
 * @return Number of cycles taken from the 68000 for DMA.
 */
unsigned int Vdp::updateDMA(void)
{
	/**
	 * DMA transfer rate depends on the following:
	 * - Horizontal resolution. (H32/H40)
	 * - DMA type.
	 * - Are we in VBlank or active drawing?
	 *
	 * DMA_Timing_Table offset:
	 * [DMAT1 DMAT0][HRES VBLANK]
	 * - DMAT1/DMAT0 == DMA type.
	 * - HRES == horizontal resolution.
	 * - VBLANK == vertical blanking.
	 */

	// Horizontal resolution.
	// TODO: Optimize this conditional? (Not sure if the compiler optimizes it...)
	unsigned int offset = (d->isH40() ? 2 : 0);

	// Check if we're in VBlank or if the VDP is disabled.
	if (VDP_Lines.currentLine >= VDP_Lines.totalVisibleLines ||
	    (!(d->VDP_Reg.m5.Set2 & 0x40)))
	{
		// In VBlank, or VDP is disabled.
		offset |= 1;
	}

	// Cycles elapsed is based on M68K cycles per line.
	unsigned int cycles = M68K_Mem::CPL_M68K;

	// DMA timing table.
	static const uint8_t DMA_Timing_Table[4][4] = {
		/* Format: H32 active, H32 blanking, H40 active, H40 blanking */
		{8,    83,   9, 102},	/* 68K to VRam (1 word == 2 bytes) */
		{16,  167,  18, 205},	/* 68K to CRam or VSRam */
		{15,  166,  17, 204},	/* VRam Fill */
		{8,    83,   9, 102},	/* VRam Copy (1 word == 2 bytes) */
	};

	// Get the DMA transfer rate.
	const uint8_t timing = DMA_Timing_Table[(int)d->DMAT_Type & 3][offset];
	if (DMAT_Length > timing) {
		// DMA is not finished.
		DMAT_Length -= timing;
		if ((int)d->DMAT_Type & 2) {
			// Internal DMA. (FILL, COPY)
			// M68K doesn't have to wait.
			return 0;
		}

		// External DMA. (ROM/RAM to VRAM)
		// Return the total number of cycles the M68K should wait.
		return cycles;
	}

	// DMA is finished. Do some processing.
	const unsigned int len_tmp = DMAT_Length;
	DMAT_Length = 0;

	// Calculate the DMA cycles used on this scanline.
	// (NOTE: I have no idea how this formula was created.)
	//cycles = (((cycles << 16) / timing) * len_tmp) >> 16;
	cycles <<= 16;
	cycles /= timing;
	cycles *= len_tmp;
	cycles >>= 16;

	// Clear the DMA Busy flag.
	d->Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, false);

	if ((int)d->DMAT_Type & 2) {
		// Internal DMA. (FILL, COPY)
		// M68K doesn't have to wait.
		return 0;
	}

	// External DMA. (ROM/RAM to VRAM)
	// Return the total number of cycles the M68K should wait.
	return cycles;
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
 * Perform a DMA Fill operation. (Called from Vdp::writeDataMD().)
 * @param data 16-bit data.
 */
void VdpPrivate::DMA_Fill(uint16_t data)
{
	// Set the VRam flag.
	markVRamDirty();

	// Get the values. (length is in bytes)
	// NOTE: When writing to VRAM, DMA FILL uses bytes, not words.
	// When writing to CRAM or VSRAM, DMA FILL uses words.
	uint32_t address = VDP_Ctrl.address;
	// NOTE: length == 0 is handled as 65536, since the value
	// isn't checked until after the first iteration.
	uint16_t length = DMA_Length();

	// Set the DMA Busy flag.
	// TODO: Needs to run on a line-by-line basis.
	Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, true);

	// TODO: Although we decrement DMAT_Length correctly based on
	// DMA cycles per line, we fill everything immediately instead
	// of filling at the correct rate.
	// Perhaps this should be combined with DMA_LOOP.
	VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;	// Clear CD5.

	// Set DMA type and length.
	DMAT_Type = DMAT_FILL;
	q->DMAT_Length = (length > 0 ? length : 65536);
	set_DMA_Length(0);

	// TODO: Do DMA FILL line-by-line instead of all at once.
	const uint8_t fill_hi = (data >> 8) & 0xFF;
	switch (VDP_Ctrl.code & VdpTypes::CD_DEST_MODE_MASK) {
		case VdpTypes::CD_DEST_VRAM_WRITE:
			// Write to VRAM.
			do {
				// NOTE: DMA FILL writes to the adjacent byte.
				VRam.u8[address ^ 1 ^ U16DATA_U8_INVERT] = fill_hi;
				address += VDP_Reg.m5.Auto_Inc;
				address &= 0xFFFF;	// TODO: 128 KB support.
			} while (--length != 0);
			break;

		case VdpTypes::CD_DEST_CRAM_WRITE:
			// Write to CRAM.
			// TODO: FIFO emulation.
			do {
				palette.writeCRam_16((address & 0x7E), data);
				address += VDP_Reg.m5.Auto_Inc;
				address &= 0xFFFF;	// TODO: 128 KB support.
			} while (--length != 0);
			break;

		case VdpTypes::CD_DEST_VSRAM_WRITE:
			// Write to VSRAM.
			// TODO: FIFO emulation.
			do {
				VSRam.u16[(address & 0x7E) >> 1] = data;
				address += VDP_Reg.m5.Auto_Inc;
				address &= 0xFFFF;	// TODO: 128 KB support.
			} while (--length != 0);
			break;

		default:
			// Unsupported...
			DMAT_Type = DMAT_MEM_TO_VRAM;
			q->DMAT_Length = 0;
			Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, false);
			break;
	}

	// Save the new address.
	VDP_Ctrl.address = (address & 0xFFFF);	// TODO: 128 KB support.

	// NOTE: DMA FILL updates the DMA source address,
	// even though it isn't used.
	inc_DMA_Src_Adr(q->DMAT_Length);
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
		case VdpTypes::CD_DEST_VRAM:
			// VRam Write.
			markVRamDirty();
			address &= 0xFFFF;	// VRam is 64 KB. (32 Kwords)
			if (address & 0x0001) {
				// Odd address.
				// This results in the data bytes being swapped
				// before writing to (address & ~1).
				VRam.u16[address>>1] = (data << 8 | data >> 8);
			} else {
				// Even address.
				// Data is written normally.
				VRam.u16[address>>1] = data;
			}
			break;

		case VdpTypes::CD_DEST_CRAM_INT_W:
		case VdpTypes::CD_DEST_CRAM_INT_R:	// TODO: Not needed?
			// CRam Write.
			// TODO: According to the Genesis Software Manual, writing at
			// odd addresses results in "interesting side effects".
			// Those side effects aren't listed, so we're just going to
			// mask the LSB for now.

			// Write the word to CRam.
			// CRam is 128 bytes. (64 words)
			palette.writeCRam_16((address & 0x7E), data);
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
	VDP_Ctrl.address &= 0xFFFF;	// TODO: 128 KB support.
}

/**
 * Mem-to-DMA loop.
 * @param src_component Source component.
 * @param dest_component Destination component.
 */
template<VdpPrivate::DMA_Src_t src_component, VdpPrivate::DMA_Dest_t dest_component>
inline void VdpPrivate::T_DMA_Loop(void)
{
	// Get the DMA source address.
	// NOTE: DMA_Src_Adr is the source address / 2.
	uint32_t src_address = DMA_Src_Adr() * 2;

	// NOTE: DON'T get DMA length from the registers.
	// It's been reset to 0 already.
	// Just use DMAT_Length.
	int length = q->DMAT_Length;

	LOG_MSG(vdp_io, LOG_MSG_LEVEL_DEBUG2,
		"<%d, %d> src_address == 0x%06X, dest_address == 0x%04X, length == %d",
		src_component, dest_component, src_address, VDP_Ctrl.address, length);

	// Mask the source address, depending on type.
	switch (src_component) {
		case DMA_SRC_ROM:
			src_address &= 0x3FFFFE;
			break;

		case DMA_SRC_M68K_RAM:
			src_address &= 0xFFFE;
			break;

		// TODO: Port to LibGens.
#if 0
		case DMA_SRC_PRG_RAM:
			src_address = ((src_address & 0x1FFFE) + Bank_M68K);
			break;

		case DMA_SRC_WORD_RAM_2M:
			/**
			 * DMA access from Word RAM has a bug:
			 * - The first word is transferred incorrectly.
			 * - The second word written to VRAM is actually the first word requested.
			 * - The last word requested is not written.
			 * The -2 source address offset simulates this.
			 *
			 * Reference:
			 * - http://e02stealth.tumblr.com/post/13505293669/sonic-megamix-is-not-sonic-cd
			 *
			 * TODO: Verify 128 KB wrap-around works here.
			 * TODO: Use a garbage value instead?
			 * genplus-gx increments src_address and dest_address.
			 */
			src_address -= 2;
			src_address &= 0x3FFFE;
			break;

		case DMA_SRC_WORD_RAM_1M_0:
		case DMA_SRC_WORD_RAM_1M_1:
		case DMA_SRC_WORD_RAM_CELL_1M_0:
		case DMA_SRC_WORD_RAM_CELL_1M_1:
			src_address -= 2;
			src_address &= 0x1FFFE;
			break;
#endif

		default:	// to make gcc shut up
			break;
	}

	// Determine if any flags should be set.
	switch (dest_component) {
		case DMA_DEST_VRAM:
			markVRamDirty();
			DMAT_Type = DMAT_MEM_TO_VRAM;
			break;

		case DMA_DEST_CRAM:
		case DMA_DEST_VSRAM:
			DMAT_Type = DMAT_MEM_TO_CRAM_VSRAM;
			break;

		default:	// to make gcc shut up
			break;
	}

	// Divide source address by 2 to get word addresses.
	// This will help with 128 KB wrapping.
	uint16_t src_word_address = (src_address >> 1);

	// src_base_address is used to ensure 128 KB wrapping.
	unsigned int src_base_address = ((src_address & 0xFE0000) >> 1);

	// TODO: Do DMA MEM-to-VRAM line-by-line instead of all at once.
	do {
		// Get the word.
		uint16_t w;
		switch (src_component) {
			case DMA_SRC_ROM: {
				// TODO: Banking is done in 512 KB segments.
				// Optimize this by getting a pointer to the segment?
				const uint32_t req_addr = ((src_word_address | src_base_address) << 1);
				w = M68K_Mem::ms_RomCartridge->readWord(req_addr);
				break;
			}

			case DMA_SRC_M68K_RAM:
				//w = M68K_Mem::Ram_68k.u16[src_word_address];
				w = Ram_68k.u16[src_word_address];
				break;

			// TODO: Port to LibGens.
#if 0
			case DMA_SRC_PRG_RAM:
				// TODO: This is untested!
				w = Ram_Prg.u16[src_word_address];
				break;

			case DMA_SRC_WORD_RAM_2M:
				w = Ram_Word_2M.u16[src_word_address];
				break;

			case DMA_SRC_WORD_RAM_1M_0:
				// TODO: This is untested!
				w = Ram_Word_1M.u16[src_word_address];
				break;

			case DMA_SRC_WORD_RAM_1M_1:
				// TODO: This is untested!
				w = Ram_Word_1M.u16[src_word_address + (0x20000 >> 1)];
				break;

			case DMA_SRC_WORD_RAM_CELL_1M_0:
				// TODO: This is untested!
				// Cell conversion is required.
				w = Cell_Conv_Tab[src_word_address];
				w = Ram_Word_1M.u16[w];
				break;

			case DMA_SRC_WORD_RAM_CELL_1M_1:
				// TODO: This is untested!
				// Cell conversion is required.
				w = Cell_Conv_Tab[src_word_address];
				w = Ram_Word_1M.u16[w + (0x20000 >> 1)];
				break;
#endif

			default:	// to make gcc shut up
				w = 0;	// NOTE: Remove this once everything is ported to LibGens.
				break;
		}

		// Increment the source word address.
		src_word_address++;

		// Write the word.
		// TODO: Might not work if Auto_Inc is odd...
		vdpDataWrite_int(w);
	} while (--length != 0);

	// DMA is done.
	VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;

	// If any bytes weren't copied, subtract it from the saved length.
	q->DMAT_Length -= length;
	if (q->DMAT_Length <= 0) {
		// No DMA left!
		Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, false);
		return;
	}

	// Save the new source address.
	// NOTE: The new DMA_Address is the wrapped version.
	// The old asm code saved the unwrapped version.
	// Ergo, it simply added length to DMA_Address.
	inc_DMA_Src_Adr(q->DMAT_Length);

	// Update DMA.
	q->updateDMA();

	// NOTE: main68k_releaseCycles() takes no parameters,
	// but the actual function subtracts eax from __io_cycle_counter.
	// eax was equal to DMAT_Length.
	M68K::ReleaseCycles(q->DMAT_Length);
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
		d->VDP_Ctrl.address &= ~0x3FFF;
		d->VDP_Ctrl.address |= (ctrl & 0x3FFF);

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
	 * [CD5 CD4 CD3 CD2   x   x A15 A14] (D7-D0)
	 *
	 * CD = access code
	 *  A = address
	 *
	 * NOTE: CD1-CD0 and A13-A01 are left intact after the
	 * first control word is processed. They are replaced
	 * when the second control word is processed.
	 *
	 * NOTE 2: CD5 is only updated if DMA Enabled == 1.
	 * (VDP_Reg.m5.Set2 & 0x04)
	 */
	d->VDP_Ctrl.ctrl_latch = 0;	// Clear the control word latch.

	// Update the VDP address counter.
	d->VDP_Ctrl.address &= ~0xC000;
	d->VDP_Ctrl.address |= ((ctrl & 0x0003) << 14);

	// Update the VDP access code register: CD(4..2)
	d->VDP_Ctrl.code &= ~0x1C;
	d->VDP_Ctrl.code |= ((ctrl >> 2) & 0x1C);
	if (d->VDP_Reg.m5.Set2 & 0x04) {
		// DMA is enabled. Update CD5.
		d->VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;
		d->VDP_Ctrl.code |= ((ctrl >> 2) & VdpTypes::CD_DMA_ENABLE);
	}

	// If CD5 is not set, DMA is disabled,
	// so we're done here.
	if (!(d->VDP_Ctrl.code & VdpTypes::CD_DMA_ENABLE))
		return;

	// Check for DMA FILL.
	if (d->VDP_Ctrl.DMA_Mode == 0x80) {
		// DMA FILL.
		// Operation is processed on DATA WRITE.
		return;
	}

	// Determine the DMA destination.
	// NOTE: Ignoring CD0.
	VdpPrivate::DMA_Dest_t dest_component;
	switch (d->VDP_Ctrl.code & VdpTypes::CD_DEST_MASK) {
		case VdpTypes::CD_DEST_VRAM:
			dest_component = VdpPrivate::DMA_DEST_VRAM;
			break;
		case VdpTypes::CD_DEST_CRAM_INT_W:
		case VdpTypes::CD_DEST_CRAM_INT_R:	// TODO: Is this needed?
			dest_component = VdpPrivate::DMA_DEST_CRAM;
			break;
		case VdpTypes::CD_DEST_VSRAM:
			dest_component = VdpPrivate::DMA_DEST_VSRAM;
			break;
		default:
			// Invalid destination component.
			return;
	}

	// Get the DMA addresses.
	uint32_t src_address = d->DMA_Src_Adr();	// Src Address / 2
	uint16_t dest_address = d->VDP_Ctrl.address;	// Dest Address (TODO: uint32_t for 128 KB)

	int length = d->DMA_Length();
	if (length == 0) {
		// DMA length is zero.
		if (options.zeroLengthDMA) {
			// Zero-Length DMA transfers are enabled.
			// Ignore this request.
			d->VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;
			return;
		}

		// Zero-Length DMA trnasfers are disabled.
		// The MD VDP decrements the DMA length counter before checking if it has
		// reached zero. So, doing a zero-length DMA request will actually do a
		// DMA request for 65,536 words.
		length = 0x10000;
	}

	// Check for DMA COPY.
	if (d->VDP_Ctrl.DMA_Mode == 0xC0) {
		// DMA COPY.
		// TODO: Verify that CD4 is set; if it isn't, VDP should lock up.
		src_address &= 0xFFFF;
		d->Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, true);	// Set the DMA BUSY bit.
		DMAT_Length = length;
		d->DMAT_Type = VdpPrivate::DMAT_COPY;
		d->set_DMA_Length(0);
		d->markVRamDirty();

		// TODO: Is this correct with regards to endianness?
		// TODO: Do DMA COPY line-by-line instead of all at once.
		do {
			d->VRam.u8[dest_address] = d->VRam.u8[src_address];

			// Increment the addresses.
			src_address = ((src_address + 1) & 0xFFFF);
			dest_address = ((dest_address + d->VDP_Reg.m5.Auto_Inc) & 0xFFFF);
		} while (--length != 0);

		// Save the new addresses.
		// NOTE: DMA COPY uses bytes, not words.
		d->inc_DMA_Src_Adr(DMAT_Length);	// TODO: Should DMA_Src_Adr_H's DMA flags be cleared?
		d->VDP_Ctrl.address = dest_address;
		return;
	}

	if (d->VDP_Ctrl.DMA_Mode & 0x80) {
		// TODO: What does this mean?
		d->VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;
		return;
	}

	// Multiply the source address by two to get the real source address.
	src_address *= 2;

	// Determine the source component.
	VdpPrivate::DMA_Src_t src_component = VdpPrivate::DMA_SRC_ROM;	// TODO: Determine a better default.
	int WRam_Mode;

	// Maximum ROM source address: 4 MB for Sega CD or 32X, 10 MB for standard MD.
	const uint32_t maxRomSrcAddress =
		((SysStatus.SegaCD || SysStatus._32X)
		 ? 0x3FFFFF
		 : 0x9FFFFF);

	if (src_address <= maxRomSrcAddress) {
		// Main ROM.
		src_component = VdpPrivate::DMA_SRC_ROM;
	} else if (!SysStatus.SegaCD) {
		// SegaCD is not started. Assume M68K RAM.
		// TODO: This includes invalid addresses!
		src_component = VdpPrivate::DMA_SRC_M68K_RAM;
	} else {
		// SegaCD is started.
		if (src_address >= 0x240000) {
			// Assume M68K RAM.
			// TODO: This includes invalid addresses!
			src_component = VdpPrivate::DMA_SRC_M68K_RAM;
		} else if (src_address < 0x40000) {
			// Program RAM.
			src_component = VdpPrivate::DMA_SRC_PRG_RAM;
		} else {
			// Word RAM. Check the Word RAM state to determine the mode.
			// TODO: Determine how this works.
			// TODO: Port to LibGens.
#if 0
			WRam_Mode = (Ram_Word_State & 0x03) + 3;
			if (WRam_Mode < 5 || src_address < 0x220000) {
				src_component = (VdpPrivate::DMA_Src_t)WRam_Mode;
			} else {
				src_component = (VdpPrivate::DMA_Src_t)(WRam_Mode + 2);
			}
#endif
		}
	}

	// Set the DMA BUSY bit.
	d->Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, true);

	// Save the length, and clear the length registers.
	// FIXME: Should be updating the length registers
	// as we go along, but the M68K isn't properly "locked"
	// for the entire time period.
	DMAT_Length = length;
	d->set_DMA_Length(0);

	switch (DMA_TYPE(src_component, dest_component)) {
		case DMA_TYPE(VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_VRAM):
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_CRAM):
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_VSRAM):
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_VRAM):
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_CRAM):
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_VSRAM):
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_VRAM):
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_CRAM):
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_VSRAM):
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			d->T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		default:
			// Invalid DMA mode.
			d->VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;
			break;
	}

	return;
}

}
