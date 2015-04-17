/***************************************************************************
 * libzomg: Zipped Original Memory from Genesis.                           *
 * PngWriter.cpp: PNG image writer.                                        *
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

#include "PngWriter.hpp"
#include "img_data.h"
#include "zomg_byteswap.h"

// libpng
#include <png.h>

// MiniZip
#include "minizip/zip.h"

// C includes.
#include <stdint.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

// C includes. (C++ namespace)
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

/** PngWriterPrivate **/

class PngWriterPrivate
{
	public:
		PngWriterPrivate(PngWriter *q);
		~PngWriterPrivate();

	protected:
		friend class PngWriter;
		PngWriter *const q;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibPngWriter-specific version of Q_DISABLE_COPY().
		PngWriterPrivate(const PngWriterPrivate &);
		PngWriterPrivate &operator=(const PngWriterPrivate &);

	public:
		// Pixel masks.
		static const uint16_t MASK_RED_15       = 0x7C00;
		static const uint16_t MASK_GREEN_15     = 0x03E0;
		static const uint16_t MASK_BLUE_15      = 0x001F;

		static const uint16_t MASK_RED_16       = 0xF800;
		static const uint16_t MASK_GREEN_16     = 0x07E0;
		static const uint16_t MASK_BLUE_16      = 0x001F;

		static const uint32_t MASK_RED_32       = 0xFF0000;
		static const uint32_t MASK_GREEN_32     = 0x00FF00;
		static const uint32_t MASK_BLUE_32      = 0x0000FF;

		// Pixel shifts.
		static const uint8_t SHIFT_RED_15       = 7;
		static const uint8_t SHIFT_GREEN_15     = 2;
		static const uint8_t SHIFT_BLUE_15      = 3;

		static const uint8_t SHIFT_RED_16       = 8;
		static const uint8_t SHIFT_GREEN_16     = 3;
		static const uint8_t SHIFT_BLUE_16      = 3;

		static const uint8_t SHIFT_RED_32       = 16;
		static const uint8_t SHIFT_GREEN_32     = 8;
		static const uint8_t SHIFT_BLUE_32      = 0;

		/**
		 * Write 16-bit PNG rows.
		 * @param pixel Typename.
		 * @param maskR Red mask.
		 * @param maskG Green mask.
		 * @param maskB Blue mask.
		 * @param shiftR Red shift. (Right)
		 * @param shiftG Green shift. (Right)
		 * @param shiftB Blue shift. (Left)
		 * @param img_data Image data.
		 * @param row_buffer Row buffer. (Must be at least width * 3 bytes.)
		 * @param png_ptr PNG pointer.
		 */
		template<typename pixel,
			 const pixel maskR, const pixel maskG, const pixel maskB,
			 const unsigned int shiftR, const unsigned int shiftG, const unsigned int shiftB>
		static void T_writePNG_rows_16(const Zomg_Img_Data_t *img_data, uint8_t *row_buffer,
					       png_structp png_ptr);

	public:
		/**
		 * PNG MiniZip write function.
		 * @param png_ptr PNG pointer.
		 * @param buf Data to write.
		 * @param len Size of buf.
		 */
		static void png_io_minizip_write(png_structp png_ptr, png_bytep buf, png_size_t len);

		/**
		 * PNG MiniZip flush function.
		 * Required when writing PNG images.
		 * This implementation is a no-op.
		 * @param png_ptr PNG pointer.
		 */
		static void png_io_minizip_flush(png_structp png_ptr);

		/**
		 * Internal PNG write function.
		 * @param png_ptr PNG pointer.
		 * @param info_ptr PNG info pointer.
		 * @param img_data PNG image data.
		 * @return 0 on success; negative errno on error.
		 */
		static int writeToPng(png_structp png_ptr, png_infop info_ptr,
				      const Zomg_Img_Data_t *img_data);
};

PngWriterPrivate::PngWriterPrivate(PngWriter *q)
	: q(q)
{ }

PngWriterPrivate::~PngWriterPrivate()
{ }

/**
 * Write 16-bit PNG rows.
 * @param pixel Typename.
 * @param maskR Red mask.
 * @param maskG Green mask.
 * @param maskB Blue mask.
 * @param shiftR Red shift. (Right)
 * @param shiftG Green shift. (Right)
 * @param shiftB Blue shift. (Left)
 * @param img_data Image data.
 * @param row_buffer Row buffer. (Must be at least width * 3 bytes.)
 * @param png_ptr PNG pointer.
 */
