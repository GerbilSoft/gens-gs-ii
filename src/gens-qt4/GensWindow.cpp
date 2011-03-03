/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensWindow.cpp: Gens Window.                                            *
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

#include <config.h>

#include "GensWindow.hpp"
#include "gqt4_main.hpp"

// Menu definitions.
#include "actions/GensMenuBar_menus.hpp"

// Qt includes.
#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtGui/QCloseEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QVBoxLayout>

// GensQApplication.hpp is needed in order to connect signals from gqt4_app.
// Otherwise, g++ won't realize it inherits from QObject.
#include "GensQApplication.hpp"

// LibGens includes.
#include "libgens/macros/log_msg.h"

// Video Backend classes.
#include "VBackend/GensQGLWidget.hpp"

namespace GensQt4
{

/**
 * GensWindow(): Initialize the Gens window.
 */
GensWindow::GensWindow()
{
	m_scale = 1;			// Set the scale to 1x by default.
	m_hasInitResize = false;	// Initial resize hasn't occurred yet.
	m_idleThread = NULL;		// Clear the idle thread.
	m_idleThreadAllowed = false;	// Not allowed yet.
	
	// Initialize the Emulation Manager.
	m_emuManager = new EmuManager();
	
	// Initialize the Gens Action Manager.
	// TODO: Load configuration from a file?
	m_gensActions = new GensActions(this);
	
	// Initialize KeyHandlerQt.
	// TODO: Make KeyHandlerQt a standard object?
	KeyHandlerQt::Init(m_gensActions);
	
	// Set up the User Interface.
	setupUi();
	
	// Make sure all user configuration settings are applied.
	// (Lock the OSD first to prevent random messages from appearing.)
	m_vBackend->osd_lock();
	gqt4_config->emitAll();
	m_vBackend->osd_unlock();
	
	// Initialize the emulation state.
	m_idleThreadAllowed = true;
	stateChanged();
	
#ifndef Q_WS_MAC
	// Enable the context menu.
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
		this, SLOT(showContextMenu(const QPoint&)));
	setContextMenuPolicy(Qt::CustomContextMenu);
#endif
}


/**
 * ~GensWindow(): Free all resources acquired by the Gens window.
 */
GensWindow::~GensWindow()
{
	// Shut down KeyHandlerQt.
	// TODO: Make KeyHandlerQt a standard object?
	KeyHandlerQt::End();
	
	// Delete the Gens Actions Manager.
	delete m_gensActions;
	
	// Delete the Emulation Manager.
	delete m_emuManager;
	
	// Save configuration.
	// TODO: Figure out a better place to put this.
	// TODO: Config autosave.
	gqt4_config->save();
}


/**
 * setupUi(): Set up the User Interface.
 */
