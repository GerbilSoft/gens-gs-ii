/***************************************************************************
 * gens-sdl: Gens/GS II basic SDL frontend.                                *
 * OsdFont.h: Onscreen Display font struct.                                *
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

#ifndef __GENS_SDL_OSDFONT_H__
#define __GENS_SDL_OSDFONT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsdFont {
	// Font data.
	// Typically a 2D array in the arrangement data[chr][row],
	// assuming an 8px-wide font.
	const uint8_t *data;

	// Font dimensions.
	uint8_t w;
	uint8_t h;
} OsdFont;

#ifdef __cplusplus
}
#endif

#endif /* __GENS_SDL_OSDFONT_H__ */
