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

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "GensWindow.hpp"

#include <stdio.h>

// Win32 compatibility functions.
#ifdef _WIN32
#include "gqt4_win32.hpp"
#endif

// Make sure SDL_main isn't defined.
#ifdef main
#undef main
#endif

// Gens window.
static GensQt4::GensWindow *gens_window = NULL;


/**
 * gqt4_main(): Main entry point.
 * @param argc Number of arguments.
 * @param argv Arguments.
 * @return Return value.
 */
int gqt4_main(int argc, char *argv[])
{
	// Create the main UI.
	QApplication app(argc, argv);
	
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
	
	// Shut down LibGens.
	LibGens::End();
	
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
	QMessageBox::critical(NULL, title, QString(msg));
}


namespace GensQt4
{

/**
 * QuitGens(): Quit Gens.
 */
void QuitGens(void)
{
	// TODO: Save configuration.
	
	// TODO: Stop LibGens' emulation core.
	
	// Shut down LibGens.
	LibGens::End();
}

}
