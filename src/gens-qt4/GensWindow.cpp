/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GensWindow.cpp: Gens Window.                                               *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2011 by David Korth.                                    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software                *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA *
 ******************************************************************************/

#include <gens-qt4/config.gens-qt4.h>

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
#include "libgens/Vdp/Vdp.hpp"

// Video Backend classes.
#include "VBackend/GensQGLWidget.hpp"

namespace GensQt4
{

class GensWindowPrivate
{
	public:
		GensWindowPrivate(GensWindow *q);
		~GensWindowPrivate();
	
	private:
		GensWindow *const q;
		Q_DISABLE_COPY(GensWindowPrivate)
	
	public:
		void setupUi(void);

		// Key handler.
		KeyHandlerQt *keyHandler;

		// Widgets.
		VBackend *vBackend;		// GensQGLWidget.

		// Menu bar.
		GensMenuBar *gensMenuBar;
		bool isGlobalMenuBar(void) const;
		bool isShowMenuBar(void) const;
		void initMenuBar(void);

		QWidget *centralwidget;
		QVBoxLayout *layout;

		int scale;		// Temporary scaling variable.
		bool hasInitResize;	// Has the initial resize occurred?

		// Resize the window.
		void gensResize(void);

		// Set the Gens window title.
		void setGensTitle(void);

		// Emulation Manager.
		EmuManager *emuManager;

		// Actions manager.
		GensActions *gensActions;

		/** Configuration items. **/
		bool cfg_autoPause;
		int cfg_introStyle;
		bool cfg_showMenuBar;

