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

#include "VdpIo.hpp"

namespace LibGens
{

/** Static member initialization. **/

// VDP registers.
VdpIo::VDP_Reg_t VdpIo::VDP_Reg;

// These two variables are internal to Gens.
// They don't map to any actual VDP registers.
int VdpIo::DMA_Length;
unsigned int VdpIo::DMA_Address;

// DMAT variables.
unsigned int VdpIo::DMAT_Tmp;
int VdpIo::DMAT_Length;
unsigned int VdpIo::DMAT_Type;

// VDP address pointers.
// These are relative to VRam[] and are based on register values.
uint16_t *VdpIo::ScrA_Addr;
uint16_t *VdpIo::ScrB_Addr;
uint16_t *VdpIo::Win_Addr;
uint16_t *VdpIo::Spr_Addr;
uint16_t *VdpIo::H_Scroll_Addr;

// VDP convenience values: Horizontal.
// NOTE: These must be signed for VDP arithmetic to work properly!
int VdpIo::H_Cell;
int VdpIo::H_Pix;
int VdpIo::H_Pix_Begin;

// Window row shift.
// H40: 6. (64x32 window)
// H32: 5. (32x32 window)
unsigned int VdpIo::H_Win_Shift;

// VDP convenience values: Scroll.
unsigned int VdpIo::V_Scroll_MMask;
unsigned int VdpIo::H_Scroll_Mask;

unsigned int VdpIo::H_Scroll_CMul;
unsigned int VdpIo::H_Scroll_CMask;
unsigned int VdpIo::V_Scroll_CMask;

// TODO: Eliminate these.
int VdpIo::Win_X_Pos;
int VdpIo::Win_Y_Pos;

// Interlaced mode.
VdpIo::Interlaced_t VdpIo::Interlaced;

// Sprite dot overflow.
// If set, the previous line had a sprite dot overflow.
// This is needed to properly implement Sprite Masking in S1.
int VdpIo::SpriteDotOverflow;

// HACK: There's a minor issue with the SegaCD firmware.
// The firmware turns off the VDP after the last line,
// which causes the entire screen to disappear if paused.
// TODO: Don't rerun the VDP drawing functions when paused!
int VdpIo::HasVisibleLines;	// 0 if VDP was off for the whole frame.

// Horizontal Interrupt Counter.
int VdpIo::HInt_Counter;

/**
 * VDP_Ctrl: VDP control struct.
 */
VdpIo::VDP_Ctrl_t VdpIo::VDP_Ctrl;

/**
* VDP_Mode: Current VDP mode.
*/
unsigned int VdpIo::VDP_Mode;

VdpIo::VDP_VRam_t VdpIo::VRam;
VdpIo::VDP_CRam_t VdpIo::CRam;
VdpIo::VSRam_t VdpIo::VSRam;

uint8_t VdpIo::H_Counter_Table[512][2];

int VdpIo::VDP_Int;
int VdpIo::VDP_Status;

// VDP line counters.
// NOTE: Gens/GS currently uses 312 lines for PAL. It should use 313!
VdpIo::VDP_Lines_t VdpIo::VDP_Lines;

// Flags.
VdpIo::VDP_Flags_t VdpIo::VDP_Flags;

// Set this to 1 to enable zero-length DMA requests.
// Default is 0. (hardware-accurate)
int VdpIo::Zero_Length_DMA;

// System status.
// TODO: Move this to a more relevant file.
VdpIo::SysStatus_t VdpIo::SysStatus;


/** VDP tables. **/
const uint32_t VdpIo::CD_Table[64] =
{
	0x0005, 0x0009, 0x0000, 0x000A,	// bits 0-2  = Location (0x00:WRONG, 0x01:VRAM, 0x02:CRAM, 0x03:VSRAM)
	0x0007, 0x000B, 0x0000, 0x0000,	// bits 3-4  = Access   (0x00:WRONG, 0x04:READ, 0x08:WRITE)
	0x0006, 0x0000, 0x0000, 0x0000,	// bits 8-11 = DMA MEM TO VRAM (0x0000:NO DMA, 0x0100:MEM TO VRAM, 0x0200: MEM TO CRAM, 0x0300: MEM TO VSRAM)
	0x0000, 0x0000, 0x0000, 0x0000,	// bits 12   = DMA VRAM FILL (0x0000:NO DMA, 0x0400:VRAM FILL)
	
	0x0005, 0x0009, 0x0000, 0x000A,	// bits 13   = DMA VRAM COPY (0x0000:NO DMA, 0x0800:VRAM COPY)
	0x0007, 0x000B, 0x0000, 0x0000,
	0x0006, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	
	0x0005, 0x0509, 0x0000, 0x020A,
	0x0007, 0x030B, 0x0000, 0x0000,
	0x0006, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	
	/*
	0x0800, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	*/
	
	0x0800, 0x0100, 0x0000, 0x0200,
	0x0000, 0x0300, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000
};

/**
 * DMA_Timing_Table[][]: Maximum number of DMA transfers per line.
 */
const uint8_t VdpIo::DMA_Timing_Table[4][4] =
{
	/* Format: H32 active, H32 blanking, H40 active, H40 blanking */
	{8,    83,   9, 102},	/* 68K to VRam (1 word == 2 bytes) */
	{16,  167,  18, 205},	/* 68K to CRam or VSRam */
	{15,  166,  17, 204},	/* VRam Fill */
	{8,    83,   9, 102},	/* VRam Copy */
};

}

#endif /* __LIBGENS_MD_VDPIO_STATIC_HPP__ */
