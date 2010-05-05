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
#include <stdint.h>

#include <string>

namespace LibGens
{

class SdlVideo
{
	public:
		/// NOTE: Init() and End() are to be called by LibGens ONLY!
		static int Init(void) { return Init(ms_wid); }
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
		
		static void SetWindowTitle(const char *newTitle);
		
		// Resolution packing.
		static inline uint32_t PackRes(uint16_t w, uint16_t h)
		{
			return (((uint32_t)w) | (((uint32_t)h) << 16));
		}
		static inline uint16_t UnPackResW(uint32_t res)
		{
			return (res & 0xFFFF);
		}
		static inline uint16_t UnPackResH(uint32_t res)
		{
			return ((res >> 16) & 0xFFFF);
		}
		
		// TODO: Should this be publicly accessible?
		static SDL_Surface *ms_screen;
		
		// TODO: Not sure how to handle the "requested" size...
		static inline void SetPackedRes(uint32_t res)
		{
			DispW = UnPackResW(res);
			DispH = UnPackResH(res);
		}
		
		/** Color depth. **/
		
		enum BppMode
		{
			BPP_15 = 0,	// RGB555
			BPP_16 = 1,	// RGB565
			BPP_32 = 2,	// RGB888
		};
		
		static inline int BppModeToInt(BppMode bpp)
		{
			switch (bpp)
			{
				case BPP_15:
					return 15;
				case BPP_16:
					return 16;
				case BPP_32:
					return 32;
				default:
					return 0;
			}
		}
		
		/**
		 * BppOut(): Get the output color depth.
		 * @return Output color depth.
		 */
		static inline BppMode BppOut(void)
		{
			return ms_bppOut;
		}
	
	protected:
		static void *ms_wid;
		static std::string ms_sWinTitle;
		static const unsigned int SDL_VideoModeFlags;
		
		// TODO: Not sure how to handle the "requested" size...
		static int DispW;
		static int DispH;
		
		// Output color depth.
		// TODO: Allow this to be changed at runtime.
		static const BppMode ms_bppOut = BPP_16;
	
	private:
		SdlVideo() { }
		~SdlVideo() { }
};

}

#endif /* __LIBGENS_SDLVIDEO_HPP__ */
