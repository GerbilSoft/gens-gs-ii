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
#include "macros/git.h"

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

// UI to LibGens queue.
MtQueue *qToLG = NULL;


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
	
	// Print the Gens/GS startup message.
	fprintf(stderr, "Gens/GS II (Development Build)\n");
#ifdef GENS_GIT_VERSION
	fprintf(stderr, "(" GENS_GIT_VERSION ")\n");
#endif
	fprintf(stderr, "\n"
		"Copyright (c) 1999-2002 by Stéphane Dallongeville.\n"
		"Copyright (c) 2003-2004 by Stéphane Akhoun.\n"
		"Copyright (c) 2008-2010 by David Korth.\n"
		"\n"
		"This program is free software; you can redistribute it and/or modify it\n"
		"under the terms of the GNU General Public License as published by the\n"
		"Free Software Foundation; either version 2 of the License, or (at your\n"
		"option) any later version.\n"
		"\n"
		"This program is distributed in the hope that it will be useful, but\n"
		"WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n"
		"\n"
		"You should have received a copy of the GNU General Public License along\n"
		"with this program; if not, write to the Free Software Foundation, Inc.,\n"
		"51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
		"\n");
	
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_NOPARACHUTE) < 0)
	{
		LG_MSG("Error initializing SDL: %s", SDL_GetError());
		return -1;
	}
	
	// Save the window title.
	if (sWinTitle)
		m_sWinTitle = string(sWinTitle);
	else
		m_sWinTitle = "";
	
	// Initialize the message queues.
	qToLG = new MtQueue(true);
	
	// Start the LibGens thread.
	m_wid = wid;
	m_thread = SDL_CreateThread(LgThread, NULL);
	if (!m_thread)
	{
		delete qToLG;
		qToLG = NULL;
		return -2;
	}
	
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
 * LgProcessSDLQueue(): Process the SDL queue.
 */
static void LgProcessSDLQueue(void)
{
	void *param;
	MtQueue::MtQ_type type;
	
	while ((type = qToLG->pop(&param)) != MtQueue::MTQ_NONE)
	{
		switch (type)
		{
			case MtQueue::MTQ_LG_SETBGCOLOR:
			{
				// Set the background color.
				const uint32_t bgr = (uint32_t)param;
				uint8_t r, g, b;
				r = (bgr & 0xFF);
				g = (bgr >> 8) & 0xFF;
				b = (bgr >> 16) & 0xFF;
				
				SDL_FillRect(m_screen, NULL, SDL_MapRGB(m_screen->format, r, g, b));
				SDL_UpdateRect(m_screen, 0, 0, 0, 0);
				break;
			}
			
			default:
				// Unhandled message.
				break;
		}
	}
}


/**
 * LgThread(): LibGens thread.
 * @param param Parameter from Init().
 * @return 0 on success; non-zero on error.
 */
int LgThread(void *param)
{
	// Initialize SDL video.
	// TODO: Check for errors in SDL_InitSubSystem().
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	m_screen = SDL_SetVideoMode(320, 240, 0, SDL_VideoModeFlags);
	
	// Set the window title.
	SDL_WM_SetCaption(m_sWinTitle.c_str(), NULL);
	
	// SDL is initialized.
	m_isInit = true;
	
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
			
			case SDL_EVENT_MTQ:
				// Multi-threaded queue event.
				LgProcessSDLQueue();
				break;
			
			default:
				break;
		}
	}
	
	// Shut down SDL video.
	// TODO: Shut down the entire SDL system somewhere?
	SDL_FreeSurface(m_screen);
	m_screen = NULL;
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	
	// Thread exiting.
	m_isInit = false;
	LG_MSG("LibGens thread is exiting.");
	return 0;
}


}