		/** Idle thread. **/
		IdleThread *idleThread;
		bool idleThreadAllowed;
		void checkIdleThread(void);
};


/********************************
 * GensWindowPrivate functions. *
 ********************************/

GensWindowPrivate::GensWindowPrivate(GensWindow *q)
	: q(q)
	, scale(1)			// Set the scale to 1x by default.
	, hasInitResize(false)		// Initial resize hasn't occurred yet.
	, idleThread(NULL)
	, idleThreadAllowed(false)	// Not allowed yet.
{
	/** Configuration items. **/
	cfg_autoPause = gqt4_cfg->get(QLatin1String("autoPause")).toBool();
	cfg_introStyle = gqt4_cfg->getInt(QLatin1String("Intro_Effect/introStyle"));
	cfg_showMenuBar = gqt4_cfg->get(QLatin1String("GensWindow/showMenuBar")).toBool();

	// Initialize the Emulation Manager.
	emuManager = new EmuManager();

	// Initialize the Gens Action Manager.
	gensActions = new GensActions(q);

	// Initialize KeyHandlerQt.
	keyHandler = new KeyHandlerQt(q, gensActions);

	// Create the GensMenuBar.
	gensMenuBar = new GensMenuBar(q, emuManager);
}

GensWindowPrivate::~GensWindowPrivate()
{
	delete keyHandler;
	delete gensActions;
	delete emuManager;
}


/**
 * Set up the User Interface.
 */
void GensWindowPrivate::setupUi(void)
{
	if (q->objectName().isEmpty())
		q->setObjectName(QLatin1String("GensWindow"));

#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	// Create the central widget.
	centralwidget = new QWidget(q);
	centralwidget->setObjectName(QLatin1String("centralwidget"));
	q->setCentralWidget(centralwidget);

	// Connect slots by name.
	QMetaObject::connectSlotsByName(q);

	// Initialize the menu bar.
	initMenuBar();

	// Create the Video Backend.
	// TODO: Allow selection of all available VBackend classes.
	vBackend = new GensQGLWidget(centralwidget, keyHandler);
	emuManager->setVBackend(vBackend);

	// Create the layout.
	layout = new QVBoxLayout(centralwidget);
	layout->setObjectName(QLatin1String("layout"));
	layout->setMargin(0);
	layout->setSpacing(0);
	centralwidget->setLayout(layout);

	// Add the Video Backend to the layout.
	layout->addWidget(vBackend);

	// Enable drag and drop.
	q->setAcceptDrops(true);

	// Connect the GensMenuBar's triggered() signal.
	QObject::connect(gensMenuBar, SIGNAL(triggered(int,bool)),
		gensActions, SLOT(doAction(int,bool)));

	// Connect Emulation Manager signals to GensWindow.
	QObject::connect(emuManager, SIGNAL(updateFps(double)),
		q, SLOT(updateFps(double)));
	QObject::connect(emuManager, SIGNAL(stateChanged(void)),
		q, SLOT(stateChanged(void)));
	QObject::connect(emuManager, SIGNAL(osdPrintMsg(int,QString)),
		q, SLOT(osdPrintMsg(int,QString)));
	QObject::connect(emuManager, SIGNAL(osdShowPreview(int,QImage)),
		q, SLOT(osdShowPreview(int,QImage)));

	// Gens Action Manager signals.
	QObject::connect(gensActions, SIGNAL(actionSetPaused(bool)),
		emuManager, SLOT(pauseRequest(bool)));
	QObject::connect(gensActions, SIGNAL(actionResetEmulator(bool)),
		emuManager, SLOT(resetEmulator(bool)));
	QObject::connect(gensActions, SIGNAL(actionResetCpu(int)),
		emuManager, SLOT(resetCpu(int)));

	// Auto Pause: Application Focus Changed signal, and setting change signal.
	QObject::connect(gqt4_app, SIGNAL(focusChanged(QWidget*,QWidget*)),
		q, SLOT(qAppFocusChanged(QWidget*,QWidget*)));

	/** Configuration items: Signals. **/
	gqt4_cfg->registerChangeNotification(QLatin1String("autoPause"),
					q, SLOT(autoPause_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("Intro_Effect/introStyle"),
					q, SLOT(introStyle_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("GensWindow/showMenuBar"),
					q, SLOT(showMenuBar_changed_slot(QVariant)));
}


/**
 * Do we have a global menu bar?
 * @return True if yes; false if no.
 */
inline bool GensWindowPrivate::isGlobalMenuBar(void) const
{
	// TODO: Support Unity and Qt global menu bars.
#ifdef Q_WS_MAC
	return true;
#else
	return false;
#endif
}

/**
 * Is the menu bar visible?
 * @return True if yes; false if no.
 */
inline bool GensWindowPrivate::isShowMenuBar(void) const
{
	return (isGlobalMenuBar() || cfg_showMenuBar);
}

/**
 * Initialize the menu bar.
 */
void GensWindowPrivate::initMenuBar(void)
{
	// TODO: If the value changed and we're windowed,
	// resize the window to compensate.
	int height_adjust = 0;
	if (!isShowMenuBar()) {
		// Hide the menu bar.
		if (!q->isMaximized() && !q->isMinimized()) {
			QWidget *menuBar = q->menuWidget();
			if (menuBar != NULL)
				height_adjust = -menuBar->height();
		}
		if (!isGlobalMenuBar())
			q->setMenuBar(NULL);
	} else {
		// Check if the menu bar was there already.
		const bool wasMenuBarThere = !!(q->menuWidget());

		// Show the menu bar.
		QMenuBar *menuBar = q->menuBar();
		gensMenuBar->createMenuBar(menuBar);

		if (!wasMenuBarThere && !q->isMaximized() && !q->isMinimized()) {
			menuBar->adjustSize();	// ensure the menu bar gets the correct size
			height_adjust = menuBar->height();
		}
	}

	if (!isGlobalMenuBar() && height_adjust != 0) {
		// Adjust the window height to compensate for the menu bar change.
		q->resize(q->width(), q->height() + height_adjust);
	}
}


/**
 * Resize the Gens window to show the image at its expected size.
 */
void GensWindowPrivate::gensResize(void)
{
	// Get the drawing size.
	// TODO: Scale for larger renderers.
	int img_width = 320 * scale;
	int img_height = 240 * scale;

	// Enforce a minimum size of 320x240.
	if (img_width < 320)
		img_width = 320;
	if (img_height < 240)
		img_height = 240;

	// Calculate the window height.
	int win_height = img_height;
	if (q->menuWidget())
		win_height += q->menuWidget()->size().height();

	// Set the new window size.
	q->resize(img_width, win_height);
}


/**
 * Set the Gens window title.
 */
void GensWindowPrivate::setGensTitle(void)
{
	// Update the window title to reflect the emulation status.
	QString title;
#if !defined(GENS_ENABLE_EMULATION)
	title += tr("[NO-EMU]") + QChar(L' ');
#endif

	if (!emuManager->isRomOpen()) {
		// No ROM is running.
		title += gqt4_app->applicationName();
	} else {
		// ROM is running.
		if (emuManager->paused().paused_manual) {
			// Emulator is paused manually.
			title += q->tr("[Paused]") + QChar(L' ');
		}
		title += emuManager->romName();
	}

	q->setWindowTitle(title);
}


/**
 * Chek the idle thread state.
 */
void GensWindowPrivate::checkIdleThread(void)
{
	if (emuManager->isRomOpen() ||
		!idleThreadAllowed ||
		cfg_introStyle == 0)
	{
		// Make sure the idle thread isn't running.
		// TODO: Check for race conditions.
		if (idleThread) {
			idleThread->stop();
			idleThread->wait();
			delete idleThread;
			idleThread = NULL;
		}
	} else {
		// Make sure the idle thread is running.
		// TODO: Check for race conditions.
		if (!idleThread || !idleThread->isRunning()) {
			// Idle thread isn't running.
			delete idleThread;	// if it exists but isn't running
			idleThread = new IdleThread(q);
			QObject::connect(idleThread, SIGNAL(frameDone()),
				q, SLOT(idleThread_frameDone()));

			// Start the idle thread.
			idleThread->start();
		}
	}
}


/*************************
 * GensWindow functions. *
 *************************/

/**
 * Initialize the Gens window.
 */
GensWindow::GensWindow()
	: d(new GensWindowPrivate(this))
{
	// Set up the User Interface.
	d->setupUi();

	// Initialize the emulation state.
	d->idleThreadAllowed = true;
	stateChanged();

	// Enable the context menu.
	connect(this, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(showContextMenu(QPoint)));
	setContextMenuPolicy(Qt::CustomContextMenu);
}


/**
 * Free all resources acquired by the Gens window.
 */
GensWindow::~GensWindow()
{
	delete d;
}


/**
 * Window is being closed.
 * @param event Close event.
 */
void GensWindow::closeEvent(QCloseEvent *event)
{
	// Remove the Application Focus Changed signal to
	// prevent segmentation faults.
	// (e.g. caused by a signal being received after the
	//  GensWindow has been deleted)
	disconnect(gqt4_app, SIGNAL(focusChanged(QWidget*,QWidget*)),
		   this, SLOT(qAppFocusChanged(QWidget*,QWidget*)));

	// Quit.
	d->emuManager->closeRom();
	QuitGens();

	// Accept the close event.
	event->accept();
}


/**
 * Window is being shown.
 * @param event Show event.
 */
void GensWindow::showEvent(QShowEvent *event)
{
	// Event type isn't useful here.
	Q_UNUSED(event)

	// If we've already run the initial resize, don't run it again.
	if (d->hasInitResize)
		return;

	// Run the initial resize.
	d->hasInitResize = true;
	d->gensResize();
}


/**
 * An item is being dragged onto the window.
 * @param event QDragEnterEvent describing the item.
 */
void GensWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;

	// One or more URLs have been dragged onto the window.
	const QList<QUrl>& lstUrls = event->mimeData()->urls();
	if (lstUrls.size() != 1) {
		// More than one file dragged onto the window.
		// We only accept one at a time.
		return;
	}

	// One URL has been dropped.
	const QUrl& url = lstUrls.at(0);

	// Make sure the URL is file://.
	// TODO: Add support for other protocols later.
	if (url.scheme() != QLatin1String("file"))
		return;

	// Override the propsed action with Copy, and accept it.
	event->setDropAction(Qt::CopyAction);
	event->accept();
}


/**
 * An item has been dropped onto the window.
 * @param event QDropEvent describing the item.
 */
void GensWindow::dropEvent(QDropEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;

	// One or more URLs have been dragged onto the window.
	const QList<QUrl>& lstUrls = event->mimeData()->urls();
	if (lstUrls.size() != 1) {
		// More than one file dragged onto the window.
		// We only accept one at a time.
		return;
	}

	// One URL has been dropped.
	const QUrl& url = lstUrls.at(0);

	// Make sure the URL is file://.
	// TODO: Add support for other protocols later.
	if (url.scheme() != QLatin1String("file"))
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
	d->emuManager->openRom(filename);
}


/**
 * Widget state has changed.
 * @param event State change event.
 */
void GensWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the menu bar.
		d->gensMenuBar->retranslate();
		d->initMenuBar();
	}

