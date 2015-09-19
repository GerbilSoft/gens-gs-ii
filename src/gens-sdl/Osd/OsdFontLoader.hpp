/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * OsdFontLoader.hpp: Onscreen Display font loader.                        *
 *                                                                         *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#ifndef __GENS_SDL_OSD_OSDFONTLOADER_HPP__
#define __GENS_SDL_OSD_OSDFONTLOADER_HPP__

// C includes.
#include <stdint.h>

namespace GensSdl {

class OsdFontLoader {
	private:
		// TODO: Singleton class instead of static class?
		OsdFontLoader() { };
		~OsdFontLoader() { };

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add GensSdl-specific version of Q_DISABLE_COPY().
		OsdFontLoader(const OsdFontLoader &);
		OsdFontLoader &operator=(const OsdFontLoader &);

	public:
		/**
		 * Load a font as 8-bit grayscale. (GL_ALPHA8, I8)
		 * @param name		[in]  Font name.
		 * @param p_chrW	[out] Character width.
		 * @param p_chrH	[out] Character height.
		 * @param p_sz		[out] Size of allocated data, in bytes.
		 * @return Allocated image data, or nullptr on error.
		 * Caller must free the image data using free().
		 * TODO: This may be switched to aligned_malloc() / aligned_free() later.
		 */
		static void *load_A8(const char *name,
			uint8_t *p_chrW, uint8_t *p_chrH, unsigned int *p_sz);
};

}

#endif /* __GENS_SDL_OSD_OSDFONTLOADER_HPP__ */
