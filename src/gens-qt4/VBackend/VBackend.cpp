/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * VBackend.cpp: Video Backend class.                                      *
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
#include <cassert>

// Qt includes.
#include <QtCore/QMutexLocker>

// LibGens includes.
#include "libgens/Effects/PausedEffect.hpp"
#include "libgens/Effects/FastBlur.hpp"
#include "libgens/Util/Timing.hpp"

// gqt4_main.hpp has GensConfig.
#include "gqt4_main.hpp"

// Qt Key Handler.
#include "Input/KeyHandlerQt.hpp"

namespace GensQt4
{

VBackend::VBackend(QWidget *parent, KeyHandlerQt *keyHandler)
	: QWidget(parent)

	// No source framebuffer initially.
	, m_srcFb(nullptr)
	, m_srcBpp(LibGens::VdpPalette::BPP_32)

	// Key handler.
	, m_keyHandler(keyHandler)

	// Mark the video backend as dirty on startup.
	, m_vbDirty(true)
	, m_mdScreenDirty(true)
	, m_lastBpp(LibGens::VdpPalette::BPP_MAX)

	// Allocate the internal screen buffer.
	, m_intScreen(new LibGens::MdFb())

	// We're not running anything initially.
	// TODO: Remove m_running and just use m_emuContext?
	, m_emuContext(nullptr)
	, m_running(false)

	// Make sure the aspect ratio constraint is initialized correctly.
	, m_aspectRatioConstraint_changed(true)

