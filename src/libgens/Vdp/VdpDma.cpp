/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpDma.cpp: VDP class: DMA functions.                                   *
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

// FIXME: All of the DMA code needs to be completely reworked.

#include "Vdp.hpp"
#include "Vdp_p.hpp"

// LOG_MSG() subsystem.
#include "macros/log_msg.h"

// M68K CPU.
#include "cpu/M68K_Mem.hpp"
#include "Cartridge/RomCartridgeMD.hpp"

namespace LibGens {

/** VdpPrivate **/

/**
 * Perform a DMA Fill operation. (Called from Vdp::writeDataMD().)
 * @param data 16-bit data.
 */
void VdpPrivate::DMA_Fill(uint16_t data)
{
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
				if (VRam.u8[address ^ 1 ^ U16DATA_U8_INVERT] != fill_hi) {
					VRam.u8[address ^ 1 ^ U16DATA_U8_INVERT] = fill_hi;
					cache.mark_dirty(address ^ 1 ^ U16DATA_U8_INVERT);
				}
				if ((address & Spr_Tbl_Mask) == Spr_Tbl_Addr) {
					// Sprite Attribute Table.
					SprAttrTbl_m5.b[(address & ~Spr_Tbl_Mask) ^ U16DATA_U8_INVERT] = fill_hi;
				}
				address += VDP_Reg.m5.Auto_Inc;
				address &= VRam_Mask;
			} while (--length != 0);
			break;

		case VdpTypes::CD_DEST_CRAM_WRITE:
			// Write to CRAM.
			// TODO: FIFO emulation.
			do {
				palette.writeCRam_16((address & 0x7E), data);
				address += VDP_Reg.m5.Auto_Inc;
				address &= VRam_Mask;
			} while (--length != 0);
			break;

