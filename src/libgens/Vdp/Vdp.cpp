/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Vdp.cpp: VDP class: General functions.                                  *
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

#include "macros/log_msg.h"
#include "macros/common.h"

// C includes. (C++ namespace)
#include <cstring>

// ZOMG
#include "libzomg/Zomg.hpp"

// VDP includes.
#include "VdpPalette.hpp"

// Private classes.
#include "Vdp_p.hpp"
#include "VdpRend_Err_p.hpp"

namespace LibGens {

/** VdpPrivate **/

// Default VDP emulation options.
const VdpTypes::VdpEmuOptions_t VdpPrivate::def_vdpEmuOptions = {
	VdpTypes::INTREND_FLICKER,	// intRendMode
	true,				// borderColorEmulation
	true,				// ntscV30Rolling
	true,				// spriteLimits
	// The following options should not be changed
	// unless the user knows what they're doing!
	false,				// zeroLengthDMA
	true,				// vscrollBug
	false,				// updatePaletteInVBlankOnly
};

VdpPrivate::VdpPrivate(Vdp *q)
	: q(q)
	, VRam_Mask(0xFFFF)	// Always ensure this mask is valid.
	, d_err(new VdpRend_Err_Private(q))
{
	// TODO: Initialize all private variables.

	// Initialize the Horizontal Counter table.
	unsigned int hc_val;
	for (int hc = 0; hc < 512; hc++) {
		// H32
		hc_val = ((hc * 170) / 488) - 0x18;
		H_Counter_Table[hc][0] = (uint8_t)hc_val;

		// H40
		hc_val = ((hc * 205) / 488) - 0x1C;
		H_Counter_Table[hc][1] = (uint8_t)hc_val;
	}

	// Clear VDP_Reg before initializing the VDP.
	// Valgrind complains if we don't do this.
	memset(&VDP_Reg.reg[0], 0x00, sizeof(VDP_Reg.reg));
	VDP_Mode = (VDP_Mode_t)0;
}

VdpPrivate::~VdpPrivate()
{
	delete d_err;
}

/** Vdp **/

/**
 * Initialize the VDP subsystem.
 * @param fb Existing MdFb to use. (If nullptr, allocate a new MdFb.)
 */
Vdp::Vdp(MdFb *fb)
	: d(new VdpPrivate(this))
	, options(VdpPrivate::def_vdpEmuOptions)
	, DMAT_Length(0)
	, MD_Screen(fb ? fb->ref() : new MdFb())
{
	// Initialize system status.
	// TODO: Move SysStatus somewhere else?
	SysStatus.data = 0;

	// Initialize the VDP rendering subsystem.
	d->rend_init();

	// Reset the VDP.
	reset();
}

/**
 * Shut down the VDP subsystem.
 */
Vdp::~Vdp(void)
{
	// Shut down the VDP rendering subsystem.
	d->rend_end();

	// Unreference the framebuffer.
	MD_Screen->unref();
}

/**
 * Reset the VDP.
 */
void Vdp::reset(void)
{
	// Reset the VDP rendering arrays.
	d->rend_reset();

	// Clear VRam and VSRam.
	memset(&d->VRam, 0x00, sizeof(d->VRam));
	memset(&d->VSRam, 0x00, sizeof(d->VSRam));

	// Reset the palette. (Includes CRam.)
	d->palette.reset();

	/**
	 * VDP registers.
	 * Default register values: (Mode 5)
	 * - 0x01 (Mode1):   0x04 (H_Int off, Mode 5 [MD])
	 * - 0x0A (H_Int):   0xFF (disabled).
	 * - 0x0C (Mode4):   0x81 (H40, S/H off, no interlace)
	 * - 0x0F (AutoInc): 0x02 (auto-increment by 2 on memory access)
	 * All other registers are set to 0x00 by default.
	 */
	static const uint8_t vdp_reg_init_m5[24] = {
		0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xFF, 0x00, 0x81, 0x00, 0x00, 0x02,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	for (int i = 0; i < ARRAY_SIZE(vdp_reg_init_m5); i++) {
		d->setReg(i, vdp_reg_init_m5[i]);
	}

	// Reset the DMA variables.
	DMAT_Length = 0;
	d->DMAT_Type = VdpPrivate::DMAT_MEM_TO_VRAM;

	// VDP status register.
	// (Maintain the status of the PAL/NTSC bit.)
	const bool isPal = d->Reg_Status.isPal();
	d->Reg_Status.reset(isPal);

	// Reset more stuff.
	d->VDP_Int = 0;		// No pending interrupts.
	d->VDP_Ctrl.reset();	// VDP control struct.

	// Initialize the Horizontal Interrupt counter.
	d->HInt_Counter = d->VDP_Reg.m5.H_Int;
}

// PAL/NTSC.
bool Vdp::isPal(void) const
	{ return d->Reg_Status.isPal(); }
bool Vdp::isNtsc(void) const
	{ return d->Reg_Status.isNtsc(); }
void Vdp::setPal(void)
	{ d->Reg_Status.setBit(VdpStatus::VDP_STATUS_PAL, true); }
void Vdp::setNtsc(void)
	{ d->Reg_Status.setBit(VdpStatus::VDP_STATUS_PAL, false); }
void Vdp::setVideoMode(bool videoMode)
	{ d->Reg_Status.setBit(VdpStatus::VDP_STATUS_PAL, videoMode); }

// Display resolution.
// Required by the frontend for proper rendering.
// Also used by VdpRend_Err.
int Vdp::getHPix(void) const
	{ return d->H_Pix; }
int Vdp::getHPixBegin(void) const
	{ return d->H_Pix_Begin; }
int Vdp::getVPix(void) const
	{ return VDP_Lines.totalVisibleLines; }

/**
 * Update VDP_Lines based on CPU and VDP mode settings.
 * @param resetCurrent If true, reset VDP_Lines.Display.Current and VDP_Lines.Visible.Current.
 */
void Vdp::updateVdpLines(bool resetCurrent)
{
	// Arrays of values.
	// Indexes: 0 == 192 lines; 1 == 224 lines; 2 == 240 lines.
	static const int VisLines_Total[3] = {192, 224, 240};
	static const int VisLines_Border_Size[3] = {24, 8, 0};
	//static const int VisLines_Current_NTSC[3] = {-40, -24, 0};
	//static const int VisLines_Current_PAL[3] = {-67+1, -51+1, -43+1};

	// Initialize VDP_Lines.Display.
	// TODO: 312 or 313 for PAL?
	VDP_Lines.totalDisplayLines = (d->Reg_Status.isPal() ? 312 : 262);

	// Line offset.
	int LineOffset;

	// Check the current video mode.
	// NOTE: Unlike Gens/GS, we don't check if a ROM is loaded because
	// the VDP code isn't used at all in Gens/GS II during "idle".
	if (d->VDP_Mode & VdpPrivate::VDP_MODE_M5) {
		// Mode 5. Must be either 224 lines or 240 lines.
		if (d->VDP_Mode & VdpPrivate::VDP_MODE_M3)
			LineOffset = 2; // 240 lines.
		else
			LineOffset = 1; // 224 lines.
	} else {
		// Mode 4 or TMS9918 mode.
		// Mode 4 may be 192 lines, 224 lines, or 240 lines.
		// Modes 0-3 may only be 192 lines.
		// TODO: If emulating SMS1, disable 224-line and 240-line modes.
		switch (d->VDP_Mode) {
			case VdpPrivate::VDP_MODE_M4_224:
				// Mode 4: 224 lines.
				LineOffset = 1;
				break;
			case VdpPrivate::VDP_MODE_M4_240:
				// Mode 4: 240 lines.
				LineOffset = 2;
				break;
			default:
				// Modes 0-4: 192 lines.
				LineOffset = 0;
				break;
		}
	}

	VDP_Lines.totalVisibleLines = VisLines_Total[LineOffset];
	VDP_Lines.Border.borderSize = VisLines_Border_Size[LineOffset];

	// Calculate border parameters.
	if (VDP_Lines.Border.borderSize > 0) {
		VDP_Lines.Border.borderStartBottom = VDP_Lines.totalVisibleLines;
		VDP_Lines.Border.borderEndBottom = VDP_Lines.Border.borderStartBottom + VDP_Lines.Border.borderSize - 1;

		VDP_Lines.Border.borderEndTop = VDP_Lines.totalDisplayLines - 1;
		VDP_Lines.Border.borderStartTop = VDP_Lines.Border.borderEndTop - VDP_Lines.Border.borderSize + 1;
	} else {
		// No border.
		VDP_Lines.Border.borderStartBottom = -1;
		VDP_Lines.Border.borderEndBottom = -1;
		VDP_Lines.Border.borderStartTop = -1;
		VDP_Lines.Border.borderEndTop = -1;
	}

	if (resetCurrent) {
		// Reset VDP_Lines.currentLine.
		// NOTE: VDP starts at visible line 0.
		VDP_Lines.currentLine = 0;
	}

	// Check interlaced mode.
	d->Interlaced = (VdpTypes::Interlaced_t)
			(((d->VDP_Reg.m5.Set4 & 0x02) >> 1) |	// LSM0
			 ((d->VDP_Reg.m5.Set4 & 0x04) >> 1));	// LSM1
}

/**
 * Check if VBlank is allowed in NTSC V30 mode.
 */
void Vdp::Check_NTSC_V30_VBlank(void)
{
	// TODO: Only do this in Mode 5, and maybe Mode 4 if SMS2 is in use.
	if (d->Reg_Status.isPal() || !(d->VDP_Reg.m5.Set2 & 0x08)) {
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

	if (options.ntscV30Rolling) {
		VDP_Lines.NTSC_V30.Offset += 11;	// TODO: Figure out a good offset increment.
		VDP_Lines.NTSC_V30.Offset %= 240;	// Prevent overflow.
	} else {
		// Rolling is disabled.
		VDP_Lines.NTSC_V30.Offset = 0;
	}
}

/**
 * Start of frame.
 * This updates the "Interlaced" flag and HINT counter,
 * and clears the VBLANK flag.
 *
 * "Interlaced" flag behavior: (VDP_STATUS_ODD)
 * - If interlaced mode is not set, the flag is cleared.
 * - If interlaced mode is set, the flag is toggled.
 * TODO: Move to VdpIo.cpp?
 */
void Vdp::startFrame(void)
{
	// Update the "Interlaced" flag.
	if (d->VDP_Reg.m5.Set4 & 0x06)
		d->Reg_Status.toggleBit(VdpStatus::VDP_STATUS_ODD);
	else
		d->Reg_Status.setBit(VdpStatus::VDP_STATUS_ODD, false);

	d->HInt_Counter = d->VDP_Reg.m5.H_Int;
	d->Reg_Status.setBit(VdpStatus::VDP_STATUS_VBLANK, false);
}

/**
 * Set a bit in the status register.
 * Wrapper for VdpStatus::setBit().
 * Needed because the emulation loops toggle HBLANK/VBLANK.
 * @param bit Status bit to set.
 * @param value New value.
 * TODO: Move to VdpIo.cpp?
 */
void Vdp::setStatusBit(VdpStatus::StatusBits bit, bool value)
{
	d->Reg_Status.setBit(bit, value);
}

/**
 * Decrement the HINT counter.
 * If it goes below 0, an HBLANK interrupt will occur.
 * @param reload If true, reload the HINT counter afterwards.
 * TODO: Move to VdpIo.cpp?
 */
void Vdp::decrementHIntCounter(bool reload)
{
	if (--d->HInt_Counter < 0) {
		updateIRQLine(0x4);
		if (reload) {
			d->HInt_Counter = d->VDP_Reg.m5.H_Int;
		}
	}
}

/** ZOMG!!1! **/

/**
 * Save the VDP state. (MD mode)
 * @param zomg ZOMG savestate object to save to.
 */
void Vdp::zomgSaveMD(LibZomg::Zomg *zomg) const
{
	// NOTE: This is MD only.
	// TODO: Assert if called when not emulating MD VDP.
	// TODO: Error handling.

	// Save the user-accessible VDP registers.
	// TODO: Move "24" to a const somewhere.
	zomg->saveVdpReg(d->VDP_Reg.reg, 24);

	// Save the internal registers.
	Zomg_VdpCtrl_16_t ctrl_reg;
	ctrl_reg.header = ZOMG_VDPCTRL_16_HEADER;
	ctrl_reg.ctrl_latch = !!(d->VDP_Ctrl.ctrl_latch);
	ctrl_reg.code = d->VDP_Ctrl.code;
	ctrl_reg.address = d->VDP_Ctrl.address;
	ctrl_reg.status = d->Reg_Status.read_raw();

	// TODO: Implement the FIFO.
	memset(ctrl_reg.data_fifo, 0x00, sizeof(ctrl_reg.data_fifo));
	ctrl_reg.data_fifo_count = 0;

	// Make sure reserved fields are zero.
	ctrl_reg.reserved2 = 0;

	zomg->saveVdpCtrl_16(&ctrl_reg);

	// TODO: Save DMA status.

	// Save VRam.
	zomg->saveVRam(d->VRam.u16, sizeof(d->VRam.u16), ZOMG_BYTEORDER_16H);

	// Save CRam.
	Zomg_CRam_t cram;
	d->palette.zomgSaveCRam(&cram);
	zomg->saveCRam(&cram, ZOMG_BYTEORDER_16H);

	// Save VSRam. (MD only)
	zomg->saveMD_VSRam(d->VSRam.u16, sizeof(d->VSRam.u16), ZOMG_BYTEORDER_16H);

	// Save VDP SAT. (MD only)
	// NOTE: VDP SAT only has 80 entries.
	// SprAttrTbl_m5 is 128 entries to prevent overflow.
	zomg->saveMD_VDP_SAT(d->SprAttrTbl_m5.w, d->SprAttrTbl_sz, ZOMG_BYTEORDER_16H);
}


/**
 * Restore the VDP state. (MD mode)
 * @param zomg ZOMG savestate object to restore from.
 */
void Vdp::zomgRestoreMD(LibZomg::Zomg *zomg)
{
	// NOTE: This is MD only.
	// TODO: Assert if called when not emulating MD VDP.
	// TODO: Error handling.

	// Load the user-accessible VDP registers.
	// TODO: Move "24" to a const somewhere.
	uint8_t vdp_reg[24];
	zomg->loadVdpReg(vdp_reg, 24);

	// TODO: On MD, load the DMA information from the savestate.
	// (Calculate DMAT_Type.)
	// Writing to register 23 changes the DMA status.
	for (int i = 23; i >= 0; i--) {
		d->setReg(i, vdp_reg[i]);
	}

	// Load the internal registers.
	// NOTE: LibZomg verifies that the header is correct.
	Zomg_VdpCtrl_16_t ctrl_reg;
	int ret = zomg->loadVdpCtrl_16(&ctrl_reg);
	if (ret > 0) {
		d->VDP_Ctrl.ctrl_latch = !!ctrl_reg.ctrl_latch;
		d->VDP_Ctrl.code = ctrl_reg.code;
		d->VDP_Ctrl.address = ctrl_reg.address;

		// TODO: Do we want to load the NTSC/PAL bit from the savestate?
		uint16_t status = d->Reg_Status.read_raw() & VdpStatus::VDP_STATUS_PAL;
		status |= ctrl_reg.status & ~VdpStatus::VDP_STATUS_PAL;
		d->Reg_Status.write_raw(status);

		// TODO: Implement the FIFO.
	} else {
		// TODO: Handle this error...
		LOG_MSG(vdp_m5, LOG_MSG_LEVEL_WARNING,
			"WARNING: zomg->loadVdpCtrl_16() failed: error %d\n", ret);
	}

	// Load VRam.
	zomg->loadVRam(d->VRam.u16, sizeof(d->VRam.u16), ZOMG_BYTEORDER_16H);

	// Load CRam.
	Zomg_CRam_t cram;
	zomg->loadCRam(&cram, ZOMG_BYTEORDER_16H);
	d->palette.zomgRestoreCRam(&cram);

	// Load VSRam. (MD only)
	// FIXME: 80 bytes normally; 128 bytes for Genesis 3.
	zomg->loadMD_VSRam(d->VSRam.u16, sizeof(d->VSRam.u16), ZOMG_BYTEORDER_16H);

	// Load VDP SAT. (MD only)
	// NOTE: VDP SAT only has 80 entries.
	// SprAttrTbl_m5 is 128 entries to prevent overflow.
	ret = zomg->loadMD_VDP_SAT(d->SprAttrTbl_m5.w, d->SprAttrTbl_sz, ZOMG_BYTEORDER_16H);
	if (ret <= 0) {
		// Savestate doesn't have an SAT.
		// Create it by copying from VRAM.
		// TODO: If ret < d->SprAttrTbl_sz?
		memcpy(d->SprAttrTbl_m5.w, d->Spr_Tbl_Addr_PtrM5(0), d->SprAttrTbl_sz);
	}

	// Clear the sprite dot overflow flag.
	d->sprDotOverflow = false;

	// Re-cache the current and next lines.
	// TODO: Only if display is enabled.
	// TODO: Is this necessary?
	unsigned int sovr;
	int line = VDP_Lines.currentLine;
	if (line >= VDP_Lines.totalDisplayLines)
		line -= VDP_Lines.totalDisplayLines;
	if (d->Interlaced == VdpTypes::INTERLACED_MODE_2) {
		d->T_Update_Sprite_Line_Cache<true>(line - 1);
		sovr = d->T_Update_Sprite_Line_Cache<true>(line);
	} else {
		d->T_Update_Sprite_Line_Cache<false>(line - 1);
		sovr = d->T_Update_Sprite_Line_Cache<false>(line);
	}

	if (sovr) {
		// Sprite overflow!
		d->Reg_Status.setBit(VdpStatus::VDP_STATUS_SOVR, true);
	}
}

}