void GensWindow::setupUi(void)
{
	if (this->objectName().isEmpty())
		this->setObjectName(QString::fromUtf8("GensWindow"));
	
#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif
	
	// Create the central widget.
	centralwidget = new QWidget(this);
	centralwidget->setObjectName(QString::fromLatin1("centralwidget"));
	this->setCentralWidget(centralwidget);
	
	// Connect slots by name.
	QMetaObject::connectSlotsByName(this);
	
	// Create the menubar.
	m_gensMenuBar = new GensMenuBar(this, m_emuManager);
	this->setMenuBar(m_gensMenuBar->createMenuBar());
	
	// Create the Video Backend.
	// TODO: Allow selection of all available VBackend classes.
	m_vBackend = new GensQGLWidget(this->centralwidget);
	
	// Create the layout.
	layout = new QVBoxLayout(this->centralwidget);
	layout->setObjectName(QString::fromLatin1("layout"));
	layout->setMargin(0);
	layout->setSpacing(0);
	centralwidget->setLayout(layout);
	
	// Add the Video Backend to the layout.
	layout->addWidget(m_vBackend);
	
	// Enable drag and drop.
	setAcceptDrops(true);
	
	// Connect the GensMenuBar's triggered() signal.
	connect(m_gensMenuBar, SIGNAL(triggered(int, bool)),
		m_gensActions, SLOT(doAction(int, bool)));
	
	// Connect Emulation Manager signals to GensWindow.
	connect(m_emuManager, SIGNAL(updateFps(double)),
		this, SLOT(updateFps(double)));
	connect(m_emuManager, SIGNAL(stateChanged(void)),
		this, SLOT(stateChanged(void)));
	connect(m_emuManager, SIGNAL(updateVideo(void)),
		this, SLOT(updateVideo(void)));
	connect(m_emuManager, SIGNAL(osdPrintMsg(int, const QString&)),
		this, SLOT(osdPrintMsg(int, const QString&)));
	connect(m_emuManager, SIGNAL(osdShowPreview(int, const QImage&)),
		this, SLOT(osdShowPreview(int, const QImage&)));
	
	// Gens Action Manager signals.
	connect(m_gensActions, SIGNAL(actionSetPaused(bool)),
		m_emuManager, SLOT(pauseRequest(bool)));
	connect(m_gensActions, SIGNAL(actionResetEmulator(bool)),
		m_emuManager, SLOT(resetEmulator(bool)));
	connect(m_gensActions, SIGNAL(actionResetCpu(int)),
		m_emuManager, SLOT(resetCpu(int)));
	
	// Auto Pause: Application Focus Changed signal, and setting change signal.
	connect(gqt4_app, SIGNAL(focusChanged(QWidget*, QWidget*)),
		this, SLOT(qAppFocusChanged(QWidget*, QWidget*)));
	connect(gqt4_config, SIGNAL(autoPause_changed(bool)),
		this, SLOT(autoPause_changed_slot(bool)));
	
	// Intro Style Changed signal.
	connect(gqt4_config, SIGNAL(introStyle_changed(int)),
		this, SLOT(introStyle_changed_slot(int)));
	
	// Show Menu Bar Changed signal.
	connect(gqt4_config, SIGNAL(showMenuBar_changed(bool)),
		this, SLOT(showMenuBar_changed_slot(bool)));
}


/**
 * closeEvent(): Window is being closed.
 * @param event Close event.
 */
void GensWindow::closeEvent(QCloseEvent *event)
{
	// Remove the Application Focus Changed signal to
	// prevent segmentation faults.
	// (e.g. caused by a signal being received after the
	//  GensWindow has been deleted)
	disconnect(gqt4_app, SIGNAL(focusChanged(QWidget*, QWidget*)),
		   this, SLOT(qAppFocusChanged(QWidget*, QWidget*)));
	
	// Quit.
	m_emuManager->closeRom();
	QuitGens();
	
	// Accept the close event.
	event->accept();
}


/**
 * showEvent(): Window is being shown.
 * @param event Show event.
 */
void GensWindow::showEvent(QShowEvent *event)
{
	// Event type isn't useful here.
	((void)event);
	
	// If we've already run the initial resize, don't run it again.
	if (m_hasInitResize)
		return;
	
	// Run the initial resize.
	m_hasInitResize = true;
	gensResize();
}


/**
 * dragEnterEvent(): An item is being dragged onto the window.
 * @param event QDragEnterEvent describing the item.
 */
void GensWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;
	
	// One or more URLs have been dragged onto the window.
	const QList<QUrl>& lstUrls = event->mimeData()->urls();
	if (lstUrls.size() != 1)
	{
		// More than one file dragged onto the window.
		// We only accept one at a time.
		return;
	}
	
	// One URL has been dropped.
	const QUrl& url = lstUrls.at(0);
	
	// Make sure the URL is file://.
	// TODO: Add support for other protocols later.
	if (url.scheme() != QString::fromLatin1("file"))
		return;
	
	// Override the propsed action with Copy, and accept it.
	event->setDropAction(Qt::CopyAction);
	event->accept();
}


/**
 * dropEvent(): An item has been dropped onto the window.
 * @param event QDropEvent describing the item.
 */
