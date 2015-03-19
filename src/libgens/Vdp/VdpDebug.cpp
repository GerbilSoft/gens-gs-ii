/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpDebug.cpp: VDP class: Debugging functions.                           *
 * For use by MDP plugins and test suites.                                 *
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
#include "Vdp_p.hpp"

// C includes. (C++ namespace)
#include <stdlib.h>

namespace LibGens {

// TODO: Use MDP error codes.

/**
 * Get a VDP register value.
 * @param reg_num Register number. (NOT MASKED for M4 in MD)
 * @param out Buffer for register value.
 * @return MDP error code.
 */
int Vdp::dbg_getReg(int reg_num, uint8_t *out) const
{
	// TODO: M4 on SMS.
	if (reg_num < 0 || reg_num > 23)
		return -1;
	*out = d->VDP_Reg.reg[reg_num];
	return 0;
}

/**
 * Set a VDP register value.
 * @param reg_num Register number. (NOT MASKED for M4 in MD)
 * @param val Register value.
 * @return MDP error code.
 */
int Vdp::dbg_setReg(int reg_num, uint8_t val)
{
	// TODO: M4 on SMS.
	// TODO: Don't mask the write if it's M4 on MD.
	if (reg_num < 0 || reg_num > 23)
		return -1;
	d->setReg(reg_num, val);
	return 0;
}

/**
 * Get the VDP code register.
 * @param CD Buffer for register value.
 * @return MDP error code.
 */
int Vdp::dbg_getCode(uint8_t *CD) const
{
	*CD = d->VDP_Ctrl.code;
	return 0;
}

/**
 * Set the VDP code register.
 * @param CD Register value.
 * @return MDP error code.
 */
int Vdp::dbg_setCode(uint8_t CD)
{
	// TODO: Additional processing?
	d->VDP_Ctrl.code = CD;
	return 0;
}

/**
 * Get the VDP address register.
 * @param address Buffer for register value.
 * @return MDP error code.
 */
int Vdp::dbg_getAddress(uint32_t *address) const
{
	*address = d->VDP_Ctrl.address;
	return 0;
}

/**
 * Set the VDP address register.
 * @param address Register value.
 * @return MDP error code.
 */
int Vdp::dbg_setAddress(uint32_t address)
{
	// TODO: Additional processing?
	// TODO: Convert VDP_Ctrl.address to uint32_t for 128 KB support.
	d->VDP_Ctrl.address = (uint16_t)address;
	return 0;
}

/**
 * Get the VDP control word latch.
 * @param latch Buffer for latch value.
 * @return MDP error code.
 */
int Vdp::dbg_getCtrlLatch(int *latch) const
{
	*latch = d->VDP_Ctrl.ctrl_latch;
	return 0;
}

/**
 * Set the VDP control word latch.
 * @param latch Latch value.
 * @return MDP error code.
 */
int Vdp::dbg_setCtrlLatch(int latch)
{
	// TODO: Additional processing?
	// NOTE: Latch is effectively a bool, so normalize it.
	d->VDP_Ctrl.ctrl_latch = !!latch;
	return 0;
}

/**
 * Write data to VRAM.
 * @param address Destination address.
 * @param vram VRAM data.
 * @param length Length, in bytes.
 * @return 0 on success; non-zero on error.
 */
int Vdp::dbg_writeVRam_16(uint32_t address, const uint16_t *vram, int length)
{
	if (address & 1 || length & 1 ||
	    address >= 0x10000 || address + length > 0x10000) {
		// Invalid address:
		// - Address and length must be even.
		// - Must start within VRAM.
		// - Must not wrap around the end of VRAM.
		// TODO: Support 128 KB VRAM?
		return -1;
	}

	memcpy(&d->VRam.u16[address>>1], vram, length);

	// Check if the VRAM write overlaps the Sprite Attribute Table.
	// TODO: Optimize this into a few calculations and a memcpy.
	for (; length > 0; address += 2, length -= 2, vram++) {
		if ((address & d->Spr_Tbl_Mask) == d->Spr_Tbl_Addr) {
			// Sprite Attribute Table.
			d->SprAttrTbl_m5.w[(address & ~d->Spr_Tbl_Mask) >> 1] = *vram;
		}
	}

	/* TODO: Potential optimization things...
	const uint32_t VRAM_end = address + length + 1;
	uint32_t SAT_min = d->Spr_Tbl_Addr;
	uint32_t SAT_max = SAT_min + ~d->Spr_Tbl_Mask;
	*/

	return 0;
}

/**
 * Write data to CRAM.
 * @param address Destination address.
 * @param cram CRAM data.
 * @param length Length, in bytes.
 * @return 0 on success; non-zero on error.
 */
int Vdp::dbg_writeCRam_16(uint8_t address, const uint16_t *cram, int length)
{
	if (address & 1 || length & 1 ||
	    address >= 0x80 || (int)address + length > 0x80) {
		// Invalid address:
		// - Address and length must be even.
		// - Must start within CRAM.
		// - Must not wrap around the end of CRAM.
		return -1;
	}

	for (; length > 0; address += 2, length -= 2, cram++) {
		d->palette.writeCRam_16(address, *cram);
	}
	return 0;
}

/**
 * Write data to VSRAM.
 * @param address Destination address.
 * @param vram VSRAM data.
 * @param length Length, in bytes.
 * @return 0 on success; non-zero on error.
 */
int Vdp::dbg_writeVSRam_16(uint8_t address, const uint16_t *vsram, int length)
{
	// TODO: Allow 0x50-0x7F on Genesis 3?
	if (address & 1 || length & 1 ||
	    address >= 0x50 || (int)address + length > 0x50) {
		// Invalid address:
		// - Address and length must be even.
		// - Must start within VSRAM.
		// - Must not wrap around the end of VSRAM.
		return -1;
	}

	memcpy(&d->VSRam.u16[address>>1], vsram, length);
	return 0;
}

}
