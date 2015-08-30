/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * PngReader.hpp: PNG image reader.                                        *
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

#ifndef __LIBZOMG_PngReader_HPP__
#define __LIBZOMG_PngReader_HPP__

#include "minizip/zip.h"

// Image data struct.
extern "C" struct _Zomg_Img_Data_t;

namespace LibZomg {

class PngReaderPrivate;
class PngReader
{
	public:
		PngReader();
		~PngReader();

	protected:
		friend class PngReaderPrivate;
		PngReaderPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibZomg-specific version of Q_DISABLE_COPY().
		PngReader(const PngReader &);
		PngReader &operator=(const PngReader &);

	public:
		// TODO: Better memory allocation semantics?

		enum ReaderFlags {
			/**
			 * Default flags.
			 */
			RF_Default = 0,

			/**
			 * Use standard alpha channel semantics.
			 * 0x00 == transparent, 0xFF == opaque
			 * This is compatible with OpenGL.
			 */
			RF_STANDARD_ALPHA	= 0,

			/**
			 * Use inverted alpha channel semantics.
			 * 0xFF == transparent, 0xFF == opaque
			 * This is compatible with Gens/GS II's
			 * internal rendering code, which doesn't use
			 * alpha-transparency and clears the high byte.
			 */
			RF_INVERTED_ALPHA	= (1 << 0),
		};

		/**
		 * Read an image from a PNG file in memory.
		 * Image is always loaded as 32-bit xBGR.
		 * @param img_data	[out] Image data. (Caller must free img_data->data on success.)
		 * @param png_file	[in] PNG file data.
		 * @param png_size	[in] Size of PNG file data.
		 * @param flags		[in, opt] ReaderFlags.
		 * @return 0 on success; negative errno on error.
		 */
		int readFromMem(_Zomg_Img_Data_t *img_data,
				const void *png_file, size_t png_size,
				int flags = RF_Default);

		/**
		 * Read an image from a PNG file.
		 * Image is always loaded as 32-bit xBGR.
		 * @param img_data	[out] Image data. (Caller must free img_data->data on success.)
		 * @param filename	[in] PNG file.
		 * @param flags		[in, opt] ReaderFlags.
		 * @return 0 on success; negative errno on error.
		 */
		int readFromFile(_Zomg_Img_Data_t *img_data, const char *filename,
				 int flags = RF_Default);
};

}

#endif /* __LIBZOMG_PngReader_HPP__ */