	// Onscreen display.
	, m_osdLockCnt(0)
	, m_updateOnNextOsdProcess(false)
{
	if (m_keyHandler) {
		// Connect the key handler's "destroyed" signal.
		connect(m_keyHandler, SIGNAL(destroyed()),
			this, SLOT(keyHandlerDestroyed()));
	}

	/** Video effect settings. **/
	m_cfg_fastBlur = gqt4_cfg->get(QLatin1String("Graphics/fastBlur")).toBool();
	m_cfg_pauseTint = gqt4_cfg->get(QLatin1String("pauseTint")).toBool();
	m_cfg_aspectRatioConstraint = gqt4_cfg->get(QLatin1String("Graphics/aspectRatioConstraint")).toBool();
	m_cfg_bilinearFilter = gqt4_cfg->get(QLatin1String("Graphics/bilinearFilter")).toBool();
	m_cfg_stretchMode = (StretchMode_t)gqt4_cfg->getInt(QLatin1String("Graphics/stretchMode"));

	/** Video effect settings: Signals. **/
	gqt4_cfg->registerChangeNotification(QLatin1String("Graphics/fastBlur"),
					this, SLOT(fastBlur_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("pauseTint"),
					this, SLOT(pauseTint_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("Graphics/aspectRatioConstraint"),
					this, SLOT(aspectRatioConstraint_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("Graphics/bilinearFilter"),
					this, SLOT(bilinearFilter_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("Graphics/stretchMode"),
					this, SLOT(stretchMode_changed_slot(QVariant)));
	
	// Initialize the paused setting.
	m_paused.data = 0;

	// Initialize the OSD settings.
	m_cfg_osdFpsEnabled = gqt4_cfg->get(QLatin1String("OSD/fpsEnabled")).toBool();
	m_cfg_osdFpsColor   = gqt4_cfg->get(QLatin1String("OSD/fpsColor")).value<QColor>();
	if (!m_cfg_osdFpsColor.isValid())
		m_cfg_osdFpsColor = QColor(Qt::white);
	m_cfg_osdMsgEnabled = gqt4_cfg->get(QLatin1String("OSD/msgEnabled")).toBool();
	m_cfg_osdMsgColor   = gqt4_cfg->get(QLatin1String("OSD/msgColor")).value<QColor>();
	if (!m_cfg_osdMsgColor.isValid())
		m_cfg_osdMsgColor = QColor(Qt::white);

	// Mark the OSD as dirty initially.
	setOsdListDirty();

	/** OSD settings: Signals. **/
	// TODO: Reconnect signals if ConfigStore is deleted/recreated?
	gqt4_cfg->registerChangeNotification(QLatin1String("OSD/fpsEnabled"),
					this, SLOT(osdFpsEnabled_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("OSD/fpsColor"),
					this, SLOT(osdFpsColor_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("OSD/msgEnabled"),
					this, SLOT(osdMsgEnabled_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("OSD/msgColor"),
					this, SLOT(osdMsgColor_changed_slot(QVariant)));

	// Connect the message timer to the OSD message timer slot.
	connect(&m_msgTimer, SIGNAL(timeout()),
		this, SLOT(osd_process_MsgTimer_slot()));
}

VBackend::~VBackend()
{
	// Delete internal objects.

	// Internal screen buffer.
	if (m_intScreen)
		m_intScreen->unref();

	// Source framebuffer.
	if (m_srcFb)
		m_srcFb->unref();
}

/**
 * Video Backend update function.
 * @param fb Framebuffer to use.
 * @param bpp Color depth.
 */
void VBackend::vbUpdate(const LibGens::MdFb *fb, LibGens::VdpPalette::ColorDepth bpp)
{
	// Save the framebuffer.
	if (m_srcFb != fb) {
		// Framebuffer has changed.
		if (m_srcFb)
			m_srcFb->unref();
		m_srcFb = fb;
		if (m_srcFb)
			m_srcFb->ref();
		m_mdScreenDirty = true;
	}

	// Save the color depth.
	m_srcBpp = bpp;

	// Update the video backend.
	vbUpdate_int();
}

/**
 * Set the key handler.
 * @param newKeyHandler New key handler.
 */
void VBackend::setKeyHandler(KeyHandlerQt *newKeyHandler)
{
	if (m_keyHandler) {
		// Disconnect the existing key handler's "destroyed" signal.
		disconnect(m_keyHandler, SIGNAL(destroyed()),
			   this, SLOT(keyHandlerDestroyed()));
	}

	m_keyHandler = newKeyHandler;
	if (m_keyHandler) {
		// Connect the new key handler's "destroyed" signal.
		connect(m_keyHandler, SIGNAL(destroyed()),
			this, SLOT(keyHandlerDestroyed()));
	}
}

/**
 * Key handler was destroyed.
 */
void VBackend::keyHandlerDestroyed(void)
{
	m_keyHandler = nullptr;
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
	if ((pause_manual_changed && m_cfg_pauseTint) || osdFpsEnabled()) {
		// Update the video backend.
		if (isRunning()) {
			setVbDirty();
			if (osdFpsEnabled())
				setOsdListDirty();
			vbUpdate_int(); // TODO: Do we really need to call this here?
		}
	}

	// If emulation isn't running or emulation is paused, start the message timer.
	if (!isRunning() || isPaused())
		m_msgTimer.start(MSGTIMER_INTERVAL);
}

/**
 * Stretch mode setting has changed.
 * @param newStretchMode (int) New stretch mode setting.
 */
void VBackend::stretchMode_changed_slot(const QVariant &newStretchMode)
{
	m_cfg_stretchMode = (StretchMode_t)newStretchMode.toInt();

	// Print a message to the OSD.
	//: OSD message indicating the Stretch Mode has been changed.
	QString msg = tr("Stretch Mode set to %1.", "osd-stretch");
	switch (m_cfg_stretchMode) {
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

	osd_printqs(1500, msg, true);
}

/**
 * Fast Blur effect has changed.
 * @param newFastBlur (bool) New Fast Blur effect setting.
 */
void VBackend::fastBlur_changed_slot(const QVariant &newFastBlur)
{
	// Save the new Fast Blur setting.
	m_cfg_fastBlur = newFastBlur.toBool();

	// Print a message to the OSD.
	QString msg;
	if (m_cfg_fastBlur) {
		//: OSD message indicating Fast Blur has been enabled.
		msg = tr("Fast Blur enabled.", "osd");
	} else {
		//: OSD message indicating Fast Blur has been disabled.
		msg = tr("Fast Blur disabled.", "osd");
	}

	osd_printqs(1500, msg, true);
}


/**
 * Aspect ratio constraint setting has changed.
 * @param newAspectRatioConstraint (bool) New aspect ratio constraint setting.
 */
void VBackend::aspectRatioConstraint_changed_slot(const QVariant &newAspectRatioConstraint)
{
	// Save the new Aspect Ratio Constraint setting.
	m_cfg_aspectRatioConstraint = newAspectRatioConstraint.toBool();

	// Aspect Ratio Constraint setting has changed.
	m_aspectRatioConstraint_changed = true;

	// Update the Video Backend even when not running.
	setVbDirty();

	// TODO: Only if paused, or regardless of pause?
	if (!isRunning() || isPaused())
		vbUpdate_int();
}

/**
 * Bilinear Filter setting has changed.
 * @param newBilinearFilter (bool) New bilinear filter setting.
 */
void VBackend::bilinearFilter_changed_slot(const QVariant &newBilinearFilter)
{
	// Save the new Bilinear Filter setting.
	m_cfg_bilinearFilter = newBilinearFilter.toBool();

	// TODO: Only if paused, or regardless of pause?
	if (!isRunning() || isPaused()) {
		setVbDirty();
		vbUpdate_int();
	}
}

/**
 * Pause Tint effect setting has changed.
 * @param newPauseTint (bool) New pause tint effect setting.
 */
void VBackend::pauseTint_changed_slot(const QVariant &newPauseTint)
{
	// Save the new Pause Tint effect setting.
	m_cfg_pauseTint = newPauseTint.toBool();

	// Update the video backend if emulation is running,
	// and if we're currently paused manually.
	if (isRunning() && isManualPaused()) {
		setVbDirty();
		vbUpdate_int();
	}
}

/**
 * Update the Paused effect.
 * @param fromMdScreen If true, copies MD_Screen[] to m_intScreen.
 */
void VBackend::updatePausedEffect(bool fromMdScreen)
{
	// Use LibGens' software paused effect function.
	LibGens::PausedEffect::DoPausedEffect(m_intScreen, fromMdScreen);

	// Mark the video backend as dirty.
	setVbDirty();
}

/**
 * Update the Fast Blur effect.
 * @param fromMdScreen If true, copies MD_Screen[] to m_intScreen.
 */
void VBackend::updateFastBlur(bool fromMdScreen)
{
	// Use LibGens' software paused effect function.
	LibGens::FastBlur::DoFastBlur(m_intScreen, fromMdScreen);

	// Mark the video backend as dirty.
	setVbDirty();
}

/**
 * setEmuContext(): Set the emulation context for this video backend.
 * @param newEmuContext New emulation context.
 */
void VBackend::setEmuContext(LibGens::EmuContext *newEmuContext)
{
	// Lock m_emuContext while updating.
	QMutexLocker lockEmuContext(&m_mtxEmuContext);
	if (m_emuContext == newEmuContext)
		return;
	m_emuContext = newEmuContext;
	lockEmuContext.unlock();

	// setEmuContext() replaces setRunning().
	// Mark the OSD list as dirty if the FPS counter is visible.
	if (osdFpsEnabled())
		setOsdListDirty();

	setVbDirty();
	setMdScreenDirty();

	// TODO: Should we update the video buffer here?
	// Updated if there's no emulation contextor if emulation is paused.
	if (!newEmuContext || isPaused())
		vbUpdate_int();

	// If there's no emulation context or emulation is paused,
	// start the message timer.
	if (!newEmuContext || isPaused())
		m_msgTimer.start(MSGTIMER_INTERVAL);
}

}
