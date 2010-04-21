/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * lg_main.cpp: Main emulation code.                                       *
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

#include "lg_main.hpp"

// C includes.
#include <stdio.h>
#include <unistd.h>

// C++ includes.
#include <string>
using std::string;

#include <SDL/SDL.h>

#define LG_MSG(str, ...) \
	fprintf(stderr, "LibGens::%s(): " str "\n", __func__, ##__VA_ARGS__)


namespace LibGens
{


static bool m_isInit = false;
static const void *m_wid = NULL;
static SDL_Thread *m_thread = NULL;

// SDL video stuff.
// TODO: Move to a separate file.
static SDL_Surface *m_screen = NULL;
static string m_sWinTitle;

static const unsigned int SDL_VideoModeFlags = \
	(SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_ASYNCBLIT | SDL_HWACCEL);


/**
 * IsRunning(): Determines if LibGens is running.
 * @return True if the LibGens thread is running; false otherwise.
 */
bool IsRunning(void)
{
	return (m_isInit && m_thread);
}


/**
 * Init(): Initialize LibGens.
 * @param wid Parent window ID for the Gens window.
 * @param sWinTitle Window title.
 * @return 0 on success; non-zero on error.
 */
int Init(const void *wid, const char *sWinTitle)
{
	// TODO: Reference counting?
	if (m_isInit)
		return 0;
	
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0)
	{
		LG_MSG("Error initializing SDL: %s", SDL_GetError());
		return -1;
	}
	
	// Save the window title.
	if (sWinTitle)
		m_sWinTitle = string(sWinTitle);
	else
		m_sWinTitle = "";
	
	// Start the LibGens thread.
	m_wid = wid;
	m_thread = SDL_CreateThread(LgThread, NULL);
	if (!m_thread)
		return -2;
	
	// Wait 10s for the LibGens thread to be initialized.
	int tmr = 10000;
	while (!m_isInit && tmr > 0)
	{
		usleep(100000);
		tmr -= 100;
	}
	
	if (tmr <= 0)
	{
		// Error initializing the LibGens thread.
		LG_MSG("Error initializing LibGens thread.");
		SDL_KillThread(m_thread);
		m_thread = NULL;
		m_isInit = false;
		return -3;
	}
	
	// LibGens thread initialized.
	LG_MSG("LibGens thread initialized.");
	return 0;
}


/**
 * End(): Stop LibGens.
 * TODO
 * @return 0 on success; non-zero on error.
 */
int End(void)
{
	if (!m_thread || !m_isInit)
		return -1;
	
	// Post a QUIT message to the SDL thread.
	SDL_Event event;
	event.type = SDL_QUIT;
	SDL_PushEvent(&event);
	
	// Wait for the thread to exit.
	int status;
	SDL_WaitThread(m_thread, &status);
	m_thread = NULL;
	return 0;
}


/**
 * LgThread(): LibGens thread.
 * @param param Parameter from Init().
 * @return 0 on success; non-zero on error.
 */
int LgThread(void *param)
{
	// Initialize SDL video.
	m_screen = SDL_SetVideoMode(320, 240, 0, SDL_VideoModeFlags);
	
	// Set the window title.
	SDL_WM_SetCaption(m_sWinTitle.c_str(), NULL);
	
	// SDL is initialized.
	m_isInit = true;
	
	// Simple background stuff.
	int bg_shade = 0;
	bool bg_dir = false;
	
	// Event loop.
	SDL_Event event;
	bool done = false;
	
	while (!done)
	{
		if (SDL_WaitEvent(&event) == 0)
			break;
		
		switch (event.type)
		{
			case SDL_QUIT:
				done = true;
				break;
			
			default:
				// Invert the background color.
				if (!bg_dir)
				{
					bg_shade += 8;
					if (bg_shade >= 255)
					{
						bg_shade = 255;
						bg_dir = true;
					}
				}
				else
				{
					bg_shade -= 8;
					if (bg_shade <= 0)
					{
						bg_shade = 0;
						bg_dir = false;
					}
				}
				
				SDL_FillRect(m_screen, NULL, SDL_MapRGB(m_screen->format, bg_shade, bg_shade, bg_shade));
				SDL_UpdateRect(m_screen, 0, 0, 0, 0);
				break;
		}
	}
	
	// Thread exiting.
	m_isInit = false;
	LG_MSG("LibGens thread is exiting.");
	return 0;
}


}
