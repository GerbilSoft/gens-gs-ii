/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpReg.cpp: VDP class: Register functions.                              *
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

// Vdp private class.
#include "Vdp_p.hpp"

namespace LibGens {

/**
 * Update VDP_Mode.
 */
void VdpPrivate::updateVdpMode(void)
{
	const VDP_Mode_t prevVdpMode = VDP_Mode;
	const register uint8_t Set1 = VDP_Reg.m5.Set1;
	const register uint8_t Set2 = VDP_Reg.m5.Set2;
	// NOTE: M1, M2, and M3 descriptions match the official
	// TMS9918A datasheet. M2 is *not* Graphic II, since
	// this mode was not present in the original TMS9918,
	// and was added in the TMS9918A.
	VDP_Mode = (VDP_Mode_t)
		   (((Set2 & 0x10) >> 4) |	// M1 (Text)
		    ((Set2 & 0x08) >> 2) |	// M2 (Multicolor)
		    ((Set1 & 0x02) << 1) |	// M3 (Graphic II)
		    ((Set1 & 0x04) << 1) |	// M4/PSEL
		    ((Set2 & 0x04) << 2));	// M5

	if (!(Set2 & 0x08)) {
		// V28 mode. Reset the NTSC V30 roll values.
		q->VDP_Lines.NTSC_V30.Offset = 0;
		q->VDP_Lines.NTSC_V30.VBlank_Div = 0;
	}

	// If the VDP mode has changed, CRam needs to be updated.
	if (prevVdpMode != VDP_Mode) {
		// Update the VDP mode variables.
		if (VDP_Mode & VDP_MODE_M5) {
			// Mode 5.
			// TODO: Only if we weren't in Mode 5 before?
			palette.setPalMode(VdpPalette::PALMODE_MD);
			palette.setMdColorMask(!(VDP_Mode & VDP_MODE_M4));
		} else {
			// TODO: Support other palette modes.
		}
	}

	// Initialize Vdp::VDP_Lines.
	// Don't reset the VDP current line variables here,
	// since this might not be the beginning of the frame.
	q->updateVdpLines(false);
}

/**
 * Update the VDP address cache. (Mode 5)
 * @param updateMask 1 == 64KB/128KB; 2 == H32/H40; 3 == both
 */
void VdpPrivate::updateVdpAddrCache_m5(unsigned int updateMask)
{
	// TODO: Optimize based on updateMask?
	((void)updateMask);

	// Reset the Window and Sprite masks for H32/H40.
	// TODO: Make a const array and copy from that?
	if (isH40()) {
		// H40 mode.
		H_Cell = 40;
		H_Win_Shift = 6;
		H_Pix = 320;
		H_Pix_Begin = 0;

		// Update the table masks.
		Win_Tbl_Mask = 0xF000;
		Spr_Tbl_Mask = 0xFC00;
	} else {
		// H32 mode.
		H_Cell = 32;
		H_Win_Shift = 5;
		H_Pix = 256;
		H_Pix_Begin = 32;

		// Update the table masks.
		Win_Tbl_Mask = 0xF800;
		Spr_Tbl_Mask = 0xFE00;
	}

	if (!is128KB()) {
		// 64 KB mode.
		VRam_Mask = 0xFFFF;
		ScrA_Tbl_Mask = 0xE000;
		ScrB_Tbl_Mask = 0xE000;
		H_Scroll_Tbl_Mask = 0xFC00;
	} else {
		// 128 KB mode.
		VRam_Mask = 0x1FFFF;
		ScrA_Tbl_Mask = 0x1E000;
		ScrB_Tbl_Mask = 0x1E000;
		H_Scroll_Tbl_Mask = 0x1FC00;
		Win_Tbl_Mask |= 0x10000;
		Spr_Tbl_Mask |= 0x10000;
	}

	// Check the window horizontal position.
	Win_X_Pos = ((VDP_Reg.m5.Win_H_Pos & 0x1F) * 2);
	if (Win_X_Pos > H_Cell)
		Win_X_Pos = H_Cell;

	// Update nametable addresses.
	ScrA_Tbl_Addr = (VDP_Reg.m5.Pat_ScrA_Adr << 10) & ScrA_Tbl_Mask;
	Win_Tbl_Addr = (VDP_Reg.m5.Pat_Win_Adr << 10) & Win_Tbl_Mask;
	ScrB_Tbl_Addr = (VDP_Reg.m5.Pat_ScrB_Adr << 13) & ScrB_Tbl_Mask;
	Spr_Tbl_Addr = (VDP_Reg.m5.Spr_Att_Adr << 9) & Spr_Tbl_Mask;
	Win_Tbl_Addr = (VDP_Reg.m5.Pat_Win_Adr << 10) & Win_Tbl_Mask;
	H_Scroll_Tbl_Addr = (VDP_Reg.m5.H_Scr_Adr << 10) & H_Scroll_Tbl_Mask;

	// Update pattern generator addresses.
	if (!is128KB()) {
		// All pattern generator base addresses are 0.
		ScrA_Gen_Addr = 0;
		ScrB_Gen_Addr = 0;
		Spr_Gen_Addr = 0;
	} else {
		// TODO: Cache ScrA_A16?
		const uint32_t ScrA_A16 = ((VDP_Reg.m5.Pat_Data_Adr & 0x01) << 16);
		ScrA_Gen_Addr = ScrA_A16;
		if (ScrA_A16) {
			ScrB_Gen_Addr = ((VDP_Reg.m5.Pat_Data_Adr & 0x10) << 12);
		}
		Spr_Gen_Addr = ((VDP_Reg.m5.Spr_Pat_Adr & 0x20) << 11);	// Update the Window and Sprite Attribute Table base addresses.
	}
}

/**
 * Set the value of a register. (Mode 5 only!)
 * @param reg_num Register number.
 * @param val New value for the register.
 */
void VdpPrivate::setReg(int reg_num, uint8_t val)
{
	if (reg_num < 0)
		return;

	/**
	 * Register write notes:
	 * - Genesis VDP: mask = 0x1F, max = 23
	 *   - max = 10 when Mode 5 is disabled.
	 *   - mask is 0x0F when in SMS system mode.
	 * - SMS1/2 VDP: mask = 0x0F, max = 10
	 * - TMS9918A: mask = 0x07, max = 7
	 */

	/**
	 * Highest addressable register depends on VDP mode.
	 * - Mode 5: Register 23
	 * - Mode 4: Register 10
	 * - TODO: Other modes? (assuming 10 for now)
	 */
	const int max_reg = ((VDP_Mode & VDP_MODE_M5) ? 23 : 10);
	if (reg_num > max_reg)
		return;

	// Check what bits have changed.
	// Used to optimize away some recalculations.
	const uint8_t diff = (VDP_Reg.reg[reg_num] ^ val);

	// Save the new register value.
	// NOTE: Cannot optimize away write if the value is
	// the same because some registers trigger operations,
	// e.g. DMA registers. (Maybe Mode 4 or less?)
	VDP_Reg.reg[reg_num] = val;

	// Update things affected by the register.
	switch (reg_num) {
		case 0:	// Mode Set 1
			q->updateIRQLine(0);

			if (diff & 0x06) {
				// PSEL and/or M3 have changed.
				// TODO: Handle them separately?
				updateVdpMode();
				updateVdpAddrCache_m5(1);
			}
			break;

		case 1:	// Mode Set 2
			if (diff & 0x8C) {
				// VRAM, M1, M2, and/or M5 have changed.
				updateVdpMode();
			}
			// TODO: If emulating TMS9918A, and VRAM bit has changed,
			// swap memory organization from 4K to 16K or vice-versa.
			break;

		case 2:
			// Scroll A base address.
			ScrA_Tbl_Addr = (val << 10) & ScrA_Tbl_Mask;
			break;

		case 3:
			// Window base address.
			Win_Tbl_Addr = (val << 10) & Win_Tbl_Mask;
			break;

		case 4:
			// Scroll B base address.
			ScrB_Tbl_Addr = (val << 13) & ScrB_Tbl_Mask;
			break;

		case 5:
			// Sprite Attribute Table base address.
			Spr_Tbl_Addr = (VDP_Reg.m5.Spr_Att_Adr << 9) & Spr_Tbl_Mask;

			/**
			 * NOTE: The Sprite Attribute Table does *not*
			 * get updated if this register changes.
			 * In order for it to get updated, the ROM
			 * will need to rewrite the SAT to VRAM
			 * using either port writes or DMA.
			 */
			break;

		case 6:
			// Sprite Pattern Generator base address.
			// (128 KB mode only)
			if (!is128KB()) {
				// 64 KB mode.
				// All pattern generator base addresses are 0.
				Spr_Gen_Addr = 0;
			} else {
				// 128 KB mode.
				Spr_Gen_Addr = ((val & 0x20) << 11);
			}
			break;

		case 7:
			// Background Color.
			// TODO: This is only valid for MD. SMS and GG function differently.
			// NOTE: This will automatically mark CRam as dirty if the index has changed.
			palette.setBgColorIdx(val & 0x3F);
			break;

		case 11: {
			// Mode Set 3.
			static const uint8_t H_Scroll_Mask_Table[4] = {0x00, 0x07, 0xF8, 0xFF};

			// Check the Vertical Scroll mode. (Bit 3)
			// 0: Full scrolling. (Mask == 0)
			// 1: 2CELL scrolling. (Mask == 0x7E)
			V_Scroll_MMask = ((val & 4) ? 0x7E : 0);

			// Horizontal Scroll mode
			H_Scroll_Mask = H_Scroll_Mask_Table[val & 3];

			break;
		}

		case 12:
			// Mode Set 4.
			if (diff & 0x08) {
				// Update the Shadow/Highlight setting.
				palette.setMdShadowHighlight(!!(VDP_Reg.m5.Set4 & 0x08));
			}
			if (diff & 0x81) {
				// H32/H40 mode has changed.
				updateVdpAddrCache_m5(2);
			}
			break;

		case 13:
			// H Scroll Table base address.
			H_Scroll_Tbl_Addr = (val << 10) & H_Scroll_Tbl_Mask;
			break;

		case 14:
			// Nametable Pattern Generator base address.
			// (128 KB mode only.)
			if (!is128KB()) {
				// 64 KB mode.
				// All pattern generator base addresses are 0.
				ScrA_Gen_Addr = 0;
				ScrB_Gen_Addr = 0;
			} else {
				// 128 KB mode.
				// TODO: Cache ScrA_A16?
				const uint32_t ScrA_A16 = ((VDP_Reg.m5.Pat_Data_Adr & 0x01) << 16);
				ScrA_Gen_Addr = ScrA_A16;
				if (ScrA_A16) {
					ScrB_Gen_Addr = ((VDP_Reg.m5.Pat_Data_Adr & 0x10) << 12);
				}
			}
			break;

		case 16: {
			// Scroll Size.
			int tmp = (val & 0x3);
			tmp |= (val & 0x30) >> 2;

			/**
			 * Scroll size table.
			 * Format:
			 * - idx 0: H_Scroll_CMul
			 * - idx 1: H_Scroll_CMask
			 * - idx 2: V_Scroll_CMask
			 * - idx 3: reserved (padding)
			 */
			static const struct {
				uint8_t H_Scroll_CMul;
				uint8_t H_Scroll_CMask;
				uint8_t V_Scroll_CMask;
				uint8_t reserved;
			} Scroll_Size_Tbl[] = {
				// V32_H32 (VXX_H32)      // V32_H64 (VXX_H64)
				{0x05, 0x1F, 0x1F, 0x00}, {0x06, 0x3F, 0x1F, 0x00},
				// V32_HXX (V??_HXX)      // V32_H128 (V??_H128)
				{0x06, 0x3F, 0x00, 0x00}, {0x07, 0x7F, 0x1F, 0x00},

				// V64_H32                // V64_H64 (V128_H64)
				{0x05, 0x1F, 0x3F, 0x00}, {0x06, 0x3F, 0x3F, 0x00},
				// V64_HXX (V??_HXX)      // V64_H128 (V??_H128)
				{0x06, 0x3F, 0x00, 0x00}, {0x07, 0x7F, 0x1F, 0x00},

				// VXX_H32 (V32_H32)      // VXX_H64 (V32_H64)
				{0x05, 0x1F, 0x1F, 0x00}, {0x06, 0x3F, 0x1F, 0x00},
				// VXX_HXX (V??_HXX)      // VXX_H128 (V??_H128)
				{0x06, 0x3F, 0x00, 0x00}, {0x07, 0x7F, 0x1F, 0x00},

				// V128_H32               // V128_H64 (V64_H64)
				{0x05, 0x1F, 0x7F, 0x00}, {0x06, 0x3F, 0x3F, 0x00},
				// V128_HXX (V??_HXX)     // V128_H128 (V??_H128)
				{0x06, 0x3F, 0x00, 0x00}, {0x07, 0x7F, 0x1F, 0x00}
			};

			// Get the values from the scroll size table.
			H_Scroll_CMul  = Scroll_Size_Tbl[tmp].H_Scroll_CMul;
			H_Scroll_CMask = Scroll_Size_Tbl[tmp].H_Scroll_CMask;
			V_Scroll_CMask = Scroll_Size_Tbl[tmp].V_Scroll_CMask;
			break;
		}

		case 17:
			// Window H position.
			Win_X_Pos = ((val & 0x1F) * 2);
			if (Win_X_Pos > H_Cell)
				Win_X_Pos = H_Cell;
			break;

		case 18:
			// Window V position.
			Win_Y_Pos = (val & 0x1F);
			break;

		case 19:
		case 20:
			// DMA Length.
			break;

		case 21:
		case 22:
			// DMA Source Address: Low, Mid.
			break;

		case 23:
			// DMA Source Address: High.
			// High 2 bits indicate DMA mode.
			VDP_Ctrl.DMA_Mode = (val & 0xC0);
			break;

		default:	// to make gcc shut up
			break;
	}
}

/**
 * Reset the VDP registers.
 * @param bootRomFix If true, set the registers as the boot ROM would.
 */
void VdpPrivate::resetRegisters(bool bootRomFix)
{
	// Always clear the actual register data before
	// updating the cache.
	memset(VDP_Reg.reg, 0, sizeof(VDP_Reg.reg));

	// TODO: Handle systems other than MD here.
	if (bootRomFix) {
		/**
		 * Assuming TMSS is not present.
		 * Set the VDP registers the same way TMSS does:
		 * - Reg.00: 0x04	(ModeSet1) Hint OFF, Full Palette, No HV latch, VDP disabled
		 * - Reg.01: 0x04	(ModeSet2) 64 KB VRAM, Mode 5, Video OFF, Vint OFF, DMA disabled, 224 lines
		 * - Reg.02: 0x30	PlaneA = 0xC000
		 * - Reg.03: 0x3C	Window = 0xF000
		 * - Reg.04: 0x07	PlaneB = 0xE000
		 * - Reg.05: 0x6C	Sprite = 0xD800
		 * - Reg.06: 0x00	(128 KB) Sprite Pattern Generator base address (unused)
		 * - Reg.07: 0x00	Background color
		 * - Reg.08: 0x00	SMS X scroll, unused in Mode 5
		 * - Reg.09: 0x00	SMS Y scroll, unused in Mode 5
		 * - Reg.10: 0xFF	Hint lines (0xFF = disabled)
		 * - Reg.11: 0x00	(ModeSet3) THint OFF, Full Y Scroll, Full X Scroll
		 * - Reg.12: 0x81	(ModeSet4) H40, S/H disable, no interlace
		 * - Reg.13: 0x37	HScroll = 0xDC00
		 * - Reg.14: 0x00	(128 KB) Nametable Pattern Generator base address (unused)
		 * - Reg.15: 0x02	VDP address auto-increment value
		 * - Reg.16: 0x01	Scroll plane size: 64x32
		 * - Reg.17: 0x00	Window width
		 * - Reg.18: 0x00	Window height
		 * - Reg.19: 0x00	DMA Length L
		 * - Reg.20: 0x00	DMA Length H
		 * - Reg.21: 0x00	DMA Source L
		 * - Reg.22: 0x00	DMA Source M
		 * - Reg.23: 0x80	DMA Source H
		 *
		 * NOTE: These are the effective register values *after*
		 * TMSS is run, not the initial values.
		 *
		 * Reference: TmEE's TMSS disassembly.
		 */

		static const uint8_t vdp_reg_init_m5[24] = {
			0x04, 0x04, 0x30, 0x3C, 0x07, 0x6C, 0x00, 0x00,
			0x00, 0x00, 0xFF, 0x00, 0x81, 0x37, 0x00, 0x02,
			0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
		};
		for (int i = 0; i < ARRAY_SIZE(vdp_reg_init_m5); i++) {
			setReg(i, vdp_reg_init_m5[i]);
		}
	} else {
		// Registers are cleared.
		// Update the caches.
		updateVdpMode();
		updateVdpAddrCache_m5(3);

		// Scroll size isn't updated by the above functions.
		setReg(16, 0);
	}

	// Always set Reg.10 to 0xFF to prevent
	// spurious horizontal interrupts.
	setReg(10, 0xFF);
}

}
