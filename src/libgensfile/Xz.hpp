/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * Xz.hpp: Xz archive handler.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2016 by David Korth.                                 *
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

#ifndef __LIBGENSFILE_XZ_HPP__
#define __LIBGENSFILE_XZ_HPP__

#include "LzmaSdk.hpp"

// LZMA SDK includes.
#include "lzma/Xz.h"

namespace LibGensFile {

class Xz : public LzmaSdk
{
	public:
		/**
		 * Open a file with this archive handler.
		 * Check isOpen() afterwards to see if the file was opened.
		 * If it wasn't, check lastError() for the POSIX error code.
		 * @param filename Name of the file to open.
		 */
		Xz(const char *filename);
		virtual ~Xz();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Xz(const Xz &);
		Xz &operator=(const Xz &);

	public:
		/**
		 * Close the archive file.
		 */
		virtual void close(void) final;

		/**
		 * Get information about all files in the archive.
		 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int getFileInfo(mdp_z_entry_t **z_entry_out) final;

		/**
		 * Read all or part of a file from the archive.
		 * NOTE: This function is NOT optimized for random seeking.
		 * The skip functionality is intended for skipping over headers,
		 * e.g. for SMD-format ROMs.
		 *
		 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract.
		 * @param start_pos	[in]  Starting position within the file.
		 * @param read_len	[in]  Number of bytes to read.
		 * @param buf		[out] Buffer to read the file into.
		 * @param siz		[in]  Size of buf. (Must be >= read_len.)
		 * @param ret_siz	[out] Pointer to file_offset_t to store the number of bytes read.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int readFile(const mdp_z_entry_t *z_entry,
				     file_offset_t start_pos, file_offset_t read_len,
				     void *buf, file_offset_t siz, file_offset_t *ret_siz) final;

	private:
		// Xz archive.
		CXzs m_xzs;
		CXzUnpacker m_xzu;

		// Decompression buffers.
		uint8_t *m_inBuf, *m_outBuf;
		size_t m_inBufSz, m_outBufSz;
};

}

#endif /* __LIBGENSFILE_XZ_HPP__ */
