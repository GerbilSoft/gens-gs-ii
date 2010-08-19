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

// Message timer.
#include "MsgTimer.hpp"

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
	setOsdListDirty();	// TODO: Set this on startup?
	
	// Create the message timer.
	m_msgTimer = new MsgTimer(this);
}

VBackend::~VBackend()
{
	// Delete the message timer.
	delete m_msgTimer;
	m_msgTimer = NULL;
	
	// Delete the internal framebuffer.
	delete m_intScreen;
	m_intScreen = NULL;
}


/**
 * setRunning(): Set the emulation running state.
 * @param newIsRunning True if emulation is running; false if it isn't.
 */
void VBackend::setRunning(bool newIsRunning)
{
	if (m_running == newIsRunning)
		return;
	
	m_running = newIsRunning;
	
	// Mark the OSD list as dirty if the FPS counter is visible.
	if (m_showFps)
		setOsdListDirty();
	
	// If emulation isn't running, start the message timer.
	if (!isRunning())
		m_msgTimer->start();
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
	setOsdListDirty();
	
	if (!isRunning())
	{
		// Emulation isn't running.
		// Start the message timer.
		m_msgTimer->start();
	}
}


/**
 * osd_process(): Process the OSD queue.
 * Do NOT call this function externally or from derived classes!
 * It is to be used exclusively with MsgTimer.
 * @return Number of messages remaining in the OSD queue.
 */
int VBackend::osd_process(void)
{
	// Check the message list for expired messages.
	bool isMsgRemoved = false;
	double curTime = LibGens::Timing::GetTimeD();
	for (int i = (m_osdList.size() - 1); i >= 0; i--)
	{
		if (curTime >= m_osdList[i].endTime)
		{
			// Message duration has elapsed.
			// Remove the message from the list.
			m_osdList.removeAt(i);
			isMsgRemoved = true;
			continue;
		}
	}
	
	if (isMsgRemoved)
	{
		// At least one message was removed.
		// Update the video backend.
		setOsdListDirty();
		vbUpdate();
	}
	
	// Return the number of messages remaining.
	return m_osdList.size();
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
	
	if (isRunning() && !isPaused())
		setOsdListDirty();
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
	
	if (isRunning() && !isPaused())
		setOsdListDirty();
}

}
