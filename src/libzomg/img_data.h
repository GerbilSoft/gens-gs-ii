/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * img_data.h: Image data struct.                                            *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __LIBZOMG_IMG_DATA_H__
#define __LIBZOMG_IMG_DATA_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Image data struct.
 * Used for PngWriter and PngReader.
 */
typedef struct _Zomg_Img_Data_t {
	void *data;		// Image data.
	uint32_t w;		// Width.
	uint32_t h;		// Height.
	uint32_t pitch;		// Pitch, in bytes.
	uint8_t bpp;		// Color depth. (15, 16, 32) [TODO: Use an enum?]

	/**
	 * Aspect ratio.
	 * Typical values: (non-interlaced)
	 * - H40 (320px): x=4, y=4
	 * - H32 (256px): x=5, y=4
	 *
	 * Interlaced mode should have the same values,
	 * assuming the width is doubled. If it isn't,
	 * double the X values.
	 *
	 * NOTE: PNG uses uint32_t for aspect ratio,
	 * so we're using that here instead of uint8_t.
	 *
	 * When writing, this will always be saved as
	 * PNG_RESOLUTION_UNKNOWN.
	 *
	 * When reading, the unit type is ignored.
	 */
	uint32_t phys_x;
	uint32_t phys_y;
} Zomg_Img_Data_t;

#ifdef __cplusplus
}
#endif

#endif /* __LIBZOMG_IMG_DATA_H__ */
