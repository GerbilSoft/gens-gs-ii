/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * VBackend.cpp: Video Backend class.                                      *
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

#include <config.h>
#include "VBackend.hpp"
#include <stdio.h>

// LibGens includes.
#include "libgens/Effects/PausedEffect.hpp"
#include "libgens/Effects/FastBlur.hpp"


namespace GensQt4
{

VBackend::VBackend()
{
	// Mark the video backend as dirty on startup.
	m_vbDirty = true;
	
	// Set the internal framebuffer to NULL by default.
	m_intScreen = NULL;
	
	// Clear the effects flags.
	m_paused = false;
	m_fastBlur = false;
}

VBackend::~VBackend()
{
	// Delete the internal framebuffer if it was allocated.
	if (m_intScreen)
	{
		delete m_intScreen;
		m_intScreen = NULL;
	}
}


/**
 * updatePausedEffect(): Update the Paused effect.
 * @param fromMdScreen If true, copies MD_Screen[] to m_intScreen.
 */
void VBackend::updatePausedEffect(bool fromMdScreen)
{
	// Allocate the internal framebuffer, if necessary.
	if (!m_intScreen)
		m_intScreen = new LibGens::VdpRend::Screen_t;
	
	// Use LibGens' software paused effect function.
	LibGens::PausedEffect::DoPausedEffect(m_intScreen, fromMdScreen);
	
	// Mark the video buffer as dirty.
	setVbDirty();
}


/**
 * updateFastBlur(): Update the Fast Blur effect.
 * @param fromMdScreen If true, copies MD_Screen[] to m_intScreen.
 */
void VBackend::updateFastBlur(bool fromMdScreen)
{
	// Allocate the internal framebuffer, if necessary.
	if (!m_intScreen)
		m_intScreen = new LibGens::VdpRend::Screen_t;
	
	// Use LibGens' software paused effect function.
	LibGens::FastBlur::DoFastBlur(m_intScreen, fromMdScreen);
	
	// Mark the video buffer as dirty.
	setVbDirty();
}

}
