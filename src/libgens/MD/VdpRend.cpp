/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend.cpp: VDP rendering class.                                       *
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

#include "VdpRend.hpp"
#include "VdpIo.hpp"

/** Static member initialization. **/
#include "VdpRend_static.hpp"

// Mode-specific renderers.
#include "VdpRend_m5.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

/**
 * VdpRend::Init(): Initialize the VDP rendering subsystem.
 * This function should only be called from VdpIo::Init()!
 */
void VdpRend::Init(void)
{
	// Recalculate the VDP palette.
	// TODO: Wait until the Bpp is set?
	m_palette.recalcFull();
}


/**
 * VdpRend::Init(): Shut down the VDP rendering subsystem.
 * This function should only be called from VdpIo::Init()!
 */
void VdpRend::End(void)
{
	// TODO
}


/**
 * VdpRend::Reset(): Reset the VDP rendering arrays.
 * This function should only be called from VdpIo::Reset()!
 */
void VdpRend::Reset(void)
{
	// Clear MD_Screen.
	memset(&MD_Screen, 0x00, sizeof(MD_Screen));
	
	// Reset the active palettes.
	if (!(VDP_Layers & VDP_LAYER_PALETTE_LOCK))
		m_palette.resetActive();
	
	// Sprite arrays.
	memset(&Sprite_Struct, 0x00, sizeof(Sprite_Struct));
	memset(&Sprite_Visible, 0x00, sizeof(Sprite_Visible));
}


/**
 * VdpRend::Render_Line(): Render a line.
 */
void VdpRend::Render_Line(void)
{
	// TODO: 32X-specific function.
	if (VdpIo::VDP_Mode & VDP_MODE_M5)
	{
		// Mode 5.
		// TODO: Port to LibGens.
		if (VdpIo::SysStatus._32X) { }
#if 0
			VDP_Render_Line_m5_32X();
#endif
		else
			VdpRend_m5::Render_Line();
	}
	else
	{
		// Unsupported mode.
		// TODO: Port to LibGens.
#if 0
		VDP_Render_Error();
#endif
	}
	
	// Update the VDP render error cache.
	// TODO: Port to LibGens.
#if 0
	VDP_Render_Error_Update();
#endif
}

}