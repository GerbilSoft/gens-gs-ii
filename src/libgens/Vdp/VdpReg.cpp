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
	VDP_Mode = (VDP_Mode_t)
		   (((Set2 & 0x10) >> 4) |	// M1
		    ((Set1 & 0x02))      |	// M2
		    ((Set2 & 0x08) >> 1) |	// M3
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
			palette.setPalMode(VdpPalette::PALMODE_MD);
			palette.setMdColorMask(!(VDP_Mode & 0x08));	// M4/PSEL
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
 * Set the value of a register. (Mode 5 only!)
 * @param reg_num Register number.
 * @param val New value for the register.
 */
void VdpPrivate::setReg(int reg_num, uint8_t val)
{
	if (reg_num < 0)
		return;

	/**
	 * Highest addressable register depends on VDP mode.
	 * - Mode 5: Register 23
	 * - Mode 4: Register 10
	 * - TODO: Other modes? (assuming 10 for now)
	 */
	const int max_reg = ((VDP_Mode & VDP_MODE_M5) ? 23 : 10);
	if (reg_num > max_reg)
		return;

	// Save the new register value.
	VDP_Reg.reg[reg_num] = val;

	// Temporary value for calculation.
	unsigned int tmp;

	// Update things affected by the register.
	switch (reg_num) {
		case 0:
		case 1:
			// Mode Set 1, Mode Set 2.
			q->updateIRQLine(0);
			updateVdpMode();

			// Reset the masks and addresses for 64 KB mode.
			VRam_Mask = 0xFFFF;
			ScrA_Addr &= 0xFFFF;
			ScrB_Addr &= 0xFFFF;
			Spr_Addr &= 0xFFFF;

			// If 128 KB mode is enabled, append the bits.
			if (is128KB()) {
				VRam_Mask = 0x1FFFF;
				ScrA_Addr |= ((VDP_Reg.m5.Pat_Data_Adr & 0x01) << 16);
				ScrB_Addr |= ((VDP_Reg.m5.Pat_Data_Adr & 0x10) << 12);
				Spr_Addr |= ((VDP_Reg.m5.Spr_Pat_Adr & 0x20) << 11);
			}
			break;

		case 2:
			// Scroll A base address.
			ScrA_Addr = (val << 10) & 0xE000;
			if (is128KB()) {
				ScrA_Addr |= ((VDP_Reg.m5.Pat_Data_Adr & 0x01) << 16);
			}
			break;

		case 3:
			// Window base address.
			Win_Addr = (val << 10) & Win_Mask;
			break;

		case 4:
			// Scroll B base address.
			ScrB_Addr = (val << 13) & 0xE000;
			if (is128KB()) {
				ScrB_Addr |= ((VDP_Reg.m5.Pat_Data_Adr & 0x10) << 12);
			}
			break;

		case 5:
		case 6:
			// 5: Sprite Attribute Table base address.
			// 6: Sprite Pattern Generator base address. (128 KB mode only)
			Spr_Addr = (VDP_Reg.m5.Spr_Att_Adr << 9) & Spr_Mask;
			if (is128KB()) {
				Spr_Addr |= ((VDP_Reg.m5.Spr_Pat_Adr & 0x20) << 11);
			}

			// Update the Sprite Attribute Table.
			// TODO: Only set this if the actual value has changed.
			m_updateFlags.VRam_Spr = true;
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

			// Update the Shadow/Highlight setting.
			palette.setMdShadowHighlight(!!(VDP_Reg.m5.Set4 & 0x08));

			// TODO: Split the full recalculation into a separate function?
			if (isH40()) {
				// H40 mode.
				H_Cell = 40;
				H_Win_Shift = 6;
				H_Pix = 320;
				H_Pix_Begin = 0;

				// Update the table masks.
				Win_Mask = 0xF000;
				Spr_Mask = 0xFC00;
			} else {
				// H32 mode.
				H_Cell = 32;
				H_Win_Shift = 5;
				H_Pix = 256;
				H_Pix_Begin = 32;

				// Update the table masks.
				Win_Mask = 0xF800;
				Spr_Mask = 0xFE00;
			}

			// Check the window horizontal position.
			Win_X_Pos = ((VDP_Reg.m5.Win_H_Pos & 0x1F) * 2);
			if (Win_X_Pos > H_Cell)
				Win_X_Pos = H_Cell;

			// Update the Window and Sprite Attribute Table base addresses.
			Win_Addr = (VDP_Reg.m5.Pat_Win_Adr << 10) & Win_Mask;
			Spr_Addr = (VDP_Reg.m5.Spr_Att_Adr << 9) & Spr_Mask;
			break;

		case 13:
			// H Scroll Table base address.
			H_Scroll_Addr = (val << 10) & 0xFC00;
			break;

		case 14:
			// Nametable Pattern Generator base address.
			// (128 KB mode only.)
			ScrA_Addr = (VDP_Reg.m5.Pat_ScrA_Adr << 10) & 0xE000;
			ScrB_Addr = (VDP_Reg.m5.Pat_ScrB_Adr << 13) & 0xE000;
			if (is128KB()) {
				ScrA_Addr |= ((val & 0x01) << 16);
				ScrB_Addr |= ((val & 0x10) << 12);
			}
			break;

		case 16: {
			// Scroll Size.
			tmp = (val & 0x3);
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

}
