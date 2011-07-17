/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpIo_static.hpp: VDP class: I/O functions. (Static member init)        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __LIBGENS_MD_VDPIO_STATIC_HPP__
#define __LIBGENS_MD_VDPIO_STATIC_HPP__

#include "Vdp.hpp"
#include "VdpTypes.hpp"

namespace LibGens
{

/** Static member initialization. **/

// VDP registers.
VdpTypes::VdpReg_t Vdp::VDP_Reg;

// These two variables are internal to Gens.
// They don't map to any actual VDP registers.
int Vdp::DMA_Length;
unsigned int Vdp::DMA_Address;

// DMAT variables.
unsigned int Vdp::DMAT_Tmp;
int Vdp::DMAT_Length;
unsigned int Vdp::DMAT_Type;

// VDP address pointers.
// These are relative to VRam[] and are based on register values.
uint16_t Vdp::ScrA_Addr;
uint16_t Vdp::ScrB_Addr;
uint16_t Vdp::Win_Addr;
uint16_t Vdp::Spr_Addr;
uint16_t Vdp::H_Scroll_Addr;

// VDP convenience values: Horizontal.
// NOTE: These must be signed for VDP arithmetic to work properly!
int Vdp::H_Cell;
int Vdp::H_Pix;
int Vdp::H_Pix_Begin;

// Window row shift.
// H40: 6. (64x32 window)
// H32: 5. (32x32 window)
unsigned int Vdp::H_Win_Shift;

// VDP convenience values: Scroll.
unsigned int Vdp::V_Scroll_MMask;
unsigned int Vdp::H_Scroll_Mask;

unsigned int Vdp::H_Scroll_CMul;
unsigned int Vdp::H_Scroll_CMask;
unsigned int Vdp::V_Scroll_CMask;

// TODO: Eliminate these.
int Vdp::Win_X_Pos;
int Vdp::Win_Y_Pos;

// Interlaced mode.
VdpTypes::Interlaced_t Vdp::Interlaced;

// Sprite dot overflow.
// If set, the previous line had a sprite dot overflow.
// This is needed to properly implement Sprite Masking in S1.
int Vdp::SpriteDotOverflow;

// Horizontal Interrupt Counter.
int Vdp::HInt_Counter;

/**
 * VDP_Ctrl: VDP control struct.
 */
Vdp::VDP_Ctrl_t Vdp::VDP_Ctrl;

/**
* VDP_Mode: Current VDP mode.
*/
unsigned int Vdp::VDP_Mode;

VdpTypes::VRam_t Vdp::VRam;
VdpTypes::CRam_t Vdp::CRam;
VdpTypes::VSRam_t Vdp::VSRam;

uint8_t Vdp::H_Counter_Table[512][2];

int Vdp::VDP_Int;
int Vdp::VDP_Status;

// VDP line counters.
// NOTE: Gens/GS currently uses 312 lines for PAL. It should use 313!
VdpTypes::VdpLines_t Vdp::VDP_Lines;

// Update flags.
VdpTypes::UpdateFlags_t Vdp::UpdateFlags;

// Set this to 1 to enable zero-length DMA requests.
// Default is 0. (hardware-accurate)
int Vdp::Zero_Length_DMA;

// System status.
// TODO: Move this to a more relevant file.
Vdp::SysStatus_t Vdp::SysStatus;


/** VDP tables. **/

/**
 * CD_Table[]: VDP memory destination table.
 * Maps VDP control word destinations to VDEST_t values.
 */
const uint16_t Vdp::CD_Table[64] =
{
	// 0x00 - 0x0F
	VDEST_LOC_VRAM | VDEST_ACC_READ,	VDEST_LOC_VRAM | VDEST_ACC_WRITE,
	VDEST_INVALID,				VDEST_LOC_CRAM | VDEST_ACC_WRITE,
	VDEST_LOC_VSRAM | VDEST_ACC_READ,	VDEST_LOC_VSRAM | VDEST_ACC_WRITE,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_LOC_CRAM | VDEST_ACC_READ,	VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	
	// 0x10 - 0x1F
	VDEST_LOC_VRAM | VDEST_ACC_READ,	VDEST_LOC_VRAM | VDEST_ACC_WRITE,
	VDEST_INVALID,				VDEST_LOC_CRAM | VDEST_ACC_WRITE,
	VDEST_LOC_VSRAM | VDEST_ACC_READ,	VDEST_LOC_VSRAM | VDEST_ACC_WRITE,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_LOC_CRAM | VDEST_ACC_READ,	VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	
	// 0x20 - 0x2F
	VDEST_LOC_VRAM | VDEST_ACC_READ,	VDEST_LOC_VRAM | VDEST_ACC_WRITE | VDEST_DMA_MEM_TO_VRAM | VDEST_DMA_FILL,
	VDEST_INVALID,				VDEST_LOC_CRAM | VDEST_ACC_WRITE | VDEST_DMA_MEM_TO_CRAM,
	VDEST_LOC_VSRAM | VDEST_ACC_READ,	VDEST_LOC_VSRAM | VDEST_ACC_WRITE | VDEST_DMA_MEM_TO_VSRAM,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_LOC_CRAM | VDEST_ACC_READ,	VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	
	// 0x30 - 0x3F
	VDEST_DMA_COPY,				VDEST_DMA_MEM_TO_VRAM,
	VDEST_INVALID,				VDEST_DMA_MEM_TO_CRAM,
	VDEST_INVALID,				VDEST_DMA_MEM_TO_VSRAM,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID,
	VDEST_INVALID,				VDEST_INVALID
};


/**
 * DMA_Timing_Table[][]: Maximum number of DMA transfers per line.
 */
const uint8_t Vdp::DMA_Timing_Table[4][4] =
{
	/* Format: H32 active, H32 blanking, H40 active, H40 blanking */
	{8,    83,   9, 102},	/* 68K to VRam (1 word == 2 bytes) */
	{16,  167,  18, 205},	/* 68K to CRam or VSRam */
	{15,  166,  17, 204},	/* VRam Fill */
	{8,    83,   9, 102},	/* VRam Copy (1 word == 2 bytes) */
};

}

#endif /* __LIBGENS_MD_VDPIO_STATIC_HPP__ */
