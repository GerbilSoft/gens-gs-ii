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

#include "OsdFontLoader.hpp"

// Fonts.
// TODO: Add 'name' parameter to OsdFont.
#include "OsdFont_VGA.hpp"
#include "OsdFont_C64.hpp"

// C includes.
#include <stdlib.h>

// C includes. (C++ namespace)
#include <cstring>

namespace GensSdl {

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
void *OsdFontLoader::load_A8(const char *name,
	uint8_t *p_chrW, uint8_t *p_chrH, unsigned int *p_sz)
{
	const OsdFont *font;
	if (!strcmp(name, "VGA")) {
		font = &VGA_font;
	} else if (!strcmp(name, "C64")) {
		font = &C64_font;
	} else {
		return nullptr;
	}

	const uint8_t chrW = font->w;
	const uint8_t chrH = font->h;
	const uint8_t *fontData = font->data;

	// Allocate the image buffer.
	const unsigned int sz = (256 * chrW * chrH);
	uint8_t *img = (uint8_t*)malloc(sz);
	if (!img)
		return nullptr;

	// Converting 1bpp characters to 8bpp.
	// pitch = 8 bytes per character; 16 per line.
	const int pitch = chrW * 16;
	for (int chr = 0; chr < 256; chr++) {
		const int y_pos = (chr / 16) * chrH;
		const int x_pos = (chr & 15) * chrW;

		uint8_t *pos = &img[(y_pos * pitch) + x_pos];
		// TODO: Support chrW != 8.
		const uint8_t *p_chr_data = &fontData[chr * chrH];
		for (int y = 0; y < chrH; y++, pos += (pitch - chrW), p_chr_data++) {
			uint8_t chr_data = *p_chr_data;
			for (int x = chrW; x > 0; x--, chr_data <<= 1) {
				*pos = ((chr_data & 0x80) ? 0xFF : 0);
				pos++;
			}
		}
	}

	// Return the data.
	if (p_chrW) {
		*p_chrW = chrW;
	}
	if (p_chrW) {
		*p_chrH = chrH;
	}
	if (p_sz) {
		*p_sz = sz;
	}
	return img;
}

}
