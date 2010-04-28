/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * SdlVideo.hpp: SDL video handler.                                        *
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

#ifndef __LIBGENS_SDLVIDEO_HPP__
#define __LIBGENS_SDLVIDEO_HPP__

#include <SDL/SDL.h>
#include <string>

namespace LibGens
{

class SdlVideo
{
	public:
		/// NOTE: Init() and End() are to be called by LibGens ONLY!
		static int Init(void *wid);
		static int End(void);
		
		static int Width(void)
		{
			if (!ms_screen)
				return 0;
			
			return ms_screen->w;
		}
		
		static int Height(void)
		{
			if (!ms_screen)
				return 0;
			
			return ms_screen->h;
		}
		
		static void SetWinTitle(const char *newTitle);
		
		// TODO: Should this be publicly accessible?
		static SDL_Surface *ms_screen;
	
	protected:
		static void *ms_wid;
		static std::string ms_sWinTitle;
		static const unsigned int SDL_VideoModeFlags;
	
	private:
		SdlVideo() { }
		~SdlVideo() { }
};

}

#endif /* __LIBGENS_SDLVIDEO_HPP__ */
