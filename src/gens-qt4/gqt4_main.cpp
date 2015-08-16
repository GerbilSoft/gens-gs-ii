/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * gqt4_main.cpp: Main UI code.                                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include "gqt4_main.hpp"

// LibGens includes.
#include "libgens/lg_main.hpp"
#include "libgens/macros/log_msg.h"

// Qt includes.
#include <QtGui/QMessageBox>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDir>

#include "GensQApplication.hpp"
#include "windows/GensWindow.hpp"
#include "SigHandler.hpp"

// General configuration signal handler.
#include "ConfigHandler.hpp"

#ifdef Q_WS_X11
// X11 includes.
#include <X11/Xlib.h>
#endif /* Q_WS_X11 */

// Text translation macro.
#define TR(text) \
	QCoreApplication::translate("gqt4_main", (text), nullptr, QCoreApplication::UnicodeUTF8)

// Gens window.
static GensQt4::GensWindow *gens_window = nullptr;

/**
 * LOG_MSG() critical error handler.
 * @param channel Debug channel. (ASCII)
 * @param msg Message. (Preformatted UTF-8)
 */
void gqt4_log_msg_critical(const char *channel, const char *msg)
{
	QString title = QLatin1String("Gens Critical Error:") + QChar(L' ') +
			QLatin1String(channel);

	QMessageBox dialog(QMessageBox::Critical, title, QString::fromUtf8(msg));
	dialog.setTextFormat(Qt::PlainText);
	dialog.exec();
}

/**
 * LibGens OSD handler.
 * @param osd_type: OSD type.
 * @param param: Integer parameter.
 */
void gqt4_osd(OsdType osd_type, int param)
{
	// TODO: Make sure this function doesn't run if Gens is shutting down.
	if (!gens_window)
		return;

	gens_window->osd(osd_type, param);
}

namespace GensQt4 {

// GensQApplication.
GensQApplication *gqt4_app = nullptr;

// Configuration store.
ConfigStore *gqt4_cfg = nullptr;

// Emulation objects.
EmuThread *gqt4_emuThread = nullptr;		// Thread.
LibGens::EmuContext *gqt4_emuContext = nullptr;	// Context.

/**
 * Quit Gens.
 */
void QuitGens(void)
{
	// TODO: Stop LibGens' emulation core.

	// Stop and delete the emulation thread.
	if (gqt4_emuThread) {
		gqt4_emuThread->stop();
		gqt4_emuThread->wait();
		delete gqt4_emuThread;
		gqt4_emuThread = nullptr;
	}

	// Delete the emulation context.
	// FIXME: Delete gqt4_emuContext after VBackend is finished using it. (MEMORY LEAK)
	//delete gqt4_emuContext;

	// Shut down LibGens.
	LibGens::End();

	// Save the configuration.
	gqt4_cfg->save();
}

}

/**
 * Main entry point.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Return value.
 */
int gens_main(int argc, char *argv[])
{
	// Register the signal handler.
	GensQt4::SigHandler::Init();

#ifdef Q_WS_X11
	// Initialize X11 threading.
#if QT_VERSION >= 0x040800
	// Let Qt call XInitThreads() itself, so that Qt knows it's in use.
	// Otherwise, QPixmap may warn about threads not being initialized.
	QApplication::setAttribute(Qt::AA_X11InitThreads);
#else
	XInitThreads();
	// Make sure we set Qt::AA_X11InitThreads if compiled with an older
	// version of Qt. This ensures that if the user upgrades to a newer
	// version of Qt, it will be set properly.
	QApplication::setAttribute(static_cast<Qt::ApplicationAttribute>(10));
#endif /* QT_VERSION >= 0x040800 */
#endif /* Q_WS_X11 */

	// Initialize the GensQApplication.
	GensQt4::gqt4_app = new GensQt4::GensQApplication(argc, argv);

	// Load the configuration.
	// TODO: Do this before or after command line arguments?
	GensQt4::gqt4_cfg = new GensQt4::ConfigStore();

	// External program configuration handler.
	GensQt4::ConfigHandler *configHandler = new GensQt4::ConfigHandler();

	// TODO: Parse command line arguments.
	// They're available in app.arguments() [QStringList].

	// Initialize LibGens.
	LibGens::Init();

	// Register the LOG_MSG() critical error handler.
	log_msg_register_critical_fn(gqt4_log_msg_critical);

	// Register the LibGens OSD handler.
	lg_set_osd_fn(gqt4_osd);

	// Set the EmuContext paths.
	// TODO: Do this here or in GensWindow initialization?
	QString sramPath = GensQt4::gqt4_cfg->configPath(GensQt4::PathConfig::GCPATH_SRAM);
	LibGens::EmuContext::SetPathSRam(sramPath.toUtf8().constData());

	// Add a signal handler for path changes.
	QObject::connect(
		GensQt4::gqt4_cfg->pathConfigObject(), SIGNAL(pathChanged(GensQt4::PathConfig::ConfigPath,QString)),
		configHandler, SLOT(pathChanged(GensQt4::PathConfig::ConfigPath,QString)));

	// Set boot ROM filenames.
	// TODO: Do this here or in GensWindow initialization?
	// TODO: Something similar to PathConfig?
	// TODO: Add convenience getString(), getBool(), etc. functions.
	QString tmssRomFilename = GensQt4::gqt4_cfg->get(QLatin1String("Genesis/tmssRom")).toString();
	LibGens::EmuContext::SetTmssRomFilename(tmssRomFilename.toUtf8().constData());
	bool tmssEnabled = GensQt4::gqt4_cfg->get(QLatin1String("Genesis/tmssEnabled")).toBool();
	LibGens::EmuContext::SetTmssEnabled(tmssEnabled);

	gens_window = new GensQt4::GensWindow();
	gens_window->show();

	// Run the Qt4 UI.
	int ret = GensQt4::gqt4_app->exec();

	/**
	 * TODO: Put cleanup code in GensQApplication's aboutToQuit() signal.
	 * app.exec() may not return on some platforms!
	 *
	 * Example: On Windows, if the user logs off while the program's running,
	 * app.exec() won't return.
	 */

	// TODO: Delete the translators?

	// Unregister the LibGens OSD handler.
	// TODO: Do this earlier?
	lg_set_osd_fn(nullptr);

	// Shut down LibGens.
	LibGens::End();

	// Unregister the signal handler.
	GensQt4::SigHandler::End();

	// Delete the various objects.
	delete configHandler;
	delete GensQt4::gqt4_cfg;
	delete GensQt4::gqt4_app;

	// Finished.
	return ret;
}