template<typename pixel,
	 const pixel maskR, const pixel maskG, const pixel maskB,
	 const unsigned int shiftR, const unsigned int shiftG, const unsigned int shiftB>
void PngWriterPrivate::T_writePNG_rows_16(const Zomg_Img_Data_t *img_data, uint8_t *row_buffer,
					  png_structp png_ptr)
{
	// Write the rows.
	const uint16_t *screen = (const uint16_t*)img_data->data;
	const int row_adj = (img_data->pitch / sizeof(pixel)) - img_data->w;
	for (int y = img_data->h; y > 0; y--) {
		uint8_t *rowBufPtr = row_buffer;
		for (int x = img_data->w; x > 0; x--, rowBufPtr += 3) {
			// TODO: Fill in the unused bits with a copy of the MSBs.
			pixel MD_Color = *screen++;
			*(rowBufPtr + 0) = (uint8_t)((MD_Color & maskR) >> shiftR);
			*(rowBufPtr + 1) = (uint8_t)((MD_Color & maskG) >> shiftG);
			*(rowBufPtr + 2) = (uint8_t)((MD_Color & maskB) << shiftB);
		}

		// Write the row.
		png_write_row(png_ptr, row_buffer);

		// Next row.
		screen += row_adj;
	}
}

/**
 * PNG MiniZip memory write function.
 * @param png_ptr PNG pointer.
 * @param buf Data to write.
 * @param len Size of buf.
 */
void PngWriterPrivate::png_io_minizip_write(png_structp png_ptr, png_bytep buf, png_size_t len)
{
	void *io_ptr = png_get_io_ptr(png_ptr);
	if (!io_ptr)
		return;

	// Assuming io_ptr is a zipFile.
	zipFile zf = reinterpret_cast<zipFile>(io_ptr);
	// TODO: Check the return value!
	zipWriteInFileInZip(zf, buf, len);
}

/**
 * PNG MiniZip flush function.
 * Required when writing PNG images.
 * This implementation is a no-op.
 * @param png_ptr PNG pointer.
 */
void PngWriterPrivate::png_io_minizip_flush(png_structp png_ptr)
{
        // Do nothing!
        ((void)png_ptr);
}

/**
 * Internal PNG write function.
 * @param png_ptr PNG pointer.
 * @param info_ptr PNG info pointer.
 * @param img_data PNG image data.
 * @return 0 on success; negative errno on error.
 */
