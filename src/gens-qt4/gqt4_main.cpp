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

// Win32 compatibility functions.
#ifdef _WIN32
#include "gqt4_win32.hpp"
#endif

#include "GensQApplication.hpp"
#include "GensWindow.hpp"
#include "SigHandler.hpp"

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
	GensQt4::gqt4_config = new GensConfig();
	
	// TODO: Parse command line arguments.
	// They're available in app.arguments() [QStringList].
	
#ifdef _WIN32
	// Win32: Set the application font.
	GensQt4::Win32_SetFont();
#endif
	
	// Initialize LibGens.
	LibGens::Init();
	
	// Register the LOG_MSG() critical error handler.
	log_msg_register_critical_fn(gqt4_log_msg_critical);
	
	// Register the LibGens OSD handler.
	lg_set_osd_fn(gqt4_osd);
	
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
	
	// Unregister the LibGens OSD handler.
	// TODO: Do this earlier?
	lg_set_osd_fn(NULL);
	
	// Shut down LibGens.
	LibGens::End();
	
	// Unregister the signal handler.
	GensQt4::SigHandler::End();
	
	// Finished.
	return ret;
}


/**
 * gqt4_log_msg_critical(): LOG_MSG() critical error handler.
 * @param channel Debug channel.
 * @param msg Message. (Preformatted)
 */
void gqt4_log_msg_critical(const char *channel, const char *msg)
{
	QString title = "Gens Critical Error: " + QString(channel);
	
	QMessageBox dialog(QMessageBox::Critical,
			   title, QString(msg));
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
	{
		gqt4_config->save();
		delete gqt4_config;
		gqt4_config = NULL;
	}
}

}
