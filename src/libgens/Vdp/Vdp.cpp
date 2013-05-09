/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Vdp.cpp: VDP class: General functions.                                  *
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

// C includes. (C++ namespace)
#include <cstring>

// LOG_MSG() subsystem.
#include "macros/log_msg.h"

// ARRAY_SIZE(x)
#include "macros/common.h"

// ZOMG
#include "libzomg/Zomg.hpp"

namespace LibGens
{
	
// VdpRend_Err private class.
#include "VdpRend_Err_p.hpp"

/** Static member initialization. (VDP global items) **/
// TODO: Make these non-static?
VdpTypes::VdpEmuOptions_t Vdp::VdpEmuOptions =
{
	VdpTypes::INTREND_FLICKER,	// intRendMode
	true,				// borderColorEmulation
	true,				// ntscV30Rolling
	false,				// zeroLengthDMA
	true,				// spriteLimits
	true,				// vscrollBug
};


/**
 * Initialize the VDP subsystem.
 * @param fb Existing MdFb to use. (If nullptr, allocate a new MdFb.)
 */
Vdp::Vdp(MdFb *fb)
	: MD_Screen(fb ? fb->ref() : new MdFb())
	, d_err(new VdpRend_Err_Private(this))
{
	// Initialize the Horizontal Counter table.
	unsigned int hc_val;
	for (unsigned int hc = 0; hc < 512; hc++)
	{
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
	VDP_Mode = 0;
	
	// Initialize system status.
	// TODO: Move SysStatus somewhere else?
	SysStatus.data = 0;
	
	// Initialize the VDP rendering subsystem.
	rend_init();
	
	// Reset the VDP.
	reset();
}


/**
 * Shut down the VDP subsystem.
 */
Vdp::~Vdp(void)
{
	// Shut down the VDP rendering subsystem.
	rend_end();
	
	// Unreference the framebuffer.
	MD_Screen->unref();
}


/**
 * Reset the VDP.
 */
void Vdp::reset(void)
{
	// Reset the VDP rendering arrays.
	rend_reset();

	// Clear VRam and VSRam.
	memset(&VRam, 0x00, sizeof(VRam));
	memset(&VSRam, 0x00, sizeof(VSRam));

	// Reset the palette. (Includes CRam.)
	m_palette.reset();

	/**
	 * VDP registers.
	 * Default register values: (Mode 5)
	 * - 0x01 (Mode1):   0x04 (H_Int off, Mode 5 [MD])
	 * - 0x0A (H_Int):   0xFF (disabled).
	 * - 0x0C (Mode4):   0x81 (H40, S/H off, no interlace)
	 * - 0x0F (AutoInc): 0x02 (auto-increment by 2 on memory access)
	 * All other registers are set to 0x00 by default.
	 */
	static const uint8_t vdp_reg_init_m5[24] =
	{
		0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xFF, 0x00, 0x81, 0x00, 0x00, 0x02,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	for (int i = 0; i < ARRAY_SIZE(vdp_reg_init_m5); i++) {
		setReg(i, vdp_reg_init_m5[i]);
	}

	// Reset the DMA variables.
	DMAT_Length = 0;
	DMAT_Type = 0;

	// VDP status register.
	// (Maintain the status of the PAL/NTSC bit.)
	const bool isPal = Reg_Status.isPal();
	Reg_Status.reset(isPal);

	// Other variables.
	VDP_Int = 0;

	// VDP control struct.
	VDP_Ctrl.data[0] = 0;
	VDP_Ctrl.data[1] = 0;
	VDP_Ctrl.Access = 0;	// TODO: Initialize to (VDEST_LOC_VRAM | VDEST_ACC_READ)?
	VDP_Ctrl.Address = 0;
	VDP_Ctrl.DMA_Mode = 0;
	VDP_Ctrl.DMA = 0;
	VDP_Ctrl.ctrl_latch = false;

	// Set the VDP update flags.
	MarkVRamDirty();

	// Initialize the Horizontal Interrupt counter.
	HInt_Counter = VDP_Reg.m5.H_Int;
}


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
	zomg->saveVdpReg(VDP_Reg.reg, 24);

	// Save the internal registers.
	Zomg_VdpCtrl_16_t ctrl_reg;
	ctrl_reg.header = ZOMG_VDPCTRL_16_HEADER;
	ctrl_reg.ctrl_word[0] = VDP_Ctrl.data[0];
	ctrl_reg.ctrl_word[1] = VDP_Ctrl.data[1];
	ctrl_reg.ctrl_latch = !!VDP_Ctrl.ctrl_latch;
	ctrl_reg.access = VDP_Ctrl.Access;
	ctrl_reg.address = VDP_Ctrl.Address;
	ctrl_reg.status = Reg_Status.read_raw();

	// TODO: Implement the FIFO.
	memset(ctrl_reg.data_fifo, 0x00, sizeof(ctrl_reg.data_fifo));
	ctrl_reg.data_fifo_count = 0;

	// Make sure reserved fields are zero.
	ctrl_reg.reserved2 = 0;

	zomg->saveVdpCtrl_16(&ctrl_reg);

	// TODO: Save DMA status.
	ctrl_reg.dma_access = ((VDP_Ctrl.Access >> 8) & 0xFF);

	// Save VRam.
	zomg->saveVRam(VRam.u16, sizeof(VRam.u16), ZOMG_BYTEORDER_16H);

	// Save CRam.
	Zomg_CRam_t cram;
	m_palette.zomgSaveCRam(&cram);
	zomg->saveCRam(&cram, ZOMG_BYTEORDER_16H);

	// Save VSRam. (MD only)
	zomg->saveMD_VSRam(VSRam.u16, sizeof(VSRam.u16), ZOMG_BYTEORDER_16H);
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
	// Writing to register 23 changes the DMA status.
	for (int i = 23; i >= 0; i--) {
		setReg(i, vdp_reg[i]);
	}

	// Load the internal registers.
	// NOTE: LibZomg verifies that the header is correct.
	Zomg_VdpCtrl_16_t ctrl_reg;
	int ret = zomg->loadVdpCtrl_16(&ctrl_reg);
	if (ret > 0) {
		VDP_Ctrl.data[0] = ctrl_reg.ctrl_word[0];
		VDP_Ctrl.data[1] = ctrl_reg.ctrl_word[1];
		VDP_Ctrl.ctrl_latch = !!ctrl_reg.ctrl_latch;
		VDP_Ctrl.Access = (ctrl_reg.access | (ctrl_reg.dma_access << 8));
		VDP_Ctrl.Address = ctrl_reg.address;
		Reg_Status.write_raw(ctrl_reg.status);

		// TODO: Implement the FIFO.
	} else {
		// TODO: Handle this error...
		LOG_MSG(vdp_m5, LOG_MSG_LEVEL_WARNING,
			"WARNING: zomg->loadVdpCtrl_16() failed: error %d\n", ret);
	}

	// TODO: Load DMA status.

	// Load VRam.
	zomg->loadVRam(VRam.u16, sizeof(VRam.u16), ZOMG_BYTEORDER_16H);
	MarkVRamDirty();

	// Load CRam.
	Zomg_CRam_t cram;
	zomg->loadCRam(&cram, ZOMG_BYTEORDER_16H);
	m_palette.zomgRestoreCRam(&cram);

	// Load VSRam. (MD only)
	zomg->loadMD_VSRam(VSRam.u16, sizeof(VSRam.u16), ZOMG_BYTEORDER_16H);
}

}