int PngWriterPrivate::writeToPng(png_structp png_ptr, png_infop info_ptr,
				 const Zomg_Img_Data_t *img_data)
{
	// Row pointers and/or buffer.
	// These need to be allocated here so they can be freed
	// in case an error occurs.
	union {
		void *p;

		// Row pointers. (32-bit color)
		// Each entry points to the beginning of a row.
		png_byte **row_pointers;

		// Row buffer. (15-bit or 16-bit color)
		// libpng doesn't support 15-bit or 16-bit color natively,
		// so the rows have to be converted.
		png_byte *row_buffer;
	} row;

	if (img_data->bpp == 32) {
		row.row_pointers = (png_byte**)png_malloc(png_ptr, sizeof(png_byte*) * img_data->h);
	} else {
		row.row_buffer = (png_byte*)png_malloc(png_ptr, sizeof(png_byte*) * img_data->w * 3);
	}

	// WARNING: Do NOT initialize any C++ objects past this point!
#ifdef PNG_SETJMP_SUPPORTED
	if (setjmp(png_jmpbuf(png_ptr))) {
		// PNG write failed.
		png_destroy_write_struct(&png_ptr, &info_ptr);
		png_free(png_ptr, row.p);
		// TODO: Better error code?
		return -ENOMEM;
	}
#endif /* PNG_SETJMP_SUPPORTED */

	// Disable PNG filters.
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);

	// Set the compression level to 5. (Levels range from 1 to 9.)
	// TODO: Add a parameter for this?
	png_set_compression_level(png_ptr, 5);

	// Set up the PNG header.
	png_set_IHDR(png_ptr, info_ptr, img_data->w, img_data->h,
		     8,				// Color depth (per channel).
		     PNG_COLOR_TYPE_RGB,	// RGB color.
		     PNG_INTERLACE_NONE,	// No interlacing.
		     PNG_COMPRESSION_TYPE_DEFAULT,
		     PNG_FILTER_TYPE_DEFAULT
		     );

	// Write the sBIT chunk.
	switch (img_data->bpp) {
		case 15: {
			static const png_color_8 sBIT_15 = {5, 5, 5, 0, 0};
			png_set_sBIT(png_ptr, info_ptr, &sBIT_15);
			break;
		}
		case 16: {
			static const png_color_8 sBIT_16 = {5, 6, 5, 0, 0};
			png_set_sBIT(png_ptr, info_ptr, &sBIT_16);
			break;
		}
		case 32:
		default: {
			static const png_color_8 sBIT_32 = {8, 8, 8, 0, 0};
			png_set_sBIT(png_ptr, info_ptr, &sBIT_32);
			break;
		}
	}

	// TODO: Separate function for time and text handling?
	// TODO: Add emulator and ROM information.

	// Write the current time.
	// TODO: Use GetSystemTime() on Windows.
	// TODO: Is there a way to get 64-bit time_t on 32-bit Linux?
	time_t cur_time = time(nullptr);
	png_time cur_png_time;
	png_convert_from_time_t(&cur_png_time, cur_time);
	png_set_tIME(png_ptr, info_ptr, &cur_png_time);

	// Write the current time as a string.
	// This is used by Windows for "Date taken:".
	// NOTE: This must be in LOCAL time.
	// TODO: Use localtime_r() if available.
	// Format: "yyyy:MM:dd hh:mm:ss"
	struct tm *cur_local_time = localtime(&cur_time);
	char cur_time_str[24];
	snprintf(cur_time_str, sizeof(cur_time_str), "%04d:%02d:%02d %02d:%02d:%02d",
		 cur_local_time->tm_year+1900, cur_local_time->tm_mon+1, cur_local_time->tm_mday,
		 cur_local_time->tm_hour, cur_local_time->tm_min, cur_local_time->tm_sec);

	png_text txt_png_time;
	txt_png_time.compression = PNG_TEXT_COMPRESSION_NONE;
	// FIXME: Needs to be non-const...
	txt_png_time.key = (png_charp)"Creation Time";
	txt_png_time.text = cur_time_str;
	txt_png_time.text_length = strlen(cur_time_str);
	txt_png_time.itxt_length = 0;
	txt_png_time.lang = nullptr;
	txt_png_time.lang_key = nullptr;
	png_set_text(png_ptr, info_ptr, &txt_png_time, 1);

	// Write the PNG header to the file.
	png_write_info(png_ptr, info_ptr);

	// TODO: Other text fields.

#if ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN
	// PNG stores data in big-endian.
	// On little-endian systems, byteswapping needs to be enabled.
	// TODO: Check if this really isn't needed on big-endian systems.
	// NOTE: This apparently only affects 16-bit pixels, which we don't use...
	//png_set_swap(png_ptr);
#endif /* ZOMG_BYTEORDER == ZOMG_LIL_ENDIAN */

	// Write the image.
	switch (img_data->bpp) {
		case 15: {
			// 15-bit color. (555)
			T_writePNG_rows_16<uint16_t,
					   MASK_RED_15, MASK_GREEN_15, MASK_BLUE_15,
					   SHIFT_RED_15, SHIFT_GREEN_15, SHIFT_BLUE_15>
					  (img_data, row.row_buffer, png_ptr);
			break;
		}

		case 16: {
			// 16-bit color. (565)
			T_writePNG_rows_16<uint16_t,
					   MASK_RED_16, MASK_GREEN_16, MASK_BLUE_16,
					   SHIFT_RED_16, SHIFT_GREEN_16, SHIFT_BLUE_16>
					  (img_data, row.row_buffer, png_ptr);
			// TODO
			break;
		}

		case 32:
		default:
			// Initialize the row pointers array.
			// TODO: const uint8_t*.
			uint8_t *data = (uint8_t*)img_data->data + ((img_data->h - 1) * img_data->pitch);
			for (int y = img_data->h - 1; y >= 0; y--, data -= img_data->pitch) {
				row.row_pointers[y] = data;
			}

			// libpng expects RGB data with no alpha channel, i.e. 24-bit.
			// However, there is an option to automatically convert 32-bit
			// without alpha channel to 24-bit, so we'll use that.
			png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

			// We're using "BGR" color.
			png_set_bgr(png_ptr);

			// Write the rows.
			png_write_rows(png_ptr, row.row_pointers, img_data->h);
			break;
	}

	// Free the row pointers.
	png_free(png_ptr, row.p);

	// Finished writing the PNG image.
	png_write_end(png_ptr, info_ptr);
	return 0;
}