void GensWindow::dropEvent(QDropEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;
	
	// One or more URLs have been dragged onto the window.
	const QList<QUrl>& lstUrls = event->mimeData()->urls();
	if (lstUrls.size() != 1)
	{
		// More than one file dragged onto the window.
		// We only accept one at a time.
		return;
	}
	
	// One URL has been dropped.
	const QUrl& url = lstUrls.at(0);
	
	// Make sure the URL is file://.
	// TODO: Add support for other protocols later.
	if (url.scheme() != QString::fromLatin1("file"))
		return;
	
	// Get the local filename.
	// NOTE: url.toLocalFile() returns an empty string if this isn't file://,
	// but we're already checking for file:// above...
	QString filename = url.toLocalFile();
	if (filename.isEmpty())
		return;
	
	// Override the propsed action with Copy, and accept it.
	event->setDropAction(Qt::CopyAction);
	event->accept();
	
	// Convert the filename to native separators.
	filename = QDir::toNativeSeparators(filename);
	
	// Open the ROM.
	m_emuManager->openRom(filename);
}


/**
 * rescale(): Rescale the window.
 * @param scale New scale value.
 */
void GensWindow::rescale(int scale)
{
	if (scale <= 0 || scale > 8)
		return;
	m_scale = scale;
	gensResize();
}


/**
 * gensResize(): Resize the Gens window to show the image at its expected size.
 */
void GensWindow::gensResize(void)
{
	// Get the drawing size.
	// TODO: Scale for larger renderers.
	int img_width = 320 * m_scale;
	int img_height = 240 * m_scale;
	
	// Enforce a minimum size of 320x240.
	if (img_width < 320)
		img_width = 320;
	if (img_height < 240)
		img_height = 240;
	
	// Calculate the window height.
	int win_height = img_height;
	if (this->menuWidget())
		win_height += this->menuWidget()->size().height();
	
	// Set the new window size.
	this->resize(img_width, win_height);
}


/** Slots. **/


/**
 * setGensTitle(): Set the Gens window title.
 */
void GensWindow::setGensTitle(void)
{
	// Update the window title to reflect the emulation status.
	QString title = tr("Gens/GS II");
	title += QChar(L' ') + tr("dev");
#if !defined(GENS_ENABLE_EMULATION)
	title += QChar(L' ') + tr("NO-EMU");
#endif
	title += QLatin1String(" - ");
	
	if (!m_emuManager->isRomOpen())
	{
		// No ROM is running.
		title += tr("Idle");
	}
	else
	{
		// ROM is running.
		title += m_emuManager->sysName() + QLatin1String(": ");
		title += m_emuManager->romName();
	}
	
	this->setWindowTitle(title);
}


/**
 * osd(): LibGens OSD handler.
 * @param osd_type: OSD type.
 * @param param: Integer parameter.
 */
void GensWindow::osd(OsdType osd_type, int param)
{
	QString msg;
	switch (osd_type)
	{
		case OSD_SRAM_LOAD:
			msg = tr("SRAM loaded. (%n byte(s))", "Onscreen Display", param);
			break;
		case OSD_SRAM_SAVE:
			msg = tr("SRAM saved. (%n byte(s))", "Onscreen Display", param);
			break;
		case OSD_SRAM_AUTOSAVE:
			msg = tr("SRAM autosaved. (%n byte(s))", "Onscreen Display", param);
			break;
		case OSD_EEPROM_LOAD:
			msg = tr("EEPROM loaded. (%n byte(s))", "Onscreen Display", param);
			break;
		case OSD_EEPROM_SAVE:
			msg = tr("EEPROM saved. (%n byte(s))", "Onscreen Display", param);
			break;
		case OSD_EEPROM_AUTOSAVE:
			msg = tr("EEPROM autosaved. (%n byte(s))", "Onscreen Display", param);
			break;
		default:
			// Unknown OSD type.
			break;
	}
	
	if (msg.isEmpty())
		return;
	
	// Print the message to the screen.
	m_vBackend->osd_printqs(1500, msg);
}


