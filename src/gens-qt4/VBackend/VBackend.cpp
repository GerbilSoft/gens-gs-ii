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
#include "libgens/Util/Timing.hpp"


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
	
	// We're not running anything initially.
	m_running = false;
	
	// Initialize the FPS manager.
	resetFps();
	m_showFps = true;	// TODO: Load from configuration.
	m_osdListDirty = true;	// TODO: Set this on startup?
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


/**
 * osd_vprintf(): Print formatted text to the screen.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to write. (printf-formatted)
 * @param ap Format arguments.
 */
void VBackend::osd_vprintf(const int duration, const char *msg, va_list ap)
{
	if (duration <= 0)
		return;
	
	// Format the message.
	char msg_buf[1024];
	vsnprintf(msg_buf, sizeof(msg_buf), msg, ap);
	msg_buf[sizeof(msg_buf)-1] = 0x00;
	
	// Calculate the end time.
	double endTime = LibGens::Timing::GetTimeD() + ((double)duration / 1000.0);
	
	// Create the OSD message and add it to the list.
	OsdMessage osdMsg(msg_buf, endTime);
	m_osdList.append(osdMsg);
	m_osdListDirty = true;
}


/**
 * resetFps(): Reset the FPS manager.
 */
void VBackend::resetFps(void)
{
	for (int i = 0; i < (sizeof(m_fps)/sizeof(m_fps[0])); i++)
		m_fps[i] = -1.0;
	m_fpsAvg = 0.0;
	m_fpsPtr = 0;
	
	if (isRunning())
		m_osdListDirty = true;
}


/**
 * pushFps(): Push an FPS value.
 * @param fps FPS value.
 */
void VBackend::pushFps(double fps)
{
	m_fpsPtr = (m_fpsPtr + 1) % (sizeof(m_fps)/sizeof(m_fps[0]));
	m_fps[m_fpsPtr] = fps;
	
	// Calculate the new average.
	int count = 0;
	double sum = 0;
	for (int i = 0; i < (sizeof(m_fps)/sizeof(m_fps[0])); i++)
	{
		if (m_fps[i] >= 0.0)
		{
			sum += m_fps[i];
			count++;
		}
	}
	
	if (count <= 0)
		m_fpsAvg = 0.0;
	else
		m_fpsAvg = (sum / (double)count);
	
	if (isRunning())
		m_osdListDirty = true;
}

}