		case VdpTypes::CD_DEST_VSRAM_WRITE:
			// Write to VSRAM.
			// TODO: FIFO emulation.
			do {
				VSRam.u16[(address & 0x7E) >> 1] = data;
				address += VDP_Reg.m5.Auto_Inc;
				address &= VRam_Mask;
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
	VDP_Ctrl.address = (address & VRam_Mask);

	// NOTE: DMA FILL updates the DMA source address,
	// even though it isn't used.
	inc_DMA_Src_Adr(q->DMAT_Length);
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
	// FIXME: This breaks "NBA Jam" and "NBA Jam: Tournament Edition".
	// Disabling it breaks a test in VDPFIFOTesting.
	// Clearly there's an issue with DMA timing somewhere.
	//inc_DMA_Src_Adr(q->DMAT_Length);

	// Update DMA.
	int cycles = q->updateDMA();
	M68K::ReleaseCycles(cycles);
}

/**
 * Process a DMA control word write.
 */
void VdpPrivate::processDmaCtrlWrite(void)
{
	// Check for DMA FILL.
	if (VDP_Ctrl.DMA_Mode == 0x80) {
		// DMA FILL.
		// Operation is processed on DATA WRITE.
		return;
	}

	// Determine the DMA destination.
	// NOTE: Ignoring CD0.
	VdpPrivate::DMA_Dest_t dest_component;
	switch (VDP_Ctrl.code & VdpTypes::CD_DEST_MASK) {
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
	uint32_t src_address = DMA_Src_Adr();			// Src Address / 2
	uint32_t dest_address = VDP_Ctrl.address & VRam_Mask;	// Dest Address

	int length = DMA_Length();
	if (length == 0) {
		// DMA length is zero.
		if (q->options.zeroLengthDMA) {
			// Zero-Length DMA transfers are enabled.
			// Ignore this request.
			VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;
			return;
		}

		// Zero-Length DMA trnasfers are disabled.
		// The MD VDP decrements the DMA length counter before checking if it has
		// reached zero. So, doing a zero-length DMA request will actually do a
		// DMA request for 65,536 words.
		length = 0x10000;
	}

	// Check for DMA COPY.
	if (VDP_Ctrl.DMA_Mode == 0xC0) {
		// DMA COPY.
		// TODO: Verify that CD4 is set; if it isn't, VDP should lock up.
		src_address &= VRam_Mask;
		Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, true);	// Set the DMA BUSY bit.
		q->DMAT_Length = length;
		DMAT_Type = VdpPrivate::DMAT_COPY;
		set_DMA_Length(0);

		// TODO: Is this correct with regards to endianness?
		// TODO: Do DMA COPY line-by-line instead of all at once.
		do {
			uint8_t src = VRam.u8[src_address];
			if (VRam.u8[dest_address] != src) {
				VRam.u8[dest_address] = src;
				cache.mark_dirty(dest_address);
			}
			if ((dest_address & Spr_Tbl_Mask) == Spr_Tbl_Addr) {
				// Sprite Attribute Table.
				SprAttrTbl_m5.b[(dest_address & ~Spr_Tbl_Mask) ^ U16DATA_U8_INVERT] = src;
			}

			// Increment the addresses.
			src_address++;
			dest_address += VDP_Reg.m5.Auto_Inc;

			// Mask the addresses.
			src_address &= VRam_Mask;
			dest_address &= VRam_Mask;
		} while (--length != 0);

		// Save the new addresses.
		// NOTE: DMA COPY uses bytes, not words.
		inc_DMA_Src_Adr(q->DMAT_Length);	// TODO: Should DMA_Src_Adr_H's DMA flags be cleared?
		VDP_Ctrl.address = dest_address;
		return;
	}

	if (VDP_Ctrl.DMA_Mode & 0x80) {
		// TODO: What does this mean?
		VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;
		return;
	}

	// Multiply the source address by two to get the real source address.
	src_address *= 2;

	// Determine the source component.
	VdpPrivate::DMA_Src_t src_component = VdpPrivate::DMA_SRC_ROM;	// TODO: Determine a better default.
	int WRam_Mode;

	// Maximum ROM source address: 4 MB for Sega CD or 32X, 10 MB for standard MD.
	const uint32_t maxRomSrcAddress =
		((q->SysStatus.SegaCD || q->SysStatus._32X)
		 ? 0x3FFFFF
		 : 0x9FFFFF);

	if (src_address <= maxRomSrcAddress) {
		// Main ROM.
		src_component = VdpPrivate::DMA_SRC_ROM;
	} else if (!q->SysStatus.SegaCD) {
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
	Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, true);

	// Save the length, and clear the length registers.
	// FIXME: Should be updating the length registers
	// as we go along, but the M68K isn't properly "locked"
	// for the entire time period.
	q->DMAT_Length = length;
	// FIXME: This is needed in order to fix VDPFIFOTesting
	// Test #24: "DMA Transfer Length Reg Update", but it
	// breaks "Frank Thomas Big Hurt Baseball",
	// "NBA Jam", and "NBA Jam: Tournament Edition".
	//set_DMA_Length(0);

	switch (DMA_TYPE(src_component, dest_component)) {
		case DMA_TYPE(VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_VRAM):
			T_DMA_Loop<VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_CRAM):
			T_DMA_Loop<VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_VSRAM):
			T_DMA_Loop<VdpPrivate::DMA_SRC_ROM, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_VRAM):
			T_DMA_Loop<VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_CRAM):
			T_DMA_Loop<VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_VSRAM):
			T_DMA_Loop<VdpPrivate::DMA_SRC_M68K_RAM, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_PRG_RAM, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_VRAM):
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_CRAM):
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_VSRAM):
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_2M, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_0, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_1M_1, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_0, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_VRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_CRAM>();
			break;

		case DMA_TYPE(VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<VdpPrivate::DMA_SRC_WORD_RAM_CELL_1M_1, VdpPrivate::DMA_DEST_VSRAM>();
			break;

		default:
			// Invalid DMA mode.
			VDP_Ctrl.code &= ~VdpTypes::CD_DMA_ENABLE;
			break;
	}
}

/** Vdp **/

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
	    (!(d->VDP_Reg.m5.Set2 & VDP_REG_M5_SET2_DISP)))
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

}
