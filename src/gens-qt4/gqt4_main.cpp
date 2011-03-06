/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * gqt4_main.cpp: Main UI code.                                            *
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

#include "gqt4_main.hpp"

// LibGens includes.
#include "libgens/lg_main.hpp"
#include "libgens/macros/log_msg.h"

// Qt includes.
#include <QtGui/QMessageBox>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QLibraryInfo>

#include "GensQApplication.hpp"
#include "GensWindow.hpp"
#include "SigHandler.hpp"

// Text translation macro.
#define TR(text) \
	QCoreApplication::translate("gqt4_main", (text), NULL, QCoreApplication::UnicodeUTF8)

// Gens window.
static GensQt4::GensWindow *gens_window = NULL;


/**
 * gens_main(): Main entry point.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Return value.
 */
int gens_main(int argc, char *argv[])
{
	// Register the signal handler.
	GensQt4::SigHandler::Init();
	
	// Initialize the GensQApplication.
	GensQt4::gqt4_app = new GensQt4::GensQApplication(argc, argv);
	
	// Load the configuration.
	// TODO: Do this before or after command line arguments?
	GensQt4::gqt4_config = new GensQt4::GensConfig();
	
	// TODO: Parse command line arguments.
	// They're available in app.arguments() [QStringList].
	
	// Initialize the Qt translation system.
	// TODO: Allow switching languages on the fly?
	// TODO: Translations subdirectory.
	GensQt4::gqt4_qtTranslator = new QTranslator(GensQt4::gqt4_app);
	GensQt4::gqt4_qtTranslator->load(
		QLatin1String("qt_") + QLocale::system().name(),
		QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	GensQt4::gqt4_app->installTranslator(GensQt4::gqt4_qtTranslator);
	
	// Initialize the Gens translator.
	GensQt4::gqt4_gensTranslator = new QTranslator(GensQt4::gqt4_app);
	GensQt4::gqt4_gensTranslator->load(
		QLatin1String("gens-qt4_") + QLocale::system().name());
	GensQt4::gqt4_app->installTranslator(GensQt4::gqt4_gensTranslator);
	
	// Initialize LibGens.
	LibGens::Init();
	
	// Register the LOG_MSG() critical error handler.
	log_msg_register_critical_fn(gqt4_log_msg_critical);
	
	// Register the LibGens OSD handler.
	lg_set_osd_fn(gqt4_osd);
	
	// Set the EmuContext paths.
	// TODO: Do this here or in GensWindow initialization?
	LibGens::EmuContext::SetPathSRam(
		GensQt4::gqt4_config->userPath(GensQt4::GensConfig::GCPATH_SRAM).toUtf8().constData());
	
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
	lg_set_osd_fn(NULL);
	
	// Shut down LibGens.
	LibGens::End();
	
	// Unregister the signal handler.
	GensQt4::SigHandler::End();
	
	// Delete the configuration object.
	delete GensQt4::gqt4_config;
	GensQt4::gqt4_config = NULL;
	
	// Finished.
	return ret;
}


/**
 * gqt4_log_msg_critical(): LOG_MSG() critical error handler.
 * @param channel Debug channel. (ASCII)
 * @param msg Message. (Preformatted UTF-8)
 */
void gqt4_log_msg_critical(const char *channel, const utf8_str *msg)
{
	QString title = QLatin1String("Gens Critical Error:") + QChar(L' ') +
			QLatin1String(channel);
	
	QMessageBox dialog(QMessageBox::Critical, title, QString::fromUtf8(msg));
	dialog.setTextFormat(Qt::PlainText);
	dialog.exec();
}


/**
 * gqt4_osd(): LibGens OSD handler.
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


namespace GensQt4
{

// GensQApplication.
GensQApplication *gqt4_app = NULL;

// Configuration. (TODO: Use a smart pointer?)
GensConfig *gqt4_config = NULL;

// Emulation objects.
EmuThread *gqt4_emuThread = NULL;		// Thread.
LibGens::EmuContext *gqt4_emuContext = NULL;	// Context.

// Qt translators.
QTranslator *gqt4_qtTranslator = NULL;
QTranslator *gqt4_gensTranslator = NULL;

/**
 * QuitGens(): Quit Gens.
 */
void QuitGens(void)
{
	// TODO: Stop LibGens' emulation core.
	
	// Stop and delete the emulation thread.
	if (gqt4_emuThread)
	{
		gqt4_emuThread->stop();
		gqt4_emuThread->wait();
		delete gqt4_emuThread;
		gqt4_emuThread = NULL;
	}
	
	// Delete the emulation context.
	delete gqt4_emuContext;
	gqt4_emuContext = NULL;
	
	// Shut down LibGens.
	LibGens::End();
	
	// Save the configuration.
	if (gqt4_config)
		gqt4_config->save();
}

}
