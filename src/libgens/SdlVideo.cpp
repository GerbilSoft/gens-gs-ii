/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SdlVideo.cpp: SDL video handler.                                        *
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

#include "SdlVideo.hpp"

// Win32 compatibility wrappers.
#include "lg_win32.h"

// C++ includes.
#include <string>
using std::string;


namespace LibGens
{

// Static member initialization.
void *SdlVideo::ms_wid = NULL;
SDL_Surface *SdlVideo::ms_screen = NULL;
string SdlVideo::ms_sWinTitle;

const unsigned int SdlVideo::SDL_VideoModeFlags =
	(SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_ASYNCBLIT | SDL_HWACCEL);

// TODO: Not sure how to handle the "requested" size...
int SdlVideo::DispW = 320;
int SdlVideo::DispH = 240;


/**
 * Init(): Initialize SDL Video.
 * @param wid Parent window ID for the Gens window.
 * @return 0 on success; non-zero on error.
 */
int SdlVideo::Init(void *wid)
{
	// Save the parent window ID.
	ms_wid = wid;
	
	// TODO: Move env variable setting into a separate file for Win32/UNIX separation.
	if (ms_wid)
	{
		// Window ID specified.
		char s_wid[64];
		snprintf(s_wid, sizeof(s_wid), "%lld", (long long)(intptr_t)ms_wid);
		s_wid[sizeof(s_wid)-1] = 0x00;
		setenv("SDL_WINDOWID", s_wid, 1);
	}
	else
	{
		// Unset the Window ID variable.
		unsetenv("SDL_WINDOWID");
	}
	
	// TODO: Check for errors in SDL_InitSubSystem().
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	ms_screen = SDL_SetVideoMode(DispW, DispH, 0, SDL_VideoModeFlags);
	if (!ms_screen)
		return 1;
	
	// Unset the Window ID variable.
	unsetenv("SDL_WINDOWID");
	
	// Set the window title.
	// NOTE: On Win32, setting the window title while embedded
	// in another window causes the thread to hang!
	if (!ms_wid)
		SDL_WM_SetCaption(ms_sWinTitle.c_str(), NULL);
	
	// Video initialized.
	return 0;
}


int SdlVideo::End(void)
{
	if (!ms_screen)
		return 1;
	
	// Shut down SDL video.
	// TODO: Shut down the entire SDL system somewhere?
	SDL_FreeSurface(ms_screen);
	ms_screen = NULL;
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return 0;
}


/**
 * SetWinTitle(): Set the SDL Window Title.
 * @param newTitle New window title.
 */
void SdlVideo::SetWinTitle(const char *newTitle)
{
	if (!newTitle)
		ms_sWinTitle.clear();
	else
		ms_sWinTitle = std::string(newTitle);
	
	// Set the window title.
	// NOTE: On Win32, setting the window title while embedded
	// in another window causes the thread to hang!
	if (ms_screen && !ms_wid)
		SDL_WM_SetCaption(ms_sWinTitle.c_str(), NULL);
}

}
