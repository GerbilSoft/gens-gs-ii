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
	int w;			// Width.
	int h;			// Height.
	unsigned int pitch;	// Pitch, in bytes.
	uint8_t bpp;		// Color depth. (15, 16, 32) [TODO: Use an enum?]
} Zomg_Img_Data_t;

#ifdef __cplusplus
}
#endif

#endif /* __LIBZOMG_IMG_DATA_H__ */
