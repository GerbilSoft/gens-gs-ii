/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpIo.cpp: VDP class: I/O functions.                                    *
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

#include "Vdp.hpp"

// LOG_MSG() subsystem.
#include "macros/log_msg.h"

// M68K CPU.
#include "cpu/star_68k.h"
#include "cpu/M68K_Mem.hpp"

/** Static member initialization. **/
#include "VdpIo_static.hpp"

// Emulation Context.
#include "../EmuContext.hpp"

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

namespace LibGens
{

/**
 * Vdp::Int_Ack(): Acknowledge an interrupt.
 * @return ???
 */
uint8_t Vdp::Int_Ack(void)
{
	if ((VDP_Reg.m5.Set2 & 0x20) && (VDP_Int & 0x08))
	{
		// VBlank interrupt acknowledge.
		VDP_Int &= ~0x08;
		
		uint8_t rval_mask = VDP_Reg.m5.Set1;
		rval_mask &= 0x10;
		rval_mask >>= 2;
		
		return ((VDP_Int) & rval_mask);
	}
	
	// Reset the interrupt counter.
	VDP_Int = 0;
	return 0;
}


/**
 * Vdp::Update_IRQ_Line(): Update the IRQ line.
 */
void Vdp::Update_IRQ_Line(void)
{
	// TODO: HBlank interrupt should take priority over VBlank interrupt.
	if ((VDP_Reg.m5.Set2 & 0x20) && (VDP_Int & 0x08))
	{
		// VBlank interrupt.
		M68K::Interrupt(6, -1);
		return;
	}
	else if ((VDP_Reg.m5.Set1 & 0x10) && (VDP_Int & 0x04))
	{
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
 * updateVdpLines(): Update VDP_Lines based on CPU and VDP mode settings.
 * @param resetCurrent If true, reset VDP_Lines.Display.Current and VDP_Lines.Visible.Current.
 */
void Vdp::updateVdpLines(bool resetCurrent)
{
	// Arrays of values.
	// Indexes: 0 == 192 lines; 1 == 224 lines; 2 == 240 lines.
	static const int VisLines_Total[3] = {192, 224, 240};
	static const int VisLines_Border_Size[3] = {24, 8, 0};
	static const int VisLines_Current_NTSC[3] = {-40, -24, 0};
	static const int VisLines_Current_PAL[3] = {-67+1, -51+1, -43+1};
	
	// Initialize VDP_Lines.Display.
	VDP_Lines.Display.Total = (Reg_Status.isPal() ? 312 : 262);
	if (resetCurrent)
		VDP_Lines.Display.Current = 0;
	
	// Line offset.
	int LineOffset;
	
	// Check the current video mode.
	// NOTE: Unlike Gens/GS, we don't check if a ROM is loaded because
	// the VDP code isn't used at all in Gens/GS II during "idle".
	if (VDP_Mode & VDP_MODE_M5)
	{
		// Mode 5. Must be either 224 lines or 240 lines.
		if (VDP_Mode & VDP_MODE_M3)
			LineOffset = 2; // 240 lines.
		else
			LineOffset = 1; // 224 lines.
	}
	else
	{
		// Mode 4 or TMS9918 mode.
		// Mode 4 may be 192 lines, 224 lines, or 240 lines.
		// Modes 0-3 may only be 192 lines.
		// TODO: If emulating SMS1, disable 224-line and 240-line modes.
		switch (VDP_Mode)
		{
			case 0x0B:
				// Mode 4: 224 lines.
				LineOffset = 1;
				break;
			case 0x0E:
				// Mode 4: 240 lines.
				LineOffset = 2;
				break;
			default:
				// Modes 0-4: 192 lines.
				LineOffset = 0;
				break;
		}
	}
	
	VDP_Lines.Visible.Total = VisLines_Total[LineOffset];
	VDP_Lines.Visible.Border_Size = VisLines_Border_Size[LineOffset];
	if (resetCurrent)
	{
		// Reset VDP_Lines.Visible.Current.
		VDP_Lines.Visible.Current = (Reg_Status.isPal()
						? VisLines_Current_PAL[LineOffset]
						: VisLines_Current_NTSC[LineOffset]);
	}
	
	// Check interlaced mode.
	Interlaced = (VdpTypes::Interlaced_t)
			(((VDP_Reg.m5.Set4 & 0x02) >> 1) |	// LSM0
			 ((VDP_Reg.m5.Set4 & 0x04) >> 1));	// LSM1
}


/**
 * Vdp::Check_NTSC_V30_VBlank(): Check if VBlank is allowed in NTSC V30 mode.
 */
void Vdp::Check_NTSC_V30_VBlank(void)
{
	// TODO: Only do this in Mode 5, and maybe Mode 4 if SMS2 is in use.
	if (Reg_Status.isPal() || !(VDP_Reg.m5.Set2 & 0x08))
	{
		// Either we're in PAL mode, where V30 is allowed, or V30 isn't set.
		// VBlank is always OK.
		// TODO: Clear the NTSC V30 offset?
		VDP_Lines.NTSC_V30.VBlank_Div = 0;
		return;
	}
	
	// NTSC V30 mode. Simulate screen rolling.
	
	// If VDP_Lines.NTSC_V30.VBlank is set, we can't do a VBlank.
	// This effectively divides VBlank into 30 Hz.
	// See http://gendev.spritesmind.net/forum/viewtopic.php?p=8128#8128 for more information.
	VDP_Lines.NTSC_V30.VBlank_Div = !VDP_Lines.NTSC_V30.VBlank_Div;
	
	if (VdpEmuOptions.ntscV30Rolling)
	{
		VDP_Lines.NTSC_V30.Offset += 11;	// TODO: Figure out a good offset increment.
		VDP_Lines.NTSC_V30.Offset %= 240;	// Prevent overflow.
	}
	else
	{
		// Rolling is disabled.
		VDP_Lines.NTSC_V30.Offset = 0;
	}
}


/**
 * Vdp::Update_Mode(): Update VDP_Mode.
 */
inline void Vdp::Update_Mode(void)
{
	const unsigned int prevVdpMode = VDP_Mode;
	const register uint8_t Set1 = VDP_Reg.m5.Set1;
	const register uint8_t Set2 = VDP_Reg.m5.Set2;
	VDP_Mode = ((Set2 & 0x10) >> 4) |	// M1
		   ((Set1 & 0x02))      |	// M2
		   ((Set2 & 0x08) >> 1)	|	// M3
		   ((Set1 & 0x04) << 1) |	// M4/PSEL
		   ((Set2 & 0x04) << 2);	// M5
	
	if (!(Set2 & 0x08))
	{
		// V28 mode. Reset the NTSC V30 roll values.
		VDP_Lines.NTSC_V30.Offset = 0;
		VDP_Lines.NTSC_V30.VBlank_Div = 0;
	}
	
	// If the VDP mode has changed, CRam needs to be updated.
	if (prevVdpMode != VDP_Mode)
	{
		// Update the VDP mode variables.
		if (VDP_Mode & 0x10)
		{
			// Mode 5.
			m_palette.setPalMode(VdpPalette::PALMODE_MD);
			m_palette.setMdColorMask(!(VDP_Mode & 0x08));	// M4/PSEL
		}
		else
		{
			// TODO: Support other palette modes.
		}
	}
	
	// Initialize Vdp::VDP_Lines.
	// Don't reset the VDP current line variables here,
	// since this might not be the beginning of the frame.
	updateVdpLines(false);
}


/**
 * Set the value of a register. (Mode 5 only!)
 * @param reg_num Register number.
 * @param val New value for the register.
 */
void Vdp::setReg(int reg_num, uint8_t val)
{
	if (reg_num < 0 || reg_num >= 24)
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
			Update_IRQ_Line();

			// Update the VDP mode.
			Update_Mode();
			break;

		case 2:
			// Scroll A base address.
			ScrA_Addr = (val & 0x38) << 10;
			break;

		case 3:
			// Window base address.
			if (isH40())
				Win_Addr = (val & 0x3C) << 10;	// H40.
			else
				Win_Addr = (val & 0x3E) << 10;	// H32.
			break;

		case 4:
			// Scroll B base address.
			ScrB_Addr = (val & 0x07) << 13;
			break;

		case 5:
			// Sprite Attribute Table base address.
			if (isH40())
				Spr_Addr = (val & 0x7E) << 9;
			else
				Spr_Addr = (val & 0x7F) << 9;

			// Update the Sprite Attribute Table.
			// TODO: Only set this if the actual value has changed.
			m_updateFlags.VRam_Spr = true;
			break;

		case 7:
			// Background Color.
			// TODO: This is only valid for MD. SMS and GG function differently.
			// NOTE: This will automatically mark CRam as dirty if the index has changed.
			m_palette.setBgColorIdx(val & 0x3F);
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
			m_palette.setMdShadowHighlight(!!(VDP_Reg.m5.Set4 & 0x08));

			// H40 mode is activated by setting VDP_Reg.m5.Set4, bit 0 (0x01, RS1).
			// Bit 7 (0x80, RS0) is also needed, but RS1 is what tells the VDP
			// to increase the pixel counters to 320px per line.
			// Source: http://wiki.megadrive.org/index.php?title=VDPRegs_Addendum (Jorge)
			if (val & 0x01) {
				// H40 mode.
				H_Cell = 40;
				H_Win_Shift = 6;
				H_Pix = 320;
				H_Pix_Begin = 0;

				// Check the window horizontal position.
				Win_X_Pos = ((VDP_Reg.m5.Win_H_Pos & 0x1F) * 2);
				if (Win_X_Pos > 40)
					Win_X_Pos = 40;

				// Update the Window and Sprite Attribute Table base addresses.
				Win_Addr = (VDP_Reg.m5.Pat_Win_Adr & 0x3C) << 10;
				Spr_Addr = (VDP_Reg.m5.Spr_Att_Adr & 0x7E) << 9;
			} else {
				// H32 mode.
				H_Cell = 32;
				H_Win_Shift = 5;
				H_Pix = 256;
				H_Pix_Begin = 32;

				// Check the window horizontal position.
				Win_X_Pos = ((VDP_Reg.m5.Win_H_Pos & 0x1F) * 2);
				if (Win_X_Pos > 32)
					Win_X_Pos = 32;

				// Update the Window and Sprite Attribute Table base addresses.
				Win_Addr = (VDP_Reg.m5.Pat_Win_Adr & 0x3E) << 10;
				Spr_Addr = (VDP_Reg.m5.Spr_Att_Adr & 0x7F) << 9;
			}

			break;

		case 13:
			// H Scroll Table base address.
			H_Scroll_Addr = (val & 0x3F) << 10;
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
			struct Scroll_Size_Tbl_t
			{
				uint8_t H_Scroll_CMul;
				uint8_t H_Scroll_CMask;
				uint8_t V_Scroll_CMask;
				uint8_t reserved;
			};

			static const Scroll_Size_Tbl_t Scroll_Size_Tbl[] =
			{
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
 * Read the H Counter.
 * @return H Counter.
 */
uint8_t Vdp::Read_H_Counter(void)
{
	unsigned int odo_68K = M68K::ReadOdometer();
	odo_68K -= (M68K_Mem::Cycles_M68K - M68K_Mem::CPL_M68K);
	odo_68K &= 0x1FF;

	// H_Counter_Table[][0] == H32.
	// H_Counter_Table[][1] == H40.

	if (isH40())
		return H_Counter_Table[odo_68K][1];
	else
		return H_Counter_Table[odo_68K][0];
}


/**
 * Read the V Counter.
 * @return V Counter.
 */
uint8_t Vdp::Read_V_Counter(void)
{
	unsigned int odo_68K = M68K::ReadOdometer();
	odo_68K -= (M68K_Mem::Cycles_M68K - M68K_Mem::CPL_M68K);
	odo_68K &= 0x1FF;

	unsigned int H_Counter;
	uint8_t bl, bh;		// TODO: Figure out what this actually means.

	if (isH40()) {
		// H40
		H_Counter = H_Counter_Table[odo_68K][0];
		bl = 0xA4;
	} else {
		// H32
		H_Counter = H_Counter_Table[odo_68K][1];
		bl = 0x84;
	}

	bh = ((H_Counter <= 0xE0) ? 1 : 0);
	bl = ((H_Counter >= bl) ? 1 : 0);
	bl &= bh;

	int V_Counter = VDP_Lines.Visible.Current;
	if (V_Counter < 0)
		V_Counter += VDP_Lines.Display.Total;
	V_Counter += (bl ? 1 : 0);

	// TODO: Some of these values are wrong.
	// Rewrite HV handling to match Genesis Plus.

	// V_Counter_Overflow depends on PAL/NTSC status.
	if (Reg_Status.isPal()) {
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
	if (Interlaced == VdpTypes::INTERLACED_MODE_2) {
		// Interlaced mode is enabled.
		uint8_t vc_tmp = (V_Counter & 0xFF);
		vc_tmp = (vc_tmp << 1) | (vc_tmp >> 7);
		return vc_tmp;
	}

	// Interlaced mode is not enabled.
	return (uint8_t)(V_Counter & 0xFF);
}


/**
 * Vdp::Read_Status(): Read the VDP status register.
 * @return VDP status register.
 */
uint16_t Vdp::Read_Status(void)
{
	uint16_t status = Reg_Status.read();
	
	// If the Display is disabled, set the VBlank flag.
	if (VDP_Reg.m5.Set2 & 0x40)
		return status;
	else
		return (status | VdpStatus::VDP_STATUS_VBLANK);
}


/**
 * Read data from the VDP.
 * @return Data.
 */
uint16_t Vdp::Read_Data(void)
{
	// TODO: Test this function.
	// Soleil (crusader of Centry) reads from VRam.
	LOG_MSG(vdp_io, LOG_MSG_LEVEL_DEBUG2,
		"VDP_Ctrl.code == %02X, VDP_Ctrl.access == %04X",
		VDP_Ctrl.code, VDP_Ctrl.access);

	// Clear the VDP control word latch.
	// (It's set when the address is set.)
	VDP_Ctrl.ctrl_latch = false;

	// NOTE: volatile is needed due to an optimization issue caused by
	// -ftree-pre on gcc-4.4.2. (It also breaks on gcc-3.4.5, but that
	// flag doesn't exist on gcc-3.4.5...)
	// An example of the issue can be seen in Soleil (Crusader oF Centry).
	// The onscreen text is partially corrupted when scrolling.
	// TODO: Report this as a bug to the gcc developers.
	volatile uint16_t data;

	// Check the access mode.
	switch (VDP_Ctrl.access) {
		case (VDEST_LOC_VRAM | VDEST_ACC_READ):
			// VRam Read.
			data = VRam.u16[(VDP_Ctrl.Address & 0xFFFF) >> 1];
			break;

		case (VDEST_LOC_CRAM | VDEST_ACC_READ):
			// CRam Read.
			data = m_palette.readCRam_16(VDP_Ctrl.Address & 0x7E);
			break;

		case (VDEST_LOC_VSRAM | VDEST_ACC_READ):
			// VSRam Read.
			// TODO: Mask off high bits? (Only 10/11 bits are present.)
			// TODO: If we do that, the remaining bits should be what's in the FIFO.
			data = VSRam.u16[(VDP_Ctrl.Address & 0x7E) >> 1];
			break;

		default:
			// Invalid read specification.
			return 0;
	}

	VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
	return data;
}


/**
 * Update the DMA state.
 * @return Number of cycles taken from the 68000 for DMA.
 */
unsigned int Vdp::Update_DMA(void)
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
	unsigned int offset = (isH40() ? 2 : 0);

	// Check if we're in VBlank or if the VDP is disabled.
	if (VDP_Lines.Visible.Current < 0 ||
	    VDP_Lines.Visible.Current >= VDP_Lines.Visible.Total ||
	    (!(VDP_Reg.m5.Set2 & 0x40)))
	{
		// In VBlank, or VDP is disabled.
		offset |= 1;
	}

	// Cycles elapsed is based on M68K cycles per line.
	unsigned int cycles = M68K_Mem::CPL_M68K;

	// Get the DMA transfer rate.
	const uint8_t timing = DMA_Timing_Table[DMAT_Type & 3][offset];
	if (DMAT_Length > timing) {
		// DMA is not finished.
		DMAT_Length -= timing;
		if (DMAT_Type & 2) {
			// DMA to CRam or VSRam.
			// Return 0.
			return 0;
		}

		// DMA to VRam.
		// Return the total number of cycles.
		return cycles;
	}

	// DMA is finished. Do some processing.
	const unsigned int len_tmp = DMAT_Length;
	DMAT_Length = 0;

	// Calculate the new cycles value.
	// (NOTE: I have no idea how this formula was created.)
	//cycles = (((cycles << 16) / timing) * len_tmp) >> 16;
	cycles <<= 16;
	cycles /= timing;
	cycles *= len_tmp;
	cycles >>= 16;

	// Clear the DMA Busy flag.
	Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, false);

	if (DMAT_Type & 2) {
		// DMA to CRam or VSRam.
		// Return 0.
		return 0;
	}

	// DMA to VRam.
	// Return the total number of cycles.
	return cycles;
}


/**
 * Write data to the VDP. (8-bit)
 * @param data 8-bit data.
 */
void Vdp::Write_Data_Byte(uint8_t data)
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

	Write_Data_Word(data | (data << 8));
}


/**
 * Perform a DMA Fill operation. (Called from VDP_Write_Data_Word().)
 * @param data 16-bit data.
 */
void Vdp::DMA_Fill(uint16_t data)
{
	// Set the VRam flag.
	MarkVRamDirty();

	// Get the values. (length is in bytes)
	// NOTE: DMA Fill uses *bytes* for length, not words!
	uint16_t address = (VDP_Ctrl.Address & 0xFFFF);
	int length = DMA_Length();
	if (length == 0) {
		// DMA length is 0. Set it to 65,536 words.
		// TODO: This was actually not working in the asm,
		// since I was testing for zero after an or/mov, expecting
		// the mov to set flags. mov doesn't set flags!
		// So I'm not sure if this is right or not.
		length = 65536;
	}

	// Set the DMA Busy flag.
	Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, true);

	// TODO: Although we decrement DMAT_Length correctly based on
	// DMA cycles per line, we fill everything immediately instead
	// of filling at the correct rate.
	// Perhaps this should be combined with DMA_LOOP.
	set_DMA_Length(0);	// Clear the DMA length.
	VDP_Ctrl.DMA = 0;	// Clear the DMA mode.

	// Set DMA type and length.
	DMAT_Type = 0x02;	// DMA Fill.
	DMAT_Length = length;

	// NOTE: DMA FILL seems to treat VRam as little-endian...
	// TODO: Do DMA FILL line-by-line instead of all at once.
	if (!(address & 1)) {
		// Even VRam address.

		// Step 1: Write the VRam data to the current address. (little-endian)
		VRam.u8[address] = (data & 0xFF);
		address = ((address + 1) & 0xFFFF);
		VRam.u8[address] = ((data >> 8) & 0xFF);
		address = ((address - 1 + VDP_Reg.m5.Auto_Inc) & 0xFFFF);

		// Step 2: Write the high byte of the VRam data to the remaining addresses.
		const uint8_t fill_hi = (data >> 8) & 0xFF;
		do {
			VRam.u8[address] = fill_hi;
			address = ((address + VDP_Reg.m5.Auto_Inc) & 0xFFFF);
		} while (--length != 0);
	} else {
		// Odd VRam address.

		// Step 1: Write the VRam data to the previous address. (big-endian)
		address = ((address - 1) & 0xFFFF);
		VRam.u8[address] = ((data >> 8) & 0xFF);
		address = ((address + 1) & 0xFFFF);
		VRam.u8[address] = (data & 0xFF);
		address = ((address + VDP_Reg.m5.Auto_Inc) & 0xFFFF);

		// Step 2: Write the high byte of the VRam data to the remaining addresses.
		const uint8_t fill_hi = (data >> 8) & 0xFF;
		do {
			VRam.u8[address] = fill_hi;
			address = ((address + VDP_Reg.m5.Auto_Inc) & 0xFFFF);
		} while (--length != 0);
	}

	// Save the new address.
	VDP_Ctrl.Address = (address & 0xFFFF);
}


/**
 * Write data to the VDP. (16-bit)
 * @param data 16-bit data.
 */
void Vdp::Write_Data_Word(uint16_t data)
{
	// Clear the VDP control word latch.
	VDP_Ctrl.ctrl_latch = false;

	if (VDP_Ctrl.DMA & 0x04) {
		// DMA Fill operation is in progress.
		DMA_Fill(data);
		return;
	}

	// Check the access mode.
	uint16_t address = VDP_Ctrl.Address;
	switch (VDP_Ctrl.access) {
		case (VDEST_LOC_VRAM | VDEST_ACC_WRITE):
			// VRam Write.
			MarkVRamDirty();
			address &= 0xFFFF;	// VRam is 64 KB. (32 Kwords)
			if (address & 0x0001) {
				// Odd address.
				// VRam writes are only allowed at even addresses.
				// The VDP simply masks the low bit of the address
				// and swaps the high and low bytes before writing.
				address &= ~0x0001;
				data = (data << 8 | data >> 8);
			}

			// Write the word to VRam.
			VRam.u16[address>>1] = data;

			// Increment the address register.
			VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
			break;

		case (VDEST_LOC_CRAM | VDEST_ACC_WRITE):
			// CRam Write.
			// TODO: According to the Genesis Software Manual, writing at
			// odd addresses results in "interesting side effects".
			// Those side effects aren't listed, so we're just going to
			// mask the LSB for now.

			// Write the word to CRam.
			// CRam is 128 bytes. (64 words)
			m_palette.writeCRam_16((address & 0x7E), data);

			// Increment the address register.
			VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
			break;

		case (VDEST_LOC_VSRAM | VDEST_ACC_WRITE):
			// VSRam Write.
			// TODO: The Genesis Software Manual doesn't mention what happens
			// with regards to odd address writes for VSRam.

			// Write the word to VSRam.
			// VSRam is 80 bytes. (40 words)
			// TODO: VSRam is 80 bytes, but we're allowing a maximum of 128 bytes here...
			// TODO: Mask off high bits? (Only 10/11 bits are present.)
			VSRam.u16[(address & 0x7E) >> 1] = data;

			// Increment the address register.
			VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
			break;

		default:
			// Invalid write specification.
			break;
	}
}


/**
 * Vdp::T_DMA_Loop(): Mem-to-DMA loop.
 * @param src_component Source component.
 * @param dest_component Destination component.
 * @param src_address Source address.
 * @param dest_address Destination address.
 * @param length Length.
 */
template<Vdp::DMA_Src_t src_component, Vdp::DMA_Dest_t dest_component>
inline void Vdp::T_DMA_Loop(unsigned int src_address, uint16_t dest_address, int length)
{
	LOG_MSG(vdp_io, LOG_MSG_LEVEL_DEBUG2,
		"<%d, %d> src_address == 0x%06X, dest_address == 0x%04X, length == %d",
		src_component, dest_component, src_address, dest_address, length);

	// Save the DMA length for timing purposes.
	DMAT_Length = length;

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
			src_address -= 2;	// TODO: What is this for?
			src_address &= 0x3FFFE;
			break;

		case DMA_SRC_WORD_RAM_1M_0:
		case DMA_SRC_WORD_RAM_1M_1:
		case DMA_SRC_WORD_RAM_CELL_1M_0:
		case DMA_SRC_WORD_RAM_CELL_1M_1:
			src_address -= 2;	// TODO: What is this for?
			src_address &= 0x1FFFE;
			break;
#endif

		default:	// to make gcc shut up
			break;
	}

	// Determine if any flags should be set.
	switch (dest_component) {
		case DMA_DEST_VRAM:
			MarkVRamDirty();
			DMAT_Type = 0;
			break;

		case DMA_DEST_CRAM:
			DMAT_Type = 1;
			break;

		case DMA_DEST_VSRAM:
			DMAT_Type = 1;
			break;

		default:	// to make gcc shut up
			break;
	}

	VDP_Ctrl.DMA = 0;

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
			case DMA_SRC_ROM:
				w = M68K_Mem::Rom_Data.u16[src_word_address | src_base_address];
				break;
			
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
		switch (dest_component) {
			case DMA_DEST_VRAM:
				if (dest_address & 1)
					w = (w << 8 | w >> 8);
				VRam.u16[dest_address >> 1] = w;
				break;

			case DMA_DEST_CRAM:
				m_palette.writeCRam_16(dest_address, w);
				break;

			case DMA_DEST_VSRAM:
				// TODO: Mask off high bits? (Only 10/11 bits are present.)
				VSRam.u16[dest_address >> 1] = w;
				break;

			default:	// to make gcc shut up
				break;
		}

		dest_address = ((dest_address + VDP_Reg.m5.Auto_Inc) & 0xFFFF);

		// Check for CRam or VSRam destination overflow.
		if (dest_component == DMA_DEST_CRAM ||
		    dest_component == DMA_DEST_VSRAM)
		{
			if (dest_address >= 0x80) {
				// CRam/VSRam overflow!
				length--;	// for this word
				break;
			}
		}
	} while (--length != 0);

