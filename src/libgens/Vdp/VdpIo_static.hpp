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

/** VDP tables. **/

/**
 * Maximum number of DMA transfers per line.
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
