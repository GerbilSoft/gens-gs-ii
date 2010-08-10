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
 * main(): Main entry point.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Return value.
 */
int main(int argc, char *argv[])
{
	// Register the signal handler.
	GensQt4::SigHandler::Init();
	
	// Create the main UI.
	GensQt4::GensQApplication app(argc, argv);
	
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
	
	gens_window = new GensQt4::GensWindow();
	gens_window->show();
	
	// Run the Qt4 UI.
	int ret = app.exec();
	
	/**
	 * TODO: Put cleanup code in GensQApplication's aboutToQuit() signal.
	 * app.exec() may not return on some platforms!
	 *
	 * Example: On Windows, if the user logs off while the program's running,
	 * app.exec() won't return.
	 */
	
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


namespace GensQt4
{

// Emulation thread.
EmuThread *gqt4_emuThread = NULL;

/**
 * QuitGens(): Quit Gens.
 */
void QuitGens(void)
{
	// TODO: Save configuration.
	
	// TODO: Stop LibGens' emulation core.
	
	// Stop the emulation thread.
	if (gqt4_emuThread)
	{
		gqt4_emuThread->stop();
		gqt4_emuThread->wait();
		delete gqt4_emuThread;
		gqt4_emuThread = NULL;
	}
	
	// Shut down LibGens.
	LibGens::End();
}

}
