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

/** Static member initialization. **/
#include "VdpRend_static.hpp"

// VDP error message.
#include "VdpRend_Err.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

/**
 * Vdp::Rend_Init(): Initialize the VDP rendering subsystem.
 * This function should only be called from Init()!
 */
void Vdp::Rend_Init(void)
{
	// TODO
}


/**
 * Vdp::Rend_End(): Shut down the VDP rendering subsystem.
 * This function should only be called from Vdp::Init()!
 */
void Vdp::Rend_End(void)
{
	// TODO
}


/**
 * Vdp::Rend_Reset(): Reset the VDP rendering arrays.
 * This function should only be called from Vdp::Reset()!
 */
void Vdp::Rend_Reset(void)
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
		VdpRend_Err::Render_Line();
	}
	
	// Update the VDP render error cache.
	VdpRend_Err::Update();
}

}