	// Save the new destination address.
	VDP_Ctrl.Address = dest_address;

	// If any bytes weren't copied, subtract it from the saved length.
	DMAT_Length -= length;
	if (DMAT_Length <= 0) {
		// No DMA left!
		Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, false);
		return;
	}

	// Save the new source address.
	// NOTE: The new DMA_Address is the wrapped version.
	// The old asm code saved the unwrapped version.
	// Ergo, it simply added length to DMA_Address.
	inc_DMA_Src_Adr(DMAT_Length);

	// Update DMA.
	Update_DMA();

	// NOTE: main68k_releaseCycles() takes no parameters,
	// but the actual function subtracts eax from __io_cycle_counter.
	// eax was equal to DMAT_Length.
	M68K::ReleaseCycles(DMAT_Length);
}


/**
 * Vdp::Write_Ctrl(): Write a control word to the VDP.
 * @param data Control word.
 */
void Vdp::Write_Ctrl(uint16_t data)
{
	// TODO: Check endianness with regards to the control words. (Wordswapping!)

	// Check if this is the first or second control word.
	if (!VDP_Ctrl.ctrl_latch) {
		// First control word.
		// Check if this is a register write
		if ((data & 0xC000) == 0x8000) {
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
			VDP_Ctrl.code = 0;		// Reset the access code register.
			VDP_Ctrl.access = CD_Table[VDP_Ctrl.code];
			VDP_Ctrl.Address = 0x0000;	// Reset the address counter.

			const int reg = (data >> 8) & 0x1F;
			setReg(reg, (data & 0xFF));
			return;
		}

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
		VDP_Ctrl.data[0] = data;
		VDP_Ctrl.ctrl_latch = true;	// Latch the first control word.

		// Determine the VDP address.
		VDP_Ctrl.Address = (data & 0x3FFF);
		VDP_Ctrl.Address |= ((VDP_Ctrl.data[1] & 0x3) << 14);

		// Update the VDP access code register.
		VDP_Ctrl.code &= ~0x03;
		VDP_Ctrl.code |= ((data >> 14) & 0x03);
		VDP_Ctrl.access = (CD_Table[VDP_Ctrl.code] & 0xFF);
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
	 */
	VDP_Ctrl.data[1] = data;
	VDP_Ctrl.ctrl_latch = false;	// Clear the control word latch.

	// Determine the VDP address.
	VDP_Ctrl.Address = (VDP_Ctrl.data[0] & 0x3FFF);
	VDP_Ctrl.Address |= ((data & 3) << 14);

	// Update the VDP access code register.
	VDP_Ctrl.code &= ~0x3C;
	VDP_Ctrl.code |= ((data >> 2) & 0x3C);
	uint16_t CD = CD_Table[VDP_Ctrl.code];
	VDP_Ctrl.access = (CD & 0xFF);

	// High byte of CD is the DMA access mode.
	if (!(CD & 0xFF00)) {
		// No DMA is needed.
		return;
	}

	/** Perform a DMA operation. **/

	// Check if DMA is enabled.
	if (!(VDP_Reg.m5.Set2 & 0x10)) {
		// DMA is disabled.
		VDP_Ctrl.DMA = 0;
		return;
	}

	// Check for DMA FILL.
	if ((CD & VDEST_DMA_FILL) && (VDP_Ctrl.DMA_Mode == 0x80)) {
		// DMA FILL.
		VDP_Ctrl.DMA = ((CD >> 8) & 0xFF);
		return;
	}

	// DMA access mode is the high byte in the CD_Table[] word.
	CD >>= 8;

	// Determine the DMA destination.
	DMA_Dest_t dest_component = (DMA_Dest_t)(CD & 0x03);	// 0 == invalid; 1 == VRam; 2 == CRam; 3 == VSRam

	// Get the DMA addresses.
	uint32_t src_address = DMA_Src_Adr();			// Src Address / 2
	uint16_t dest_address = (VDP_Ctrl.Address & 0xFFFF);	// Dest Address

	// Check for CRam or VSRam destination overflow.
	if (dest_component == DMA_DEST_CRAM ||
	    dest_component == DMA_DEST_VSRAM)
	{
		// FIXME: Frogger (U) does DMA writes to CRAM $8000.
		// Obviously this won't work if we're aborting on dest_address >= 0x80.
		// We'll mask it with 0xFF for now.
		// TODO: Figure out what the actual mask should be later.
		dest_address &= 0xFF;
		if (dest_address >= 0x80) {
			// CRam/VSRam overflow! Don't do anything.
			VDP_Ctrl.DMA = 0;
			return;
		}
	}

	int length = DMA_Length();
	if (length == 0) {
		// DMA length is zero.
		if (VdpEmuOptions.zeroLengthDMA) {
			// Zero-Length DMA transfers are enabled.
			// Ignore this request.
			VDP_Ctrl.DMA = 0;
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
		src_address &= 0xFFFF;
		Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, true);	// Set the DMA BUSY bit.
		set_DMA_Length(0);
		DMAT_Length = length;
		DMAT_Type = 0x3;
		MarkVRamDirty();

		// TODO: Is this correct with regards to endianness?
		// TODO: Do DMA COPY line-by-line instead of all at once.
		do {
			VRam.u8[dest_address] = VRam.u8[src_address];

			// Increment the addresses.
			src_address = ((src_address + 1) & 0xFFFF);
			dest_address = ((dest_address + VDP_Reg.m5.Auto_Inc) & 0xFFFF);
		} while (--length != 0);

		// Save the new addresses.
		// NOTE: DMA COPY uses bytes, not words.
		inc_DMA_Src_Adr(DMAT_Length);	// TODO: Should DMA_Src_Adr_H be cleared?
		VDP_Ctrl.Address = dest_address;
		return;
	}

	if (VDP_Ctrl.DMA_Mode & 0x80) {
		// TODO: What does this mean?
		VDP_Ctrl.DMA = 0;
		return;
	}

	// Multiply the source address by two to get the real source address.
	src_address *= 2;

	// Determine the source component.
	DMA_Src_t src_component = DMA_SRC_ROM;	// TODO: Determine a better default.
	int WRam_Mode;
	if (src_address < M68K_Mem::Rom_Size) {
		// Main ROM.
		src_component = DMA_SRC_ROM;
	} else if (!SysStatus.SegaCD) {
		// SegaCD is not started. Assume M68K RAM.
		// TODO: This includes invalid addresses!
		src_component = DMA_SRC_M68K_RAM;
	} else {
		// SegaCD is started.
		if (src_address >= 0x240000) {
			// Assume M68K RAM.
			// TODO: This includes invalid addresses!
			src_component = DMA_SRC_M68K_RAM;
		} else if (src_address < 0x40000) {
			// Program RAM.
			src_component = DMA_SRC_PRG_RAM;
		} else {
			// Word RAM. Check the Word RAM state to determine the mode.
			// TODO: Determine how this works.
			// TODO: Port to LibGens.
#if 0
			WRam_Mode = (Ram_Word_State & 0x03) + 3;
			if (WRam_Mode < 5 || src_address < 0x220000) {
				src_component = (DMA_Src_t)WRam_Mode;
			} else {
				src_component = (DMA_Src_t)(WRam_Mode + 2);
			}
#endif
		}
	}

	// Set the DMA BUSY bit.
	Reg_Status.setBit(VdpStatus::VDP_STATUS_DMA, true);

	switch (DMA_TYPE(src_component, dest_component)) {
		case DMA_TYPE(DMA_SRC_ROM, DMA_DEST_VRAM):
			T_DMA_Loop<DMA_SRC_ROM, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_ROM, DMA_DEST_CRAM):
			T_DMA_Loop<DMA_SRC_ROM, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_ROM, DMA_DEST_VSRAM):
			T_DMA_Loop<DMA_SRC_ROM, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_M68K_RAM, DMA_DEST_VRAM):
			T_DMA_Loop<DMA_SRC_M68K_RAM, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_M68K_RAM, DMA_DEST_CRAM):
			T_DMA_Loop<DMA_SRC_M68K_RAM, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_M68K_RAM, DMA_DEST_VSRAM):
			T_DMA_Loop<DMA_SRC_M68K_RAM, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_PRG_RAM, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_PRG_RAM, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_PRG_RAM, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_PRG_RAM, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_PRG_RAM, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_PRG_RAM, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_2M, DMA_DEST_VRAM):
			T_DMA_Loop<DMA_SRC_WORD_RAM_2M, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_2M, DMA_DEST_CRAM):
			T_DMA_Loop<DMA_SRC_WORD_RAM_2M, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_2M, DMA_DEST_VSRAM):
			T_DMA_Loop<DMA_SRC_WORD_RAM_2M, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_0, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_0, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_0, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_0, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_0, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_0, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_1, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_1, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_1, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_1, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_1, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_1, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;

		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;

		default:
			// Invalid DMA mode.
			VDP_Ctrl.DMA = 0;
			break;
	}

	return;
}

}
