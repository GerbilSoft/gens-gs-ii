/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * PngReader.cpp: PNG image reader.                                        *
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

#include "PngReader.hpp"
#include "img_data.h"

// libpng
#include <png.h>

// C includes.
#include <stdint.h>
#include <stdlib.h>

// C includes. (C++ namespace)
#include <cassert>
#include <cerrno>
#include <csetjmp>
#include <cstring>

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::vector;

namespace LibZomg {

// TODO: Convert to a static class?

/** PngReaderPrivate **/

class PngReaderPrivate
{
	public:
		PngReaderPrivate(PngReader *q);
		~PngReaderPrivate();

	protected:
		friend class PngReader;
		PngReader *const q;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibPngReader-specific version of Q_DISABLE_COPY().
		PngReaderPrivate(const PngReaderPrivate &);
		PngReaderPrivate &operator=(const PngReaderPrivate &);

	public:
		/**
		 * PNG memory read struct.
		 */
		struct PngMemRead_t {
			const uint8_t *data;	// PNG data.
			png_size_t len;		// Length.
			png_size_t pos;		// Position.
		};

		/**
		 * PNG memory read function.
		 * @param png_ptr PNG pointer.
		 * @param buf Data to write.
		 * @param len Size of buf.
		 */
		static void png_io_mem_read(png_structp png_ptr, png_bytep buf, png_size_t len);

		/**
		 * Internal PNG read function.
		 * @param png_ptr	[in] PNG pointer.
		 * @param info_ptr	[in] PNG info pointer.
		 * @param img_data	[out] PNG image data.
		 * @param flags		[in, opt] ReaderFlags.
		 * @return 0 on success; negative errno on error.
		 */
		static int readFromPng(png_structp png_ptr, png_infop info_ptr,
				       Zomg_Img_Data_t *img_data,
				       int flags = PngReader::RF_Default);
};

PngReaderPrivate::PngReaderPrivate(PngReader *q)
	: q(q)
{ }

PngReaderPrivate::~PngReaderPrivate()
{ }

/**
 * PNG memory read function.
 * @param png_ptr PNG pointer.
 * @param buf Data to write.
 * @param len Size of buf.
 */
void PngReaderPrivate::png_io_mem_read(png_structp png_ptr, png_bytep buf, png_size_t len)
{
	// Assuming io_ptr is a PngMemRead_t.
	PngMemRead_t *png_mem = reinterpret_cast<PngMemRead_t*>(png_get_io_ptr(png_ptr));
	if (!png_mem)
		return;

	// Make sure there's enough data available.
	// TODO: Prevent overflow?
	if (png_mem->pos + len > png_mem->len) {
		// Not enough data is available.
		// TODO: This may still result in a crash. Use longjmp()?

		// Zero the buffer. (TODO: Only zero the unused portion.)
		memset(buf, 0, len);

		// Return the rest of the buffer.
		len = png_mem->len - png_mem->pos;
		if (len <= 0)
			return;
	}

	// Copy the data.
	memcpy(buf, &png_mem->data[png_mem->pos], len);
	png_mem->pos += len;
}

/**
 * Internal PNG read function.
 * @param png_ptr	[in] PNG pointer.
 * @param info_ptr	[in] PNG info pointer.
 * @param img_data	[out] PNG image data.
 * @param flags		[in, opt] ReaderFlags.
 * @return 0 on success; negative errno on error.
 */
int PngReaderPrivate::readFromPng(png_structp png_ptr, png_infop info_ptr,
				  Zomg_Img_Data_t *img_data, int flags)
{
	assert(img_data != nullptr);
	img_data->data = nullptr;	// TODO: Allow user-specified buffer?
	img_data->w = 0;
	img_data->h = 0;
	img_data->pitch = 0;
	img_data->bpp = 32;

	// If 'flags' is negative, use the defaults.
	if (flags < 0) {
		flags = 0;
	}

	// Row pointers. [NOTE: Allocated after IHDR is read.]
	png_byte **row_pointers = nullptr;

	// WARNING: Do NOT initialize any C++ objects past this point!
#ifdef PNG_SETJMP_SUPPORTED
	if (setjmp(png_jmpbuf(png_ptr))) {
		// PNG read failed.
		png_free(png_ptr, row_pointers);
		free(img_data->data);
		img_data->data = nullptr;
		// TODO: Better error code?
		return -ENOMEM;
	}
#endif /* PNG_SETJMP_SUPPORTED */

	// Read the PNG image information.
	png_read_info(png_ptr, info_ptr);

	// Read the PNG image header.
	// NOTE: libpng-1.2 defines png_uint_32 as long.
	// libpng-1.4.0beta7 appears to redefine it to unsigned int.
	// Since we're using unsigned int in img_data, we can't
	// save the values directly to the struct.
	// TODO: Conditionally use temp variables for libpng <1.4?
	int bit_depth, color_type;
	png_uint_32 img_w, img_h;
	png_get_IHDR(png_ptr, info_ptr,
		     &img_w, &img_h,
		     &bit_depth, &color_type,
		     nullptr, nullptr, nullptr);
	if (img_w <= 0 || img_h <= 0) {
		// Invalid image size.
		// TODO: Better error code?
		return -EINVAL;
	}
	// Save the image size.
	img_data->w = (unsigned int)img_w;
	img_data->h = (unsigned int)img_h;

	// Check for pHYs.
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_pHYs)) {
		// TODO: Only if unit_type == PNG_RESOLUTION_UNKNOWN?
		png_uint_32 phys_x, phys_y;
		png_get_pHYs(png_ptr, info_ptr, &phys_x, &phys_y, nullptr);
		img_data->phys_x = (unsigned int)phys_x;
		img_data->phys_y = (unsigned int)phys_y;
	} else {
		// No pHYs.
		img_data->phys_x = 0;
		img_data->phys_y = 0;
	}

	// Apply some conversions to ensure the returned
	// image data is 32-bit xBGR (or ABGR).

	// Make sure RGB color is used.
	bool has_alpha = false;
	switch (color_type) {
		case PNG_COLOR_TYPE_PALETTE:
			png_set_palette_to_rgb(png_ptr);
			break;
		case PNG_COLOR_TYPE_GRAY:
			png_set_gray_to_rgb(png_ptr);
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			has_alpha = true;
			break;
		default:
			break;
	}

	// Convert tRNS to alpha channel.
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
		has_alpha = true;
	}

	// Convert 16-bit per channel to 8-bit.
	if (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	// Get the new PNG information.
	png_get_IHDR(png_ptr, info_ptr, &img_w, &img_h, &bit_depth, &color_type,
		     nullptr, nullptr, nullptr);

	// Check if the image has an alpha channel.
	if (!has_alpha) {
		// No alpha channel.
		// Use filler instead.
		if (flags & PngReader::RF_INVERTED_ALPHA) {
			png_set_filler(png_ptr, 0x00, PNG_FILLER_AFTER);
		} else {
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
		}
	} else {
		if (flags & PngReader::RF_INVERTED_ALPHA) {
			// Invert the alpha channel.
			png_set_invert_alpha(png_ptr);
		}
	}

	// We're using "BGR" color.
	png_set_bgr(png_ptr);

	// Update the PNG info.
	png_read_update_info(png_ptr, info_ptr);

	// Allocate row pointers.
	row_pointers = (png_byte**)png_malloc(png_ptr, sizeof(png_byte*) * img_data->h);
	if (!row_pointers) {
		// Not enough memory is available.
		return -ENOMEM;
	}

	// Allocate image data.
	// (Caller must free this on success.)
	img_data->pitch = img_data->w * 4;
	uint8_t *data = (uint8_t*)malloc(img_data->pitch * img_data->h);
	if (!data) {
		// Not enough memory is available.
		png_free(png_ptr, row_pointers);
		return -ENOMEM;
	}
	img_data->data = data;

	// Initialize the row pointers array.
	data += (img_data->h - 1) * img_data->pitch;
	for (int y = img_data->h - 1; y >= 0; y--, data -= img_data->pitch) {
		row_pointers[y] = data;
	}

	// Read the image.
	png_read_image(png_ptr, row_pointers);

	// Free the row pointers.
	png_free(png_ptr, row_pointers);

	// Finished writing the PNG image.
	return 0;
}

