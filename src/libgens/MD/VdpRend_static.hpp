/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend_static.hpp: VDP rendering class. (Static member init)           *
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

#ifndef __LIBGENS_MD_VDPREND_STATIC_HPP__
#define __LIBGENS_MD_VDPREND_STATIC_HPP__

#include "VdpRend.hpp"
#include "VdpPalette.hpp"

namespace LibGens
{

/** Static member initialization. **/

// Palette manager.
VdpPalette VdpRend::m_palette;

// Active MD palette.
VdpRend::MD_Palette_t VdpRend::MD_Palette;

// Screen buffer.
VdpRend::Screen_t VdpRend::MD_Screen;

// Sprite structs.
VdpRend::Sprite_Struct_t VdpRend::Sprite_Struct[128];
unsigned int VdpRend::Sprite_Visible[128];

// If set, enforces sprite limits.
int VdpRend::Sprite_Limits = 1;

// VDP layer control.
unsigned int VdpRend::VDP_Layers = VdpRend::VDP_LAYERS_DEFAULT;

// Line buffer for current line.
VdpRend::LineBuf_t VdpRend::LineBuf;

}

#endif /* __LIBGENS_MD_VDPREND_STATIC_HPP__ */
