/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * gqt4_main.hpp: Main UI code.                                            *
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
#include "libgens/lg_main.hpp"

#include <QApplication>
#include "GensWindow.hpp"

#include <stdio.h>


int main(int argc, char *argv[])
{
	// Create the main UI.
	QApplication app(argc, argv);
	GensQt4::GensWindow gens_window;
	gens_window.show();
	
	// Initialize LibGens.
	int ret = LibGens::Init((void*)gens_window.sdl->winId(), "Gens/GS II");
	if (ret != 0)
		return ret;
	gens_window.gensResize();
	
	char buf[1024];
	int n, r, g, b;
	uint32_t color;
	color = (255 | (128 << 8) | (64 << 16));
	LibGens::qToLG->push(LibGens::MtQueue::MTQ_LG_SETBGCOLOR, (void*)color);
	
	// Run the Qt4 UI.
	ret = app.exec();
	
	// Shut down LibGens.
	LibGens::End();
	
	// Finished.
	return 0;
}


#ifdef _WIN32
/**
 * WinMain(): Win32 entry point.
 * TODO: Add Unicode version and convert the command line to UTF-8.
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// TODO: Convert lpCmdLine to argc/argv[].
	int argc = 1;
	char *argv[1] = {"gens-qt4"};
	
	// TODO: Handle nCmdShow.
	// TODO: Store hInstance.
	main(argc, argv);
}
#endif /* _WIN32 */


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