/** PngReader **/

PngReader::PngReader()
	: d(new PngReaderPrivate(this))
{ }

PngReader::~PngReader()
{
	delete d;
}

/**
 * Read an image from a PNG file in memory.
 * Image is always loaded as 32-bit xBGR.
 * @param img_data	[out] Image data. (Caller must free img_data->data on success.)
 * @param png_file	[in] PNG file data.
 * @param png_size	[in] Size of PNG file data.
 * @param flags		[in, opt] ReaderFlags.
 * @return 0 on success; negative errno on error.
 */
int PngReader::readFromMem(Zomg_Img_Data_t *img_data,
			   const void *png_file, size_t png_size,
			   int flags)
{
	if (!img_data || !png_file || png_size == 0) {
		// Invalid parameters.
		return -EINVAL;
	}

	png_structp png_ptr;
	png_infop info_ptr;

	// Initialize libpng.
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) {
		return -ENOMEM;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		return -ENOMEM;
	}

	// Initialize the custom I/O handler for the memory handler.
	PngReaderPrivate::PngMemRead_t png_mem =
		{reinterpret_cast<const uint8_t*>(png_file), png_size, 0};
	png_set_read_fn(png_ptr, &png_mem, d->png_io_mem_read);

	// Read from PNG.
	int ret = d->readFromPng(png_ptr, info_ptr, img_data, flags);
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
	return ret;
}

		/**
 * Read an image from a PNG file.
 * Image is always loaded as 32-bit xBGR.
 * @param img_data	[out] Image data. (Caller must free img_data->data on success.)
 * @param filename	[in] PNG file.
 * @param flags		[in, opt] ReaderFlags.
 * @return 0 on success; negative errno on error.
 */
int PngReader::readFromFile(Zomg_Img_Data_t *img_data,
			    const char *filename, int flags)
{
	if (!img_data || !filename || !filename[0]) {
		// Invalid parameters.
		return -EINVAL;
	}

	png_structp png_ptr;
	png_infop info_ptr;

	// Initialize libpng.
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) {
		return -ENOMEM;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		return -ENOMEM;
	}

	// Output file.
	FILE *f = fopen(filename, "rb");
	if (!f) {
		// Error opening the output file.
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		return -errno;
	}

	// Initialize standard file I/O.
	png_init_io(png_ptr, f);

	// Read from PNG.
	int ret = d->readFromPng(png_ptr, info_ptr, img_data, flags);
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
	fclose(f);
	return ret;
}

}