/**
 * setBpp(): Set color depth.
 * TODO: Should this be a public function or a slot?
 * @param newBpp Color depth.
 */
void GensWindow::setBpp(LibGens::VdpPalette::ColorDepth newBpp)
{
	// TODO: bpp changes should be pushed to the emulation queue.
	// TODO: Maybe this should be a slot called by GensConfig.
	if (LibGens::VdpRend::m_palette.bpp() == newBpp)
		return;
	
	int bppVal;
	switch (newBpp)
	{
		case LibGens::VdpPalette::BPP_15:
			bppVal = 15;
			break;
		case LibGens::VdpPalette::BPP_16:
			bppVal = 16;
			break;
		case LibGens::VdpPalette::BPP_32:
			bppVal = 32;
			break;
		default:
			return;
	}
	
	// Set the color depth.
	LibGens::VdpRend::m_palette.setBpp(newBpp);
	m_vBackend->setVbDirty();
	//m_vBackend->vbUpdate();	// TODO: Don't update immediately?
	
	const QString msg = tr("Color depth set to %1-bit.").arg(bppVal);
	m_vBackend->osd_printqs(1500, msg);
}


/**
 * stateChanged(): Emulation state changed.
 * - Update the video backend properties.
 * - Update the Gens title.
 */
void GensWindow::stateChanged(void)
{
	if (m_emuManager->isRomOpen())
	{
		// ROM is open.
		m_vBackend->setRunning(m_emuManager->isRomOpen());
		m_vBackend->setPaused(m_emuManager->paused());
	}
	else
	{
		// ROM is closed.
		paused_t unPause;
		unPause.data = 0;
		
		m_vBackend->osd_show_preview(0, QImage());
		m_vBackend->setRunning(false);
		m_vBackend->setPaused(unPause);
		m_vBackend->resetFps();
	}
	
	// Check the idle thread state.
	checkIdleThread();
	
	// Update the Gens window title.
	setGensTitle();
}


/**
 * qAppFocusChanged(): Application focus has changed.
 * @param old Old widget.
 * @param now New widget.
 */
void GensWindow::qAppFocusChanged(QWidget *old, QWidget *now)
{
	if (!gqt4_config->autoPause() ||
	    !m_emuManager->isRomOpen())
	{
		// Auto Pause is disabled,
		// or no ROM is running.
		return;
	}
	
	((void)old);
	
	// Assume window doesn't have focus by default.
	bool paused_auto = true;
	
	// Check if the currently-focused widget is a child widget of GensWindow.
	if (this->isAncestorOf(now))
	{
		// Window has focus.
		paused_auto = false;
	}
	
	// Send the pause request.
	paused_t paused_set;
	paused_t paused_clear;
	paused_set.data = 0;
	paused_clear.data = 0;
	if (paused_auto)
		paused_set.paused_auto = 1;
	else
		paused_clear.paused_auto = 1;
	
	m_emuManager->pauseRequest(paused_set, paused_clear);
}


/**
 * autoPause_changed_slot(): Auto Pause setting has changed.
 * @param newAutoPause New Auto Pause setting.
 */
void GensWindow::autoPause_changed_slot(bool newAutoPause)
{
	if (newAutoPause)
	{
		// Auto Pause is enabled.
		qAppFocusChanged(NULL, gqt4_app->focusWidget());
	}
	else
	{
		// Auto Pause is disabled.
		// Undo auto pause.
		paused_t paused_set;
		paused_t paused_clear;
		paused_set.data = 0;
		paused_clear.data = 0;
		paused_clear.paused_auto = 1;
		
		m_emuManager->pauseRequest(paused_set, paused_clear);
	}
}


/**
 * showMenuBar_changed_slot(): Show Menu Bar setting has changed.
 * @param newShowMenuBar New Show Menu Bar setting.
 */
