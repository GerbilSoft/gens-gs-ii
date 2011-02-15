/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_m5_static.hpp: VDP rendering class. (m5) (Static member init)   *
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

#ifndef __LIBGENS_MD_VDPREND_M5_STATIC_HPP__
#define __LIBGENS_MD_VDPREND_M5_STATIC_HPP__

#include "VdpRend_m5.hpp"

namespace LibGens
{

/** Static member initialization. **/

// Interlaced rendering mode.
VdpRend_m5::IntRend_Mode_t VdpRend_m5::IntRend_Mode = VdpRend_m5::INTREND_FLICKER;

// Temporary VDP data.
unsigned int VdpRend_m5::Y_FineOffset;
unsigned int VdpRend_m5::TotalSprites;

}

#endif /* __LIBGENS_MD_VDPREND_M5_STATIC_HPP__ */