/** PngWriter **/

PngWriter::PngWriter()
	: d(new PngWriterPrivate(this))
{ }

PngWriter::~PngWriter()
{
	delete d;
}

/**
 * Write an image to a PNG file.
 * @param img_data Image data.
 * @param filename PNG file.
 * @return 0 on success; negative errno on error.
 */
int PngWriter::writeToFile(const _Zomg_Img_Data_t *img_data, const char *filename)
{
	// TODO: Combine more of writeToFile() and writeToZip().
	if (!filename || !img_data || !img_data->data ||
	    img_data->w <= 0 || img_data->h <= 0 ||
	    (img_data->bpp != 15 && img_data->bpp != 16 && img_data->bpp != 32)) {
		// Invalid parameters.
		return -EINVAL;
	}

	// Set some sane limits for image size.
	if (img_data->w > 16384 || img_data->h > 16384) {
		// Image is too big.
		// TODO: -ENOSPC?
		return -ENOMEM;
	}

	// Calculate the minimum pitch.
	const unsigned int min_pitch = img_data->w * (img_data->bpp == 32 ? 4 : 2);
	if (img_data->pitch < min_pitch) {
		// Invalid parameters.
		return -EINVAL;
	}

	// Output file.
	// TODO: Convert filename to Unicode or ANSI on Windows.
	FILE *f = fopen(filename, "wb");
	if (!f) {
		// Error opening the output file.
		return -errno;
	}

	png_structp png_ptr;
	png_infop info_ptr;

	// Initialize libpng.
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) {
		return -ENOMEM;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, nullptr);
		return -ENOMEM;
	}

	// Initialize standard file I/O.
	png_init_io(png_ptr, f);

	// Write to PNG.
	int ret = d->writeToPng(png_ptr, info_ptr, img_data);
	fclose(f);
	if (ret != 0) {
		// Failed to write the PNG file.
		// Delete it so we don't end up with a half-written image.
		unlink(filename);
	}

	png_destroy_write_struct(&png_ptr, &info_ptr);
	return ret;
}

/**
 * Write an image to a PNG file in a ZIP file.
 * @param img_data Image data.
 * @param zfile ZIP file. (Must have a file open for writing.)
 * @return 0 on success; negative errno on error.
 */
int PngWriter::writeToZip(const _Zomg_Img_Data_t *img_data, zipFile zfile)
{
	// TODO: Combine more of writeToFile() and writeToZip().
	if (!zfile || !img_data || !img_data->data ||
	    img_data->w <= 0 || img_data->h <= 0 ||
	    (img_data->bpp != 15 && img_data->bpp != 16 && img_data->bpp != 32)) {
		// Invalid parameters.
		return -EINVAL;
	}

	// Set some sane limits for image size.
	if (img_data->w > 16384 || img_data->h > 16384) {
		// Image is too big.
		// TODO: -ENOSPC?
		return -ENOMEM;
	}

	// Calculate the minimum pitch.
	const unsigned int min_pitch = img_data->w * (img_data->bpp == 32 ? 4 : 2);
	if (img_data->pitch < min_pitch) {
		// Invalid parameters.
		return -EINVAL;
	}

	png_structp png_ptr;
	png_infop info_ptr;

	// Initialize libpng.
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) {
		return -ENOMEM;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, nullptr);
		return -ENOMEM;
	}

	// Initialize the custom I/O handler for MiniZip.
        png_set_write_fn(png_ptr, zfile, d->png_io_minizip_write, d->png_io_minizip_flush);

	// Write to PNG.
	// TODO: If it fails, delete the file from the ZIP?
	int ret = d->writeToPng(png_ptr, info_ptr, img_data);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return ret;
}

}
