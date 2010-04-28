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
#include "SdlVideo.hpp"

// C includes.
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// C++ includes.
#include <string>
using std::string;

#include <SDL/SDL.h>

#define LG_MSG(str, ...) \
	fprintf(stderr, "LibGens::%s(): " str "\n", __func__, ##__VA_ARGS__)


namespace LibGens
{


static bool m_isInit = false;
static SDL_Thread *ms_thread = NULL;

// UI to LibGens queue.
MtQueue *qToLG = NULL;


/**
 * IsRunning(): Determines if LibGens is running.
 * @return True if the LibGens thread is running; false otherwise.
 */
bool IsRunning(void)
{
	return (m_isInit && ms_thread);
}


/**
 * Init(): Initialize LibGens.
 * @param wid Initial parent window ID for the Gens window.
 * @param sWinTitle Initial window title.
 * @return 0 on success; non-zero on error.
 */
int Init(void *wid)
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
	
	// Initialize the message queues.
	qToLG = new MtQueue(MtQ_Callback);
	
	// Start the LibGens thread.
	ms_thread = SDL_CreateThread(LgThread, wid);
	if (!ms_thread)
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
		SDL_KillThread(ms_thread);
		ms_thread = NULL;
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
	if (!ms_thread || !m_isInit)
		return -1;
	
	// Post a QUIT message to the SDL thread.
	SDL_Event event;
	event.type = SDL_QUIT;
	SDL_PushEvent(&event);
	
	// Wait for the thread to exit.
	int status;
	SDL_WaitThread(ms_thread, &status);
	ms_thread = NULL;
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
				if (!SdlVideo::ms_screen)
					break;
				
				// Set the background color.
				const uint32_t bgr = LG_POINTER_TO_UINT(param);
				uint8_t r, g, b;
				r = (bgr & 0xFF);
				g = (bgr >> 8) & 0xFF;
				b = (bgr >> 16) & 0xFF;
				
				SDL_FillRect(SdlVideo::ms_screen, NULL, SDL_MapRGB(SdlVideo::ms_screen->format, r, g, b));
				SDL_UpdateRect(SdlVideo::ms_screen, 0, 0, 0, 0);
				break;
			}
			
			case MtQueue::MTQ_LG_UPDATE:
				if (!SdlVideo::ms_screen)
					break;
				
				// Update video.
				SDL_UpdateRect(SdlVideo::ms_screen, 0, 0, 0, 0);
				break;
			
			case MtQueue::MTQ_LG_SDLVIDEO_RESIZE:
				// Resize the SDL Video window.
				SdlVideo::End();
				SdlVideo::SetPackedRes(LG_POINTER_TO_UINT(param));
				SdlVideo::Init();
				// TODO: Notify the UI that the video has been resized.
				break;
			
			default:
				// Unhandled message.
				break;
		}
	}
}


/**
 * LgThread(): LibGens thread.
 * @param param Parameter from Init(). [initial window ID]
 * @return 0 on success; non-zero on error.
 */
int LgThread(void *param)
{
	// Initialize SDL Video.
	SdlVideo::Init(param);
	
	// LibGens SDL event loop.
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
			
			case SDL_VIDEOEXPOSE:
				// Update video.
				if (!SdlVideo::ms_screen)
					break;
				
				SDL_UpdateRect(SdlVideo::ms_screen, 0, 0, 0, 0);
				break;
			
			default:
				break;
		}
	}
	
	// Shut down SDL video.
	SdlVideo::End();
	
	// Thread exiting.
	m_isInit = false;
	LG_MSG("LibGens thread is exiting.");
	return 0;
}


/**
 * MtQ_Callback(): MtQueue callback function.
 * @param mtq MtQueue object.
 */
void MtQ_Callback(MtQueue *mtq)
{
	// Notify SDL about the new message.
	SDL_Event event;
	event.type = SDL_EVENT_MTQ;
	event.user.data1 = (void*)mtq;
	event.user.data2 = NULL;
	SDL_PushEvent(&event);
}

}
