/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * VBackend.cpp: Video Backend class.                                      *
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

#include "VBackend.hpp"

// C includes.
#include <stdio.h>
#include <assert.h>

// Qt includes.
#include <QtCore/QMutexLocker>

// LibGens includes.
#include "libgens/Effects/PausedEffect.hpp"
#include "libgens/Effects/FastBlur.hpp"
#include "libgens/Util/Timing.hpp"

// Message timer.
#include "MsgTimer.hpp"

// gqt4_main.hpp has GensConfig.
#include "gqt4_main.hpp"

// Qt Key Handler.
#include "Input/KeyHandlerQt.hpp"

namespace GensQt4
{

VBackend::VBackend(QWidget *parent, KeyHandlerQt *keyHandler)
	: QWidget(parent)
	, m_keyHandler(keyHandler)
	
	// Mark the video backend as dirty on startup.
	, m_vbDirty(true)
	, m_mdScreenDirty(true)
	, m_lastBpp(LibGens::VdpPalette::BPP_MAX)
	
	// Set the internal framebuffer to NULL by default.
	, m_intScreen(NULL)
	
	// We're not running anything initially.
	// TODO: Remove m_running and just use m_emuContext?
	, m_emuContext(NULL)
	, m_running(false)
	
	// Make sure the aspect ratio constraint is initialized correctly.
	, m_aspectRatioConstraint_changed(true)
{
	if (m_keyHandler)
	{
		// Connect the key handler's "destroyed" signal.
		connect(m_keyHandler, SIGNAL(destroyed()),
			this, SLOT(keyHandlerDestroyed()));
	}
	
	/** Configuration items. **/
	m_cfg_fastBlur = new ConfigItem(QLatin1String("Graphics/fastBlur"), false, this);
	m_cfg_pauseTint = new ConfigItem(QLatin1String("pauseTint"), false, this);
	m_cfg_aspectRatioConstraint = new ConfigItem(QLatin1String("Graphics/aspectRatioConstraint"), true, this);
	m_cfg_bilinearFilter = new ConfigItem(QLatin1String("Graphics/bilinearFilter"), true, this);;
	m_cfg_stretchMode = new ConfigItem(QLatin1String("Graphics/stretchMode"), 1, this);;
	
	// Initialize the paused setting.
	m_paused.data = 0;
	
	// Initialize the FPS manager.
	resetFps();
	
	// Initialize the OSD settings.
	m_cfg_osdFpsEnabled = new ConfigItem(QLatin1String("OSD/fpsEnabled"), true, this);
	m_cfg_osdFpsColor   = new ConfigItemColor(QLatin1String("OSD/fpsColor"), QColor(Qt::white), this);
	m_cfg_osdMsgEnabled = new ConfigItem(QLatin1String("OSD/msgEnabled"), true, this);
	m_cfg_osdMsgColor   = new ConfigItemColor(QLatin1String("OSD/msgColor"), QColor(Qt::white), this);
	m_osdLockCnt = 0;	// OSD lock counter.
	setOsdListDirty();	// TODO: Set this on startup?
	
	// Clear the preview image.
	m_preview_show = false;
	
	// Create the message timer.
	m_msgTimer = new MsgTimer(this);
	
	// Connect signals from GensConfig.
	// TODO: Reconnect signals if GensConfig is deleted/recreated.
	connect(m_cfg_osdFpsEnabled, SIGNAL(valueChanged(const QVariant&)),
		this, SLOT(osdFpsEnabled_changed_slot(const QVariant&)));
	connect(m_cfg_osdFpsColor, SIGNAL(valueChanged(const QColor&)),
		this, SLOT(osdFpsColor_changed_slot(const QColor&)));
	connect(m_cfg_osdMsgEnabled, SIGNAL(valueChanged(const QVariant&)),
		this, SLOT(osdMsgEnabled_changed_slot(const QVariant&)));
	connect(m_cfg_osdMsgColor, SIGNAL(valueChanged(const QColor&)),
		this, SLOT(osdMsgColor_changed_slot(const QColor&)));
	
	// Video effect settings.
	connect(m_cfg_fastBlur, SIGNAL(valueChanged(const QVariant&)),
		this, SLOT(fastBlur_changed_slot(const QVariant&)));
	connect(m_cfg_aspectRatioConstraint, SIGNAL(valueChanged(const QVariant&)),
		this, SLOT(aspectRatioConstraint_changed_slot(const QVariant&)));
	connect(m_cfg_bilinearFilter, SIGNAL(valueChanged(const QVariant&)),
		this, SLOT(bilinearFilter_changed_slot(const QVariant&)));
	connect(m_cfg_pauseTint, SIGNAL(valueChanged(const QVariant&)),
		this, SLOT(pauseTint_changed_slot(const QVariant&)));
	connect(m_cfg_stretchMode, SIGNAL(valueChanged(const QVariant&)),
		this, SLOT(stretchMode_changed_slot(const QVariant&)));
}

VBackend::~VBackend()
{
	// Delete internal objects.
	delete m_msgTimer;	// Message timer.
	delete m_intScreen;	// Internal screen buffer.
}


/**
 * setKeyHandler(): Set the key handler.
 * @param newKeyHandler New key handler.
 */
void VBackend::setKeyHandler(KeyHandlerQt *newKeyHandler)
{
	if (m_keyHandler)
	{
		// Disconnect the existing key handler's "destroyed" signal.
		disconnect(m_keyHandler, SIGNAL(destroyed()),
			   this, SLOT(keyHandlerDestroyed()));
	}
	
	m_keyHandler = newKeyHandler;
	if (m_keyHandler)
	{
		// Connect the new key handler's "destroyed" signal.
		connect(m_keyHandler, SIGNAL(destroyed()),
			this, SLOT(keyHandlerDestroyed()));
	}
}


/**
 * keyHandlerDestroyed(): Key handler was destroyed.
 */
void VBackend::keyHandlerDestroyed(void)
{
	m_keyHandler = NULL;
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
 * stretchMode_changed_slot(): Stretch mode setting has changed.
 * @param newStretchMode (int) New stretch mode setting.
 */
void VBackend::stretchMode_changed_slot(const QVariant& newStretchMode)
{
	StretchMode_t stretch = (StretchMode_t)newStretchMode.toInt();
	
	// Verify that the new stretch mode is valid.
	// TODO: New ConfigItem subclass for StretchMode_t.
	if ((stretch < STRETCH_NONE) || (stretch > STRETCH_FULL))
	{
		// Invalid stretch mode.
		// Reset to default.
		m_cfg_stretchMode->setValue(m_cfg_stretchMode->def());
		return;
	}
	
	// Print a message to the OSD.
	//: OSD message indicating the Stretch Mode has been changed.
	QString msg = tr("Stretch Mode set to %1.", "osd-stretch");
	switch (stretch)
	{
		case STRETCH_NONE:
			//: OSD message indicating the Stretch Mode has been set to None.
			msg = msg.arg(tr("None", "osd-stretch"));
			break;
		
		case STRETCH_H:
			//: OSD message indicating the Stretch Mode has been set to Horizontal.
			msg = msg.arg(tr("Horizontal", "osd-stretch"));
			break;
		
		case STRETCH_V:
			//: OSD message indicating the Stretch Mode has been set to Vertical.
			msg = msg.arg(tr("Vertical", "osd-stretch"));
			break;
		
		case STRETCH_FULL:
			//: OSD message indicating the Stretch Mode has been set to Full.
			msg = msg.arg(tr("Full", "osd-stretch"));
			break;
		
		default:
			msg.clear();
			break;
	}
	
	if (!msg.isEmpty())
		osd_printqs(1500, msg);
	
	// TODO: Only if paused, or regardless of pause?
	if (!isRunning() || isPaused())
	{
		setVbDirty();
		vbUpdate();
	}
}


/**
 * fastBlur_changed_slot(): Fast Blur effect has changed.
 * @param newFastBlur (bool) New Fast Blur effect setting.
 */
void VBackend::fastBlur_changed_slot(const QVariant& newFastBlur)
{
	// Print a message to the OSD.
	if (newFastBlur.toBool())
	{
		//: OSD message indicating Fast Blur has been enabled.
		osd_printqs(1500, tr("Fast Blur enabled.", "osd"));
	}
	else
	{
		//: OSD message indicating Fast Blur has been disabled.
		osd_printqs(1500, tr("Fast Blur disabled.", "osd"));
	}
	
	// If paused, update the VBackend.
	if (isRunning() && isPaused())
	{
		setVbDirty();
		vbUpdate();
	}
}


/**
 * aspectRatioConstraint_changed_slot(): Aspect ratio constraint setting has changed.
 * @param newAspectRatioConstraint (bool) New aspect ratio constraint setting.
 */
void VBackend::aspectRatioConstraint_changed_slot(const QVariant& newAspectRatioConstraint)
{
	// Aspect Ratio Constraint setting has changed.
	m_aspectRatioConstraint_changed = true;
	
	// Update the Video Backend even when not running.
	setVbDirty();
	
	// TODO: Only if paused, or regardless of pause?
	if (!isRunning() || isPaused())
		vbUpdate();
}


/**
 * bilinearFilter_changed_slot(): Bilinear filter setting has changed.
 * @param newBilinearFilter (bool) New bilinear filter setting.
 */
void VBackend::bilinearFilter_changed_slot(const QVariant& newBilinearFilter)
{
	// TODO: Only if paused, or regardless of pause?
	if (!isRunning() || isPaused())
	{
		setVbDirty();
		vbUpdate();
	}
}


/**
 * pauseTint_changed_slot(): Pause Tint effect setting has changed.
 * @param newPauseTint (bool) New pause tint effect setting.
 */
void VBackend::pauseTint_changed_slot(const QVariant& newPauseTint)
{
	// Update the video backend if emulation is running,
	// and if we're currently paused manually.
	if (isRunning() && isManualPaused())
	{
		setVbDirty();
		vbUpdate();
	}
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
		m_intScreen = new LibGens::MdFb();
	
	// Use LibGens' software paused effect function.
	LibGens::PausedEffect::DoPausedEffect(m_intScreen, fromMdScreen);
	
	// Mark the video backend as dirty.
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
		m_intScreen = new LibGens::MdFb();
	
	// Use LibGens' software paused effect function.
	LibGens::FastBlur::DoFastBlur(m_intScreen, fromMdScreen);
	
	// Mark the video backend as dirty.
	setVbDirty();
}


/**
 * osd_vprintf(): Print formatted text to the screen.
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
	msg_buf[sizeof(msg_buf)-1] = 0x00;
	
	// Convert the message to a QString and print it to the screen.
	osd_printqs(duration, QString::fromUtf8(msg_buf));
}


/**
 * osd_vprintf(): Print text to the screen.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to write.
 */
void VBackend::osd_printqs(const int duration, const QString& msg)
{
	assert(m_osdLockCnt >= 0);
	if (duration <= 0 ||		// Invalid duration.
	    !osdMsgEnabled() ||		// Messages disabled.
	    m_osdLockCnt > 0)		// OSD locked.
	{
		// Don't write anything to the message buffer.
		return;
	}
	
	// Calculate the end time.
	const double endTime = LibGens::Timing::GetTimeD() + ((double)duration / 1000.0);
	
	// Create the OSD message and add it to the list.
	OsdMessage osdMsg(msg, endTime);
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
 * osdFpsEnabled_changed_slot(): OSD FPS counter visibility setting has changed.
 * @param enable New OSD FPS counter visibility setting.
 */
void VBackend::osdFpsEnabled_changed_slot(const QVariant& enable)
{
	// TODO: Texture doesn't really need to be reuploaded...
	setVbDirty();
	
	// Mark the OSD list as dirty if the emulator is running.
	if (isRunning())
		setOsdListDirty();
}


/**
 * osdFpsColor_changed_slot(): OSD FPS counter color has changed.
 * @param color New OSD FPS counter color.
 */
void VBackend::osdFpsColor_changed_slot(const QColor& color)
{
	if (!color.isValid())
	{
		// Invalid color. Reset to default.
		m_cfg_osdFpsColor->setValue(m_cfg_osdFpsColor->def());
		return;
	}
	
	if (osdFpsEnabled() && isRunning())
	{
		// Emulator is running.
		// Update the FPS counter.
		setVbDirty();	// TODO: Texture doesn't really need to be reuploaded...
		setOsdListDirty();
	}
}


/**
 * osdMsgEnabled_changed_slot(): OSD Message visibility setting has changed.
 * @param enable New OSD Message visibility setting.
 */
void VBackend::osdMsgEnabled_changed_slot(const QVariant& enable)
{
	// TODO: Texture doesn't really need to be reuploaded...
	setVbDirty();
	
	// Mark the OSD list as dirty if the emulator is running.
	if (isRunning())
		setOsdListDirty();
}


/**
 * osdMsgColor_changed_slot(): OSD Message color has changed.
 * @param color New OSD Message color.
 */
void VBackend::osdMsgColor_changed_slot(const QColor& color)
{
	if (!color.isValid())
	{
		// Invalid color. Reset to default.
		m_cfg_osdMsgColor->setValue(m_cfg_osdMsgColor->def());
		return;
	}
	
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


/**
 * setEmuContext(): Set the emulation context for this video backend.
 * @param newEmuContext New emulation context.
 */
void VBackend::setEmuContext(LibGens::EmuContext *newEmuContext)
{
	// Emulation Context must be locked before use.
	QMutexLocker lockEmuContext(&m_mtxEmuContext);
	
	if (m_emuContext == newEmuContext)
		return;
	
	m_emuContext = newEmuContext;
	lockEmuContext.unlock();
	
	setVbDirty();
	setMdScreenDirty();
	
	// TODO: Should we update the video buffer here?
	if (!isRunning() || isPaused())
		vbUpdate();
}

}
