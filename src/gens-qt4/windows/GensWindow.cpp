/******************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                     *
 * GensWindow.cpp: Gens Window.                                               *
 *                                                                            *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                         *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                                *
 * Copyright (c) 2008-2015 by David Korth.                                    *
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
#include <QtGui/QMenuBar>
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

#include "GensWindow_p.hpp"
namespace GensQt4 {

/** GensWindowPrivate **/

GensWindowPrivate::GensWindowPrivate(GensWindow *q)
	: q_ptr(q)
	, popupMenu(nullptr)
	, scale(1)			// Set the scale to 1x by default.
	, hasInitResize(false)		// Initial resize hasn't occurred yet.
	, idleThread(nullptr)
	, idleThreadAllowed(false)	// Not allowed yet.
{
	/** Configuration items. **/
	cfg_autoPause = gqt4_cfg->get(QLatin1String("autoPause")).toBool();
	cfg_introStyle = gqt4_cfg->getInt(QLatin1String("Intro_Effect/introStyle"));
	cfg_showMenuBar = gqt4_cfg->get(QLatin1String("GensWindow/showMenuBar")).toBool();
}

GensWindowPrivate::~GensWindowPrivate()
{
	delete emuManager;
	delete keyHandler;
	// TODO: Remove this.
	//delete gensMenuBar;
}

/** GensWindowPrivate **/

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
	Q_Q(GensWindow);
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
	title += GensWindow::tr("[NO-EMU]") + QChar(L' ');
