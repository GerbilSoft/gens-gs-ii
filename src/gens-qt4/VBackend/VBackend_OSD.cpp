/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * VBackend_OSD.cpp: Video Backend class. (Onscreen display functions.)    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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

// C includes. (C++ namespace)
#include <cstdio>

namespace GensQt4
{

/**
 * Print formatted text to the screen.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to write. (printf-formatted)
 * @param ap Format arguments.
 */
void VBackend::osd_vprintf(const int duration, const utf8_str *msg, va_list ap)
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

	// Convert the message to a QString and print it to the screen.
	osd_printqs(duration, QString::fromUtf8(msg_buf));
}

/**
 * Print formatted text to the screen.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to write. (printf-formatted)
 * @param ... Format arguments.
 */
void VBackend::osd_printf(const int duration, const utf8_str *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	osd_vprintf(duration, msg, ap);
	va_end(ap);
}

/**
 * Print text to the screen.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to write.
 * @param forceVbDirty If true, and not running or paused, force the src as dirty and always update the VBackend.
 */
void VBackend::osd_printqs(const int duration, const QString &msg, bool forceVbDirty)
{
	assert(m_osdLockCnt >= 0);
	if (duration <= 0 ||		// Invalid duration.
	    !osdMsgEnabled() ||		// Messages disabled.
	    m_osdLockCnt > 0 ||		// OSD locked.
	    msg.isEmpty())		// Message is empty.
	{
		// Don't write anything to the message buffer.
		if (forceVbDirty) {
			if (!isRunning() || isPaused()) {
				setVbDirty();

				// Force a vbUpdate_int() on the next MsgTimer tick.
				m_updateOnNextOsdProcess = true;

				// Start the message timer.
				m_msgTimer.start(MSGTIMER_INTERVAL);
			}
		}
		return;
	}

	// Create the OSD message and add it to the list.
	OsdMessage osdMsg;
	osdMsg.msg = msg;
	osdMsg.duration = duration;
	osdMsg.hasDisplayed = false;
	osdMsg.endTime = LibGens::Timing::GetTimeD() + ((double)duration / 1000.0);
	m_osdList.append(osdMsg);
	setOsdListDirty();

	if (!isRunning() || isPaused()) {
		// Emulation is either not running or paused.
		// Update the VBackend.
		if (forceVbDirty)
			setVbDirty();

		// Force a vbUpdate_int() on the next MsgTimer tick.
		m_updateOnNextOsdProcess = true;

		// Start the message timer (with a quick initial interval).
		m_msgTimer.start(MSGTIMER_INTERVAL_QUICK);
	}
}

/**
 * Print a QString to the screen.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to write. (printf-formatted)
 */
void VBackend::osd_printqs(const int duration, const QString &msg)
	{ osd_printqs(duration, msg, false); }

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
 * Show a preview image on the OSD.
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
	bool oldPreviewImg_visible = m_previewImg.visible;

	// TODO: Mark as dirty.
	if (img.isNull())
		m_previewImg.clear();
	else
		m_previewImg.set(duration, img);

	// TODO: Only if running?
	if (m_previewImg.visible || oldPreviewImg_visible) {
		// Preivew is visible, or preview was just hidden.
		setVbDirty();

		if (!isRunning() || isPaused()) {
			// Emulation is either not running or paused.
			// Update the VBackend.

			// Force a vbUpdate_int() on the next MsgTimer tick.
			m_updateOnNextOsdProcess = true;

			// Start the message timer (with a quick initial interval).
			m_msgTimer.start(MSGTIMER_INTERVAL_QUICK);
		}
	}
}

/*! FPS manager. **/

/**
 * Reset the FPS manager.
 */
void VBackend::fpsReset(void)
{
	m_fpsManager.reset();
	if (osdFpsEnabled() && isRunning() && !isPaused())
		setOsdListDirty();
}

/**
 * Push an FPS value.
 * @param fps FPS value.
 */
void VBackend::fpsPush(double fps)
{
	m_fpsManager.push(fps);
	if (osdFpsEnabled() && isRunning() && !isPaused())
		setOsdListDirty();
}

/*! Recording status. **/

/**
 * Set recording status for a given component.
 * @param component	[in] Component.
 * @param isRecording	[in] True if recording; false if stopped.
 * @return 0 on success; non-zero on error.
 */
int VBackend::recSetStatus(const QString &component, bool isRecording)
{
	// Find the component in m_osdList.
	int recIdx;
	for (recIdx = 0; recIdx < m_osdRecList.size(); recIdx++) {
		if (m_osdRecList[recIdx].component == component)
			break;
	}

	const double curTime = LibGens::Timing::GetTimeD();

	if (recIdx >= m_osdRecList.size()) {
		// Not found. Add a new component.
		RecOsd recOsd;
		recOsd.component = component;
		recOsd.isRecording = isRecording;
		recOsd.duration = 0;
		recOsd.lastUpdate = curTime;
		m_osdRecList.append(recOsd);
	} else {
		// Component found.
		m_osdRecList[recIdx].isRecording = isRecording;
		m_osdRecList[recIdx].lastUpdate = curTime;
	}

	// Update the OSD.
	setOsdListDirty();

	if (!isRunning() || isPaused()) {
		// Force a vbUpdate_int() on the next MsgTimer tick.
		m_updateOnNextOsdProcess = true;

		// Start the message timer (with a quick initial interval).
		m_msgTimer.start(MSGTIMER_INTERVAL_QUICK);
	}

	return 0;
}

/**
 * Set the recording duration for a component.
 * @param component Component.
 * @param duration Recording duration.
 * @return 0 on success; non-zero on error.
 */