	// Pass the event to the base class.
	this->QMainWindow::changeEvent(event);
}


/**
 * Rescale the window.
 * @param scale New scale value.
 */
void GensWindow::rescale(int scale)
{
	if (scale <= 0 || scale > 8)
		return;
	d->scale = scale;
	d->gensResize();
}


/** Slots. **/


/**
 * LibGens OSD handler.
 * @param osd_type: OSD type.
 * @param param: Integer parameter.
 */
void GensWindow::osd(OsdType osd_type, int param)
{
	QString msg;
	switch (osd_type) {
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
	d->vBackend->osd_printqs(1500, msg);
}


/**
 * setBpp(): Set color depth.
 * TODO: Should this be a public function or a slot?
 * @param newBpp Color depth.
 */
void GensWindow::setBpp(LibGens::VdpPalette::ColorDepth newBpp)
{
	// TODO: Move to GensConfig/qEmu.
#if 0
	// TODO: bpp changes should be pushed to the emulation queue.
	// TODO: Maybe this should be a slot called by GensConfig.
	if (LibGens::Vdp::m_palette.bpp() == newBpp)
		return;

	int bppVal;
	switch (newBpp) {
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
	LibGens::Vdp::m_palette.setBpp(newBpp);
	m_vBackend->setVbDirty();
	//m_vBackend->vbUpdate();	// TODO: Don't update immediately?

	//: OSD message indicating color depth change.
	const QString msg = tr("Color depth set to %1-bit.", "osd").arg(bppVal);
	m_vBackend->osd_printqs(1500, msg);
#endif
}


/**
 * Update the FPS counter.
 */
void GensWindow::updateFps(double fps)
	{ d->vBackend->fpsPush(fps); }


/**
 * Emulation state changed.
 * - Update the video backend properties.
 * - Update the Gens title.
 */
void GensWindow::stateChanged(void)
{
	// TODO: Make sure that m_vBackend gets the new gqt4_emuContext in time.
	// FIXME: This is probably a race condition.
	// d->vBackend->setEmuContext() should be called when gqt4_emuContext changes.
	d->vBackend->setEmuContext(gqt4_emuContext);

	if (d->emuManager->isRomOpen()) {
		// ROM is open.
		d->vBackend->setPaused(d->emuManager->paused());
	} else {
		// ROM is closed.
		paused_t unPause;
		unPause.data = 0;

		d->vBackend->osd_show_preview(0, QImage());
		d->vBackend->setPaused(unPause);
		d->vBackend->fpsReset();
	}

	// Check the idle thread state.
	d->checkIdleThread();

	// Update the Gens window title.
	d->setGensTitle();
}


/**
 * Print a message on the OSD.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to print.
 */
void GensWindow::osdPrintMsg(int duration, QString msg)
	{ d->vBackend->osd_printqs(duration, msg); }

/**
 * Show a preview image on the OSD.
 * @param duration Duration for the preview image to appaer, in milliseconds.
 * @param img Image to show.
 */
void GensWindow::osdShowPreview(int duration, const QImage& img)
	{ d->vBackend->osd_show_preview(duration, img); }


/**
 * Application focus has changed.
 * @param old Old widget.
 * @param now New widget.
 */
void GensWindow::qAppFocusChanged(QWidget *old, QWidget *now)
{
	if (!d->cfg_autoPause || !d->emuManager->isRomOpen()) {
		// Auto Pause is disabled,
		// or no ROM is running.
		return;
	}

	Q_UNUSED(old)

	// Assume window doesn't have focus by default.
	bool paused_auto = true;

	// Check if the currently-focused widget is a child widget of GensWindow.
	if (this->isAncestorOf(now)) {
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

	d->emuManager->pauseRequest(paused_set, paused_clear);
}


/**
 * Auto Pause setting has changed.
 * @param newAutoPause (bool) New Auto Pause setting.
 */
void GensWindow::autoPause_changed_slot(QVariant newAutoPause)
{
	d->cfg_autoPause = newAutoPause.toBool();

	if (d->cfg_autoPause) {
		// Auto Pause is enabled.
		qAppFocusChanged(NULL, gqt4_app->focusWidget());
	} else {
		// Auto Pause is disabled.
		// Undo auto pause.
		paused_t paused_set;
		paused_t paused_clear;
		paused_set.data = 0;
		paused_clear.data = 0;
		paused_clear.paused_auto = 1;

		d->emuManager->pauseRequest(paused_set, paused_clear);
	}
}


/**
 * Show Menu Bar setting has changed.
 * @param newShowMenuBar (bool) New Show Menu Bar setting.
 */
void GensWindow::showMenuBar_changed_slot(QVariant newShowMenuBar)
{
	d->cfg_showMenuBar = newShowMenuBar.toBool();
	d->initMenuBar();
}


/** Wrapper functions for GensActions. **/
/** TODO: Have GensActions emit signals, and link them to EmuManager slots. **/

void GensWindow::openRom(void)
	{ d->emuManager->openRom(); }
void GensWindow::openRom(QString filename, QString z_filename)
	{ d->emuManager->openRom(filename, z_filename); }
void GensWindow::closeRom(void)
	{ d->emuManager->closeRom(); }
void GensWindow::saveState(void)
	{ d->emuManager->saveState(); }
void GensWindow::loadState(void)
	{ d->emuManager->loadState(); }
void GensWindow::screenShot(void)
	{ d->emuManager->screenShot(); }
void GensWindow::setAudioRate(int newRate)
	{ d->emuManager->setAudioRate(newRate); }
void GensWindow::setStereo(bool newStereo)
	{ d->emuManager->setStereo(newStereo); }

/** VBackend properties. **/
void GensWindow::toggleFastBlur(void)
	{ d->vBackend->setFastBlur(!d->vBackend->fastBlur()); }
StretchMode_t GensWindow::stretchMode(void)
	{ return d->vBackend->stretchMode(); }
void GensWindow::setStretchMode(StretchMode_t newStretchMode)
	{ d->vBackend->setStretchMode(newStretchMode); }

bool GensWindow::idleThreadAllowed(void)
	{ return d->idleThreadAllowed; }

void GensWindow::setIdleThreadAllowed(bool newIdleThreadAllowed)
{
	d->idleThreadAllowed = newIdleThreadAllowed;
	d->checkIdleThread();
}

// Wrapper for GensActions.
bool GensWindow::menuItemCheckState(int action)
	{ return d->gensMenuBar->menuItemCheckState(action); }


/**
 * The Idle thread is finished rendering a frame.
 */
void GensWindow::idleThread_frameDone(void)
{
	// Make sure the idle thread is still running.
	if (!d->idleThread || d->idleThread->isStopRequested())
		return;
	if (d->emuManager->isRomOpen())
		return;

	// Update video.
	d->emuManager->updateVBackend();

	// Resume the idle thread.
	d->idleThread->resume();
}


/**
 * Intro Style setting has changed.
 * @param newIntroStyle (int) New Intro Style setting.
 */
void GensWindow::introStyle_changed_slot(QVariant newIntroStyle)
{
	d->cfg_introStyle = newIntroStyle.toInt();
	d->checkIdleThread();

	// Prevent race conditions.
	if (!d->emuManager)
		return;

	if (!d->emuManager->isRomOpen() && d->cfg_introStyle == 0) {
		// Intro style was changed to "None", and emulation isn't running.
		// Clear the screen.
		d->emuManager->updateVBackend();
	}
}


/**
 * Show the context menu.
 * @param pos Position to show the context menu. (widget coordinates)
 */
void GensWindow::showContextMenu(const QPoint& pos)
{
	if (d->isGlobalMenuBar()) {
		// Global menu bar. Popup menus aren't necessary.
		// TODO: Enable the context menu in full-screen mode?
		return;
	} else {
		// No global menu bar.

		// on't do anything if the menu bar is visible.
		if (d->isShowMenuBar())
			return;

		// Set up the context menu.
		QPoint globalPos = this->mapToGlobal(pos);
		d->gensMenuBar->popupMenu()->popup(globalPos);
	}
}

}
