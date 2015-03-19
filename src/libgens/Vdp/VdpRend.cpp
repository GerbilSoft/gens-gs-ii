/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpRend.cpp: VDP rendering code. (Part of the Vdp class.)               *
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

// C includes.
#include <string.h>

// Vdp private class.
#include "Vdp_p.hpp"

namespace LibGens {

/**
 * Initialize the VDP rendering subsystem.
 * This function should only be called from Vdp::Vdp()!
 */
void VdpPrivate::rend_init(void)
{
	// Initialize the VDP rendering variables.
	VDP_Layers = VdpTypes::VDP_LAYERS_DEFAULT;
}

/**
 * Shut down the VDP rendering subsystem.
 * This function should only be called from Vdp::~Vdp()!
 */
void VdpPrivate::rend_end(void)
{
	// TODO
}

/**
 * Reset the VDP rendering arrays.
 * This function should only be called from Vdp::reset()!
 */
void VdpPrivate::rend_reset(void)
{
	// Clear MD_Screen.
	q->MD_Screen->clear();

	// Reset the active palettes.
	// TODO: Handle VDP_LAYER_PALETTE_LOCK in VdpPalette.
#if 0
	if (!(VDP_Layers & VdpTypes::VDP_LAYER_PALETTE_LOCK))
		m_palette.resetActive();
#endif

	// Sprite Attribute Table cache. (Mode 5)
	memset(SprAttrTbl_m5.b, 0, sizeof(SprAttrTbl_m5.b));

	// Sprite arrays.
	memset(&Sprite_Struct, 0x00, sizeof(Sprite_Struct));
	memset(&Sprite_Visible, 0x00, sizeof(Sprite_Visible));
}

/**
 * Render a line.
 */
void Vdp::renderLine(void)
{
	// TODO: 32X-specific function.
	if (d->VDP_Mode & VdpPrivate::VDP_MODE_M5) {
		// Mode 5.
		// TODO: Port to LibGens.
		if (SysStatus._32X) {
#if 0
			d->renderLine_m5_32X();
#endif
		} else {
			d->renderLine_m5();
		}
	} else {
		// Unsupported mode.
		d->renderLine_Err();
	}

	// Update the VDP render error cache.
	d->updateErr();
}

}
