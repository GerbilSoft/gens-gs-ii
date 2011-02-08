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

#include "VBackend.hpp"

// C includes.
#include <stdio.h>
#include <assert.h>

// LibGens includes.
#include "libgens/Effects/PausedEffect.hpp"
#include "libgens/Effects/FastBlur.hpp"
#include "libgens/Util/Timing.hpp"

// Message timer.
#include "MsgTimer.hpp"

// gqt4_main.hpp has GensConfig.
#include "gqt4_main.hpp"

namespace GensQt4
{

VBackend::VBackend()
{
	// Mark the video backend as dirty on startup.
	m_vbDirty = true;
	
	// Set the internal framebuffer to NULL by default.
	m_intScreen = NULL;
	
	// Reset the OSD lock counter.
	m_osdLockCnt = 0;
	
	// We're not running anything initially.
	m_running = false;
	
	// Set default video settings.
	m_paused.data = 0;
	m_fastBlur = false;
	m_stretchMode = STRETCH_H;	// TODO: Load from configuration.
	m_aspectRatioConstraint = true;
	m_aspectRatioConstraint_changed = true;
	
	// Initialize the FPS manager.
	resetFps();
	
	// Initialize the OSD settings.
	m_osdFpsEnabled = gqt4_config->osdFpsEnabled();
	m_osdFpsColor   = gqt4_config->osdFpsColor();
	m_osdMsgEnabled = gqt4_config->osdMsgEnabled();
	m_osdMsgColor   = gqt4_config->osdMsgColor();
	setOsdListDirty();	// TODO: Set this on startup?
	
	// Clear the preview image.
	m_preview_show = false;
	
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
 * setPaused(): Set the emulation paused state.
 * @param newPaused Paused state.
 */
void VBackend::setPaused(paused_t newPaused)
{
	if (m_paused.data == newPaused.data)
		return;
	
	// Update the paused status.
	const bool pause_manual_changed = (m_paused.paused_manual != newPaused.paused_manual);
	m_paused.data = newPaused.data;
	
	// Update VBackend on one of the following conditions:
	// - Manual pause changed and pause tint is enabled.
	// - FPS is enabled.
	if ((pause_manual_changed && gqt4_config->pauseTint()) || osdFpsEnabled())
	{
		// Update the video backend.
		if (isRunning())
		{
			setVbDirty();
			if (osdFpsEnabled())
				setOsdListDirty();
			vbUpdate(); // TODO: Do we really need to call this here?
		}
	}
	
	// If emulation isn't running or emulation is paused, start the message timer.
	if (!isRunning() || isPaused())
		m_msgTimer->start();
}


/**
 * setStretchMode(): Set the stretch mode setting.
 * @param newStretchMode New stretch mode setting.
 */
void VBackend::setStretchMode(StretchMode newStretchMode)
{
	if (m_stretchMode == newStretchMode)
		return;
	
	// Update the stretch mode setting.
	// TODO: Verify that this works properly.
	m_stretchMode = newStretchMode;
	if (isRunning())
		setVbDirty();
	if (isRunning())
	{
		setVbDirty();
		// TODO: Only if paused, or regardless of pause?
		if (isPaused())
			vbUpdate();
	}
}


/**
 * setFastBlur(): Set the Fast Blur effect setting.
 * @param newFastBlur True to enable Fast Blur; false to disable it.
 */
void VBackend::setFastBlur(bool newFastBlur)
{
	if (m_fastBlur == newFastBlur)
		return;
	
	// Update the Fast Blur setting.
	m_fastBlur = newFastBlur;
	if (isRunning())
	{
		setVbDirty();
		// TODO: Only if paused, or regardless of pause?
		if (isPaused())
			vbUpdate();
	}
}


/**
 * setAspectRatioConstraint(): Set the aspect ratio constraint setting.
 * @param newAspectRatioConstraint True to enable Aspect Ratio Constraint; false to disable it.
 */
void VBackend::setAspectRatioConstraint(bool newAspectRatioConstraint)
{
	if (m_aspectRatioConstraint == newAspectRatioConstraint)
		return;
	
	// Update the Aspect Ratio Constraint setting.
	m_aspectRatioConstraint = newAspectRatioConstraint;
	m_aspectRatioConstraint_changed = true;
	
	// Update the Video Backend even when not running.
	setVbDirty();
	
	// TODO: Only if paused, or regardless of pause?
	if (!isRunning() || isPaused())
		vbUpdate();
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
	if (osdFpsEnabled())
	{
		setOsdListDirty();
		if (!isRunning())
		{
			// Emulation isn't running.
			// Update the display immediately.
			vbUpdate();
		}
	}
	
	// If emulation isn't running or emulation is paused, start the message timer.
	if (!isRunning() || isPaused())
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
	assert(m_osdLockCnt >= 0);
	if (duration <= 0 ||		// Invalid duration.
	    !osdMsgEnabled() ||		// Messages disabled.
	    m_osdLockCnt > 0)		// OSD locked.
	{
		// Don't write anything to the message buffer.
		return;
	}
	
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
	
	if (!isRunning() || isPaused())
	{
		// Emulation is either not running or paused.
		// Update the VBackend.
		vbUpdate();
		
		// Start the message timer.
		m_msgTimer->start();
	}
}


/**
 * osd_lock() / osd_unlock(): Temporarily lock the OSD.
 * Primarily used when loading settings all at once.
 * Calls are cumulative; 2 locks requires 2 unlocks.
 * Calling osd_unlock() when not locked will return an error.
 * @return 0 on success; non-zero on error.
 */
int VBackend::osd_lock(void)
	{ m_osdLockCnt++; return 0; }
int VBackend::osd_unlock(void)
{
	assert(m_osdLockCnt >= 0);
	if (m_osdLockCnt <= 0)
		return -1;
	
	m_osdLockCnt--;
	return 0;
}


/**
 * osd_process(): Process the OSD queue.
 * This should ONLY be called by MsgTimer!
 * @return Number of messages remaining in the OSD queue.
 */
int VBackend::osd_process(void)
{
	bool isMsgRemoved = false;
	int msgRemaining = 0;
	
	// Get the current time.
	const double curTime = LibGens::Timing::GetTimeD();
	
	if (!osdMsgEnabled())
	{
		// Messages are disabled. Clear the message list.
		if (!m_osdList.isEmpty())
		{
			// Messages exist. Remove them.
			m_osdList.clear();
			isMsgRemoved = true;
		}
	}
	else
	{
		// Check the message list for expired messages.
		for (int i = (m_osdList.size() - 1); i >= 0; i--)
		{
			if (curTime >= m_osdList[i].endTime)
			{
				// Message duration has elapsed.
				// Remove the message from the list.
				m_osdList.removeAt(i);
				isMsgRemoved = true;
			}
		}
		
		msgRemaining = m_osdList.size();
	}
	
	// Check if the preview image has expired.
	if (m_preview_show)
	{
		if (curTime >= m_preview_endTime)
		{
			// Preview duration has elapsed.
			m_preview_show = false;
			m_preview_img = QImage();
			isMsgRemoved = true;
		}
		else
		{
			// Preview is still visible.
			// Treat it as a message.
			msgRemaining++;
		}
	}
	
	// TODO: Check m_osdList for REC_STOPPED status over a given duration. (2000 ms?)
	
	if (isMsgRemoved)
	{
		// At least one message was removed.
		// Update the video backend.
		setOsdListDirty();
		vbUpdate();
	}
	
	if (isRunning() && !isPaused())
	{
		// Emulation is either running or is no longer paused.
		// Stop the message timer.
		return 0;
	}
	
	// Emulation is still either paused or not running.
	// Return the number of messages remaining.
	return msgRemaining;
}


/**
 * resetFps(): Reset the FPS manager.
 */
void VBackend::resetFps(void)
{
	for (size_t i = 0; i < (sizeof(m_fps)/sizeof(m_fps[0])); i++)
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
	for (size_t i = 0; i < (sizeof(m_fps)/sizeof(m_fps[0])); i++)
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


/**
 * setOsdFpsEnabled(): Set the OSD FPS counter visibility setting.
 * @param enable True to show FPS; false to hide FPS.
 */
void VBackend::setOsdFpsEnabled(bool enable)
{
	if (m_osdFpsEnabled == enable)
		return;
	
	// Update the Show FPS setting.
	m_osdFpsEnabled = enable;
	setVbDirty();		// TODO: Texture doesn't really need to be reuploaded...
	
	// Mark the OSD list as dirty if the emulator is running.
	if (isRunning())
		setOsdListDirty();
}


/**
 * setOsdFpsColor(): Set the OSD FPS counter color.
 * @param color New OSD FPS counter color.
 */
void VBackend::setOsdFpsColor(const QColor& color)
{
	if (!color.isValid() || m_osdFpsColor == color)
		return;
	
	// Update the FPS counter color.
	m_osdFpsColor = color;
	
	if (osdFpsEnabled() && isRunning())
	{
		// Emulator is running.
		// Update the FPS counter.
		setVbDirty();	// TODO: Texture doesn't really need to be reuploaded...
		setOsdListDirty();
	}
}


/**
 * setOsdMsgEnabled(): Set the OSD Message visibility setting.
 * @param enable True to show messages; false to hide messages.
 */
void VBackend::setOsdMsgEnabled(bool enable)
{
	if (m_osdMsgEnabled == enable)
		return;
	
	// Update the Show FPS setting.
	m_osdMsgEnabled = enable;
	setVbDirty();		// TODO: Texture doesn't really need to be reuploaded...
	
	// Mark the OSD list as dirty if the emulator is running.
	if (isRunning())
		setOsdListDirty();
}


/**
 * setOsdMsgColor(): Set the OSD Message color.
 * @param color New OSD Message color.
 */
void VBackend::setOsdMsgColor(const QColor& color)
{
	if (!color.isValid() || m_osdMsgColor == color)
		return;
	
	// Update the message color.
	m_osdMsgColor = color;
	
	if (osdMsgEnabled() && !m_osdList.isEmpty())
	{
		// Message list has messages.
		// Redraw the messages.
		setVbDirty();	// TODO: Texture doesn't really need to be reuploaded...
		setOsdListDirty();
	}
}


/**
 * osd_show_preview(): Show a preview image on the OSD.
 * @param duration Duration for the preview image to appaer, in milliseconds.
 * @param img Image to show.
 */
void VBackend::osd_show_preview(int duration, const QImage& img)
{
	// If the OSD is locked, don't do anything.
	assert(m_osdLockCnt >= 0);
	if (m_osdLockCnt > 0)
		return;
	
	// Save the current preview visibility state.
	bool old_preview_show = m_preview_show;
	
	// TODO: Mark as dirty.
	if (img.isNull())
	{
		// NULL image.
		m_preview_img = QImage();
		m_preview_show = false;
	}
	else
	{
		// Save the image and display it on the next paintGL().
		m_preview_img = img;
		m_preview_show = true;
		m_preview_endTime = LibGens::Timing::GetTimeD() + ((double)duration / 1000.0);
	}
	
	// TODO: Only if running?
	if (m_preview_show || old_preview_show)
	{
		// Preivew is visible, or preview was just hidden.
		setVbDirty();
		
		if (!isRunning() || isPaused())
		{
			// Emulation is either not running or paused.
			// Update the VBackend.
			vbUpdate();
			
			// Start the message timer.
			m_msgTimer->start();
		}
	}
}


/**
 * recSetStatus(): Set recording status for a given component.
 * @param component	[in] Component.
 * @param isRecording	[in] True if recording; false if stopped.
 * @return 0 on success; non-zero on error.
 */
int VBackend::recSetStatus(const QString& component, bool isRecording)
{
	// Find the component in m_osdList.
	int recIdx;
	for (recIdx = 0; recIdx < m_osdRecList.size(); recIdx++)
	{
		if (m_osdRecList[recIdx].component == component)
			break;
	}
	
	const double curTime = LibGens::Timing::GetTimeD();
	
	if (recIdx >= m_osdRecList.size())
	{
		// Not found. Add a new component.
		RecOsd recOsd;
		recOsd.component = component;
		recOsd.isRecording = isRecording;
		recOsd.duration = 0;
		recOsd.lastUpdate = curTime;
		m_osdRecList.append(recOsd);
	}
	else
	{
		// Component found.
		m_osdRecList[recIdx].isRecording = isRecording;
		m_osdRecList[recIdx].lastUpdate = curTime;
	}
	
	// Update the OSD.
	setVbDirty();	// TODO: Texture doesn't really need to be reuploaded...
	setOsdListDirty();
	// TODO: Only if paused, or regardless of pause?
	if (!isRunning() || isPaused())
		vbUpdate();
	return 0;
}


/**
 * recSetDuration(): Set the recording duration for a component.
 * @param component Component.
 * @param duration Recording duration.
 * @return 0 on success; non-zero on error.
 */
int VBackend::recSetDuration(const QString& component, int duration)
{
	// Find the component in m_osdList.
	int recIdx;
	for (recIdx = 0; recIdx < m_osdRecList.size(); recIdx++)
	{
		if (m_osdRecList[recIdx].component == component)
			break;
	}
	
	if (recIdx >= m_osdRecList.size())
		return -1;
	
	// Set the recording duration.
	const double curTime = LibGens::Timing::GetTimeD();
	m_osdRecList[recIdx].duration = duration;
	m_osdRecList[recIdx].lastUpdate = curTime;
	
	// Update the OSD.
	setVbDirty();	// TODO: Texture doesn't really need to be reuploaded...
	setOsdListDirty();
	// TODO: Only if paused, or regardless of pause?
	if (!isRunning() || isPaused())
		vbUpdate();
	return 0;
}

}
