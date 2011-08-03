/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend.cpp: VDP rendering code. (Part of the Vdp class.)               *
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

// VDP error message.
#include "VdpRend_Err.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

/**
 * Vdp::rend_init(): Initialize the VDP rendering subsystem.
 * This function should only be called from Init()!
 */
void Vdp::rend_init(void)
{
	// Initialize the VDP rendering variables.
	VDP_Layers = VdpTypes::VDP_LAYERS_DEFAULT;
}


/**
 * Vdp::rend_end(): Shut down the VDP rendering subsystem.
 * This function should only be called from Vdp::Init()!
 */
void Vdp::rend_end(void)
{
	// TODO
}


/**
 * Vdp::rend_reset(): Reset the VDP rendering arrays.
 * This function should only be called from Vdp::reset()!
 */
void Vdp::rend_reset(void)
{
	// Clear MD_Screen.
	memset(&MD_Screen, 0x00, sizeof(MD_Screen));
	
	// Reset the active palettes.
	if (!(VDP_Layers & VdpTypes::VDP_LAYER_PALETTE_LOCK))
		m_palette.resetActive();
	
	// Sprite arrays.
	memset(&Sprite_Struct, 0x00, sizeof(Sprite_Struct));
	memset(&Sprite_Visible, 0x00, sizeof(Sprite_Visible));
}


/**
 * Vdp::Render_Line(): Render a line.
 */
void Vdp::Render_Line(void)
{
	// TODO: 32X-specific function.
	if (VDP_Mode & VDP_MODE_M5)
	{
		// Mode 5.
		// TODO: Port to LibGens.
		if (SysStatus._32X) { }
#if 0
			VDP_Render_Line_m5_32X();
#endif
		else
			Render_Line_m5();
	}
	else
	{
		// Unsupported mode.
		// TODO: Update for non-static VDP.
		//VdpRend_Err::Render_Line();
	}
	
	// Update the VDP render error cache.
	// TODO: Update for non-static VDP.
	//VdpRend_Err::Update();
}

}