int VBackend::recSetDuration(const QString &component, int duration)
{
	// Find the component in m_osdList.
	int recIdx;
	for (recIdx = 0; recIdx < m_osdRecList.size(); recIdx++) {
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
	setOsdListDirty();

	if (!isRunning() || isPaused()) {
		// Force a vbUpdate_int() on the next MsgTimer tick.
		m_updateOnNextOsdProcess = true;

		// Start the message timer (with a quick initial interval).
		m_msgTimer.start(MSGTIMER_INTERVAL_QUICK);
	}

	return 0;
}

/*! Properties. **/

/**
 * OSD FPS counter visibility setting has changed.
 * @param enable New OSD FPS counter visibility setting.
 */
void VBackend::osdFpsEnabled_changed_slot(const QVariant &enable)
{
	m_cfg_osdFpsEnabled = enable.toBool();

	// TODO: Texture doesn't really need to be reuploaded...
	setVbDirty();

	// Mark the OSD list as dirty if the emulator is running.
	if (isRunning())
		setOsdListDirty();
}

/**
 * OSD FPS counter color has changed.
 * @param var_color New OSD FPS counter color.
 */
void VBackend::osdFpsColor_changed_slot(const QVariant &var_color)
{
	m_cfg_osdFpsColor = var_color.value<QColor>();

	if (osdFpsEnabled() && isRunning()) {
		// Emulator is running.
		// Update the FPS counter.
		setVbDirty();	// TODO: Texture doesn't really need to be reuploaded...
		setOsdListDirty();
	}
}

/**
 * OSD Message visibility setting has changed.
 * @param enable New OSD Message visibility setting.
 */
void VBackend::osdMsgEnabled_changed_slot(const QVariant &enable)
{
	m_cfg_osdMsgEnabled = enable.toBool();

	if (!m_cfg_osdMsgEnabled) {
		// Messages have been disabled.
		// Clear the message list.
		m_osdList.clear();
	}

	// TODO: Texture doesn't really need to be reuploaded...
	setVbDirty();

	// Mark the OSD list as dirty if the emulator is running.
	if (isRunning())
		setOsdListDirty();
}

/**
 * OSD Message color has changed.
 * @param var_color New OSD Message color.
 */
void VBackend::osdMsgColor_changed_slot(const QVariant &var_color)
{
	m_cfg_osdMsgColor = var_color.value<QColor>();

	if (osdMsgEnabled() && !m_osdList.isEmpty()) {
		// Message list has messages.
		// Redraw the messages.
		setVbDirty();	// TODO: Texture doesn't really need to be reuploaded...
		setOsdListDirty();
	}
}

/*! OSD processing functions. **/

/**
 * Process the OSD queue.
 * This should be called by MsgTimer with updateVBackend == true,
 * or subclasses with updateVBackend == false.
 * @param updateVBackend True to update VBackend (MsgTimer); false otherwise (from VBackend).
 * @return Number of messages remaining in the OSD queue.
 */
int VBackend::osd_process_int(bool updateVBackend)
{
	bool isMsgRemoved = false;
	int msgRemaining = 0;

	// Get the current time.
	const double curTime = LibGens::Timing::GetTimeD();

	if (!osdMsgEnabled()) {
		// Messages are disabled. Clear the message list.
		if (!m_osdList.isEmpty()) {
			// Messages exist. Remove them.
			m_osdList.clear();
			isMsgRemoved = true;
		}
	} else {
		// Check the message list for expired messages.
		for (int i = (m_osdList.size() - 1); i >= 0; i--) {
			if (curTime >= m_osdList[i].endTime) {
				if (m_osdList[i].hasDisplayed) {
					// Message duration has elapsed.
					// Remove the message from the list.
					m_osdList.removeAt(i);
					isMsgRemoved = true;
				} else {
					// Message has *not* been displayed.
					// Reset its end time.
					m_osdList[i].endTime =
						curTime + ((double)m_osdList[i].duration / 1000.0);
				}
			}
		}

		msgRemaining = m_osdList.size();
	}

	// Check if the preview image has expired.
	if (m_previewImg.visible) {
		if (curTime >= m_previewImg.endTime) {
			// Preview duration has elapsed.
			m_previewImg.clear();
			isMsgRemoved = true;
		} else {
			// Preview is still visible.
			// Treat it as a message.
			msgRemaining++;
		}
	}

	// TODO: Check m_osdList for REC_STOPPED status over a given duration. (2000 ms?)

	if (isMsgRemoved) {
		// At least one message was removed.
		// Update the video backend.
		setOsdListDirty();
		if (updateVBackend)
			vbUpdate_int();
	} else if (m_updateOnNextOsdProcess && updateVBackend) {
		// vbUpdate_int() is forced.
		vbUpdate_int();
	}

	// Make sure this gets cleared now.
	m_updateOnNextOsdProcess = false;

	if (isRunning() && !isPaused()) {
		// Emulation is either running or is no longer paused.
		// Stop the message timer.
		return 0;
	}

	// Emulation is still either paused or not running.
	// Return the number of messages remaining.
	return msgRemaining;
}

/**
 * Process the OSD queue. (MsgTimer only)
 */
void VBackend::osd_process_MsgTimer_slot(void)
{
	int msgRemaining = osd_process_int(true);

	if (msgRemaining <= 0) {
		// No more messages.
		// Stop the timer.
		m_msgTimer.stop();
		return;
	}

	// Make sure the timer is using the normal interval now.
	m_msgTimer.setInterval(MSGTIMER_INTERVAL);
}

}