#endif

	if (!emuManager->isRomOpen()) {
		// No ROM is running.
		title += gqt4_app->applicationName();
	} else {
		// ROM is running.
		if (emuManager->paused().paused_manual) {
			// Emulator is paused manually.
			title += GensWindow::tr("[Paused]") + QChar(L' ');
		}
		title += emuManager->romName();
	}

	Q_Q(GensWindow);
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
			idleThread = nullptr;
		}
	} else {
		// Make sure the idle thread is running.
		// TODO: Check for race conditions.
		if (!idleThread || !idleThread->isRunning()) {
			// Idle thread isn't running.
			Q_Q(GensWindow);
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
	: QMainWindow()
	, d_ptr(new GensWindowPrivate(this))
{
	Q_D(GensWindow);

	// Set up the User Interface.
	d->ui.setupUi(this);

#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	// TODO: Readd if a ROM is loaded.
	this->setWindowIcon(QIcon());
#endif

	// Initialize stuff.
	d->emuManager = new EmuManager();
	// Initialize the menu bar.
	d->initMenuBar();

	// Initialize the Key Manager and KeyHandlerQt.
	d->emuManager->setKeyManager(gqt4_cfg->m_keyManager);
	d->keyHandler = new KeyHandlerQt(this, gqt4_cfg->m_keyManager);

	// Create the Video Backend.
	// TODO: Allow selection of all available VBackend classes.
	d->vBackend = new GensQGLWidget(this, d->keyHandler);
	d->emuManager->setVBackend(d->vBackend);

	// Set the central widget for the UI.
	this->setCentralWidget(d->vBackend);

	// Enable drag and drop.
	this->setAcceptDrops(true);

	// Connect Emulation Manager signals to GensWindow.
	QObject::connect(d->emuManager, SIGNAL(updateFps(double)),
		this, SLOT(updateFps(double)));
	QObject::connect(d->emuManager, SIGNAL(stateChanged(void)),
		this, SLOT(stateChanged(void)));
	QObject::connect(d->emuManager, SIGNAL(osdPrintMsg(int,QString)),
		this, SLOT(osdPrintMsg(int,QString)));
	QObject::connect(d->emuManager, SIGNAL(osdShowPreview(int,QImage)),
		this, SLOT(osdShowPreview(int,QImage)));

       // Auto Pause: Application Focus Changed signal, and setting change signal.
       QObject::connect(gqt4_app, SIGNAL(focusChanged(QWidget*,QWidget*)),
               this, SLOT(qAppFocusChanged(QWidget*,QWidget*)));

	/** Configuration items: Signals. **/
	gqt4_cfg->registerChangeNotification(QLatin1String("autoPause"),
				this, SLOT(autoPause_changed_slot(QVariant)));
	gqt4_cfg->registerChangeNotification(QLatin1String("Intro_Effect/introStyle"),
				this, SLOT(introStyle_changed_slot(QVariant)));
	/* showMenuBar is handled in the menu sync code now.
	gqt4_cfg->registerChangeNotification(QLatin1String("GensWindow/showMenuBar"),
				this, SLOT(showMenuBar_changed_slot(QVariant)));
	*/

	// Update menu bar visibility.
	d->updateMenuBarVisibility();

	// Initialize the emulation state.
	d->idleThreadAllowed = true;
	stateChanged();

	// Enable the context menu.
	// TODO: If emulating the Sega Mega Mouse, add a hotkey to capture
	// the mouse to allow use of right-click.
	connect(this, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(showContextMenu(QPoint)));
	setContextMenuPolicy(Qt::CustomContextMenu);
}

/**
 * Free all resources acquired by the Gens window.
 */
GensWindow::~GensWindow()
{
	delete d_ptr;
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
	Q_D(GensWindow);
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
	Q_UNUSED(event)

	// If we've already run the initial resize, don't run it again.
	Q_D(GensWindow);
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
	Q_D(GensWindow);
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
		Q_D(GensWindow);
		d->ui.retranslateUi(this);
		// TODO: Synchronize the menu bar.
		d->updateMenuBarVisibility();
	}

	// Pass the event to the base class.
	QMainWindow::changeEvent(event);
}

/**
 * Rescale the window.
 * @param scale New scale value.
 */
void GensWindow::rescale(int scale)
{
	if (scale <= 0 || scale > 8)
		return;

	Q_D(GensWindow);
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
	Q_D(GensWindow);
	d->vBackend->osd_printqs(1500, msg);
}

/**
 * Set color depth.
 * TODO: Should this be a public function or a slot?
 * @param newBpp Color depth.
 */
void GensWindow::setBpp(LibGens::MdFb::ColorDepth bpp)
{
	// TODO: Move to GensConfig/qEmu.
	// TODO: bpp changes should be pushed to the emulation queue.
	// TODO: Maybe this should be a slot called by GensConfig.
	// FIXME: Sometimes there's a frame of "weirdness" between color depth changes.

	if (!gqt4_emuContext)
		return;

	// TODO: If no ROM is loaded, use the 'rom closed FB'?
	LibGens::MdFb *fb = gqt4_emuContext->m_vdp->MD_Screen;
	if (fb->bpp() == bpp)
		return;

	int bppVal;
	switch (bpp) {
		case LibGens::MdFb::BPP_15:
			bppVal = 15;
			break;
		case LibGens::MdFb::BPP_16:
			bppVal = 16;
			break;
		case LibGens::MdFb::BPP_32:
			bppVal = 32;
			break;
		default:
			return;
	}

	// Set the color depth.
	Q_D(GensWindow);
	fb->setBpp(bpp);
	d->vBackend->setVbDirty();
	//d->vBackend->vbUpdate();	// TODO: Don't update immediately?

	//: OSD message indicating color depth change.
	const QString msg = tr("Color depth set to %1-bit.", "osd").arg(bppVal);
	d->vBackend->osd_printqs(1500, msg);
}

/**
 * Update the FPS counter.
 */
void GensWindow::updateFps(double fps)
{
	Q_D(GensWindow);
	d->vBackend->fpsPush(fps);
}

/**
 * Emulation state changed.
 * - Update the video backend properties.
 * - Update the Gens title.
 */
void GensWindow::stateChanged(void)
{
	// TODO: Make sure that m_vBackend gets the new gqt4_emuContext in time.
	// FIXME: This is probably a race condition.
	Q_D(GensWindow);
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

	// Update the window state.
	d->setGensTitle();
	d->updateMenusForStateChanged();
}

/**
 * Print a message on the OSD.
 * @param duration Duration for the message to appear, in milliseconds.
 * @param msg Message to print.
 */
void GensWindow::osdPrintMsg(int duration, const QString &msg)
{
	Q_D(GensWindow);
	d->vBackend->osd_printqs(duration, msg);
}

/**
 * Show a preview image on the OSD.
 * @param duration Duration for the preview image to appaer, in milliseconds.
 * @param img Image to show.
 */
void GensWindow::osdShowPreview(int duration, const QImage& img)
{
	Q_D(GensWindow);
	d->vBackend->osd_show_preview(duration, img);
}

/**
 * Application focus has changed.
 * @param old Old widget.
 * @param now New widget.
 */
void GensWindow::qAppFocusChanged(QWidget *old, QWidget *now)
{
	Q_UNUSED(old)

	Q_D(GensWindow);
	if (!d->cfg_autoPause || !d->emuManager->isRomOpen()) {
		// Auto Pause is disabled,
		// or no ROM is running.
		return;
	}

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
void GensWindow::autoPause_changed_slot(const QVariant &newAutoPause)
{
	Q_D(GensWindow);
	d->cfg_autoPause = newAutoPause.toBool();

	if (d->cfg_autoPause) {
		// Auto Pause is enabled.
		qAppFocusChanged(nullptr, gqt4_app->focusWidget());
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

// TODO: Rename to isIdleThreadAllowed().
bool GensWindow::idleThreadAllowed(void)
{
	Q_D(GensWindow);
	return d->idleThreadAllowed;
}

void GensWindow::setIdleThreadAllowed(bool idleThreadAllowed)
{
	Q_D(GensWindow);
	d->idleThreadAllowed = idleThreadAllowed;
	d->checkIdleThread();
}

/**
 * The Idle thread is finished rendering a frame.
 */
void GensWindow::idleThread_frameDone(void)
{
	// Make sure the idle thread is still running.
	Q_D(GensWindow);
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
void GensWindow::introStyle_changed_slot(const QVariant &newIntroStyle)
{
	Q_D(GensWindow);
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
	Q_D(GensWindow);
	if (d->isGlobalMenuBar()) {
		// Global menu bar. Popup menus aren't necessary.
		// TODO: Enable the context menu in full-screen mode?
		return;
	} else {
		// No global menu bar.

		// Don't do anything if the menu bar is visible.
		if (d->isShowMenuBar())
			return;

		// Create the popup menu if it doesn't already exist.
		// NOTE: If the menus are reinitialized, d->popupMenu
		// will be deleted, which will trigger a reinitialization.
		if (!d->popupMenu) {
			d->popupMenu = new QMenu(this);
			// Add all menus from the main menu bar.
			foreach (QAction *action, this->menuBar()->actions()) {
				d->popupMenu->addAction(action);
			}
		}

		// Show the popup menu.
		QPoint globalPos = this->mapToGlobal(pos);
		d->popupMenu->popup(globalPos);
	}
}

}
