/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * PngWriter.hpp: PNG image writer.                                        *
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

#ifndef __LIBZOMG_PNGWRITER_HPP__
#define __LIBZOMG_PNGWRITER_HPP__

#include "minizip/zip.h"

// Image data struct.
extern "C" struct _Zomg_Img_Data_t;

namespace LibZomg {

class PngWriterPrivate;
class PngWriter
{
	public:
		PngWriter();
		~PngWriter();

	protected:
		friend class PngWriterPrivate;
		PngWriterPrivate *const d;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibZomg-specific version of Q_DISABLE_COPY().
		PngWriter(const PngWriter &);
		PngWriter &operator=(const PngWriter &);

	public:
		/**
		 * Write an image to a PNG file.
		 * @param img_data Image data.
		 * @param filename PNG file.
		 * @return 0 on success; negative errno on error.
		 */
		int writeToFile(const _Zomg_Img_Data_t *img_data, const char *filename);

		/**
		 * Write an image to a PNG file in a ZIP file.
		 * @param img_data Image data.
		 * @param zfile ZIP file. (Must have a file open for writing.)
		 * @return 0 on success; negative errno on error.
		 */
		int writeToZip(const _Zomg_Img_Data_t *img_data, zipFile zfile);
};

}

#endif /* __LIBZOMG_PNGWRITER_HPP__ */
