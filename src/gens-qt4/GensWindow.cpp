/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensWindow.cpp: Gens Window.                                            *
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

// gens-qt4 classes.
#include "actions/GensMenuBar.hpp"


namespace GensQt4
{

/**
 * GensWindow(): Initialize the Gens window.
 */
GensWindow::GensWindow()
{
	m_scale = 1;		// Set the scale to 1x by default.
	m_hasInitResize = false;
	
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
	
	// Retranslate the UI.
	retranslateUi();
	
	// Connect slots by name.
	QMetaObject::connectSlotsByName(this);
	
	// Create the menubar.
	m_menubar = new GensMenuBar(this);
	this->setMenuBar(m_menubar);
	
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
	// TODO: Figure out a way to do this without RTTI.
	QWidget *vbackend_widget = m_vBackend->toQWidget();
	if (vbackend_widget != NULL)
	{
		layout->addWidget(vbackend_widget);
	}
	else
	{
		// Not a QWidget!
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Windowed mode: VBackend is not a QWidget!");
	}
	
	// Enable drag and drop.
	setAcceptDrops(true);
	
	// Connect the GensMenuBar's triggered() signal.
	connect(m_menubar, SIGNAL(triggered(int)),
		m_gensActions, SLOT(doAction(int)));
	
	// Connect Emulation Manager signals to GensWindow.
	// TODO: Make m_emuManager a pointer instead of an object?
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
	connect(m_gensActions, SIGNAL(actionTogglePaused(void)),
		m_emuManager, SLOT(pauseRequest(void)));
	connect(m_gensActions, SIGNAL(actionResetEmulator(bool)),
		m_emuManager, SLOT(resetEmulator(bool)));
	
	// Application Focus Changed signal.
	connect(gqt4_app, SIGNAL(focusChanged(QWidget*, QWidget*)),
		this, SLOT(qAppFocusChanged(QWidget*, QWidget*)));
	
	// Retranslate the UI.
	retranslateUi();
}


/**
 * retranslateUi(): Retranslate the User Interface.
 */
void GensWindow::retranslateUi(void)
{
	// Set the Gens title.
	setGensTitle();
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
	// TODO: Check the event type.
	((void)event);
	
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
	if (m_menubar)
		win_height += m_menubar->size().height();
	
	// Set the new window size.
	this->resize(img_width, win_height);
}


/** Slots. **/


/**
 * setGensTitle(): Set the Gens window title.
 */
void GensWindow::setGensTitle(void)
{
	// TODO: Indicate UI status.
	QString title = tr("Gens/GS II");
	title += QChar(L' ') + tr("dev");
#if !defined(GENS_ENABLE_EMULATION)
	title += QChar(L' ') + tr("NO-EMU");
#endif
	title += QString::fromLatin1(" - ");
	
	// TODO
	if (!m_emuManager->isRomOpen())
	{
		// No ROM is running.
		title += tr("Idle");
	}
	else
	{
		// ROM is running.
		title += m_emuManager->sysName() + QString::fromLatin1(": ");
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
	switch (osd_type)
	{
		case OSD_SRAM_LOAD:
			m_vBackend->osd_printf(1500, "SRAM loaded. (%d bytes)", param);
			break;
		
		case OSD_SRAM_SAVE:
			m_vBackend->osd_printf(1500, "SRAM saved. (%d bytes)", param);
			break;
		
		case OSD_SRAM_AUTOSAVE:
			m_vBackend->osd_printf(1500, "SRAM autosaved. (%d bytes)", param);
			break;
		
		case OSD_EEPROM_LOAD:
			m_vBackend->osd_printf(1500, "EEPROM loaded. (%d bytes)", param);
			break;
		
		case OSD_EEPROM_SAVE:
			m_vBackend->osd_printf(1500, "EEPROM saved. (%d bytes)", param);
			break;
		
		case OSD_EEPROM_AUTOSAVE:
			m_vBackend->osd_printf(1500, "EEPROM autosaved. (%d bytes)", param);
			break;
		
		default:
			// Unknown OSD type.
			break;
	}
}


/**
 * setBpp(): Set color depth.
 * TODO: Should this be a public function or a slot?
 * @param bpp Color depth.
 */
void GensWindow::setBpp(LibGens::VdpPalette::ColorDepth bpp)
{
	// TODO: bpp changes should be pushed to the emulation queue.
	// TODO: Maybe this should be a slot called by GensConfig.
	int bppVal;
	switch (bpp)
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
	LibGens::VdpRend::m_palette.setBpp(bpp);
	m_vBackend->setVbDirty();
	m_vBackend->vbUpdate();
	
	QString msg = tr("Color depth set to %1-bit.").arg(bppVal);
	m_vBackend->osd_printf(1500, "%s", msg.toUtf8().constData());
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
		m_vBackend->setPaused(m_emuManager->isPaused());
	}
	else
	{
		// ROM is closed.
		m_vBackend->osd_show_preview(0, QImage());
		m_vBackend->setRunning(false);
		m_vBackend->setPaused(false);
		m_vBackend->resetFps();
	}
	
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
	bool paused = true;
	
	if (now != NULL)
	{
		if (now == m_vBackend->toQWidget() ||
		    now == m_menubar ||
		    now == this)
		{
			// Window has focus.
			paused = false;
		}
	}
	
	// Send the pause request.
	if (paused != m_emuManager->isPaused())
		m_emuManager->pauseRequest(paused);
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

}