void GensWindow::showMenuBar_changed_slot(bool newShowMenuBar)
{
	if (!newShowMenuBar)
	{
		// Hide the menu bar.
		this->setMenuBar(NULL);
	}
	else if (!this->menuWidget())
	{
		// Show the menu bar.
		this->setMenuBar(m_gensMenuBar->createMenuBar());
	}
}


/** Wrapper functions for GensActions. **/
/** TODO: Have GensActions emit signals, and link them to EmuManager slots. **/

void GensWindow::openRom(void)
	{ m_emuManager->openRom(this); }
void GensWindow::closeRom(void)
	{ m_emuManager->closeRom(); }
void GensWindow::saveState(void)
	{ m_emuManager->saveState(); }
void GensWindow::loadState(void)
	{ m_emuManager->loadState(); }
void GensWindow::screenShot(void)
	{ m_emuManager->screenShot(); }
void GensWindow::setController(int port, LibGens::IoBase::IoType type)
	{ m_emuManager->setController(port, type); }
void GensWindow::setAudioRate(int newRate)
	{ m_emuManager->setAudioRate(newRate); }
void GensWindow::setStereo(bool newStereo)
	{ m_emuManager->setStereo(newStereo); }


void GensWindow::setIdleThreadAllowed(bool newIdleThreadAllowed)
{
	m_idleThreadAllowed = newIdleThreadAllowed;
	checkIdleThread();
}


void GensWindow::checkIdleThread(void)
{
	if (m_emuManager->isRomOpen() ||
		!m_idleThreadAllowed ||
		gqt4_config->introStyle() == 0)
	{
		// Make sure the idle thread isn't running.
		// TODO: Check for race conditions.
		if (m_idleThread)
		{
			m_idleThread->stop();
			m_idleThread->wait();
			delete m_idleThread;
			m_idleThread = NULL;
		}
	}
	else
	{
		// Make sure the idle thread is running.
		// TODO: Check for race conditions.
		if (!m_idleThread || !m_idleThread->isRunning())
		{
			// Idle thread isn't running.
			delete m_idleThread;	// if it exists but isn't running
			m_idleThread = new IdleThread(this);
			connect(m_idleThread, SIGNAL(frameDone()),
				this, SLOT(idleThread_frameDone()));
			
			// Start the idle thread.
			m_idleThread->start();
		}
	}
}


/**
 * idleThread_frameDone(): The Idle thread is finished rendering a frame.
 */
void GensWindow::idleThread_frameDone(void)
{
	// Make sure the idle thread is still running.
	if (!m_idleThread || m_idleThread->isStopRequested())
		return;
	if (m_emuManager->isRomOpen())
		return;
	
	// Update video.
	emit updateVideo();
	
	// Resume the idle thread.
	m_idleThread->resume();
}


/**
 * introStyle_changed_slot(): Intro Style setting has changed.
 * @param newIntroStyle New Intro Style setting.
 */
void GensWindow::introStyle_changed_slot(int newIntroStyle)
{
	checkIdleThread();
	
	// Prevent race conditions.
	if (!m_emuManager)
		return;
	
	if (!m_emuManager->isRomOpen() && newIntroStyle == 0)
	{
		// Intro style was changed to "None", and emulation isn't running.
		// Clear the screen.
		LibGens::VdpIo::Reset();
		updateVideo();
	}
}


/**
 * showContextMenu(): Show the context menu.
 * @param pos Position to show the context menu. (widget coordinates)
 */
void GensWindow::showContextMenu(const QPoint& pos)
{
#ifdef Q_WS_MAC
	// Mac OS X has a global menu bar, so this isn't necessary.
	// TODO: Enable the context menu in full-screen mode?
	((void)pos);
	return;
#else /* !Q_WS_MAC */
	// Don't do anything if the menu bar is visible.
	if (this->menuWidget() != NULL)
		return;
	
	QPoint globalPos = this->mapToGlobal(pos);
	m_gensMenuBar->popupMenu()->popup(globalPos);
#endif
}

}
