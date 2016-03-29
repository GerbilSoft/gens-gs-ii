/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * LzmaSdk.hpp: LZMA SDK common functions.                                 *
 *                                                                         *
 * Do NOT use this class directly. It's intended to be used by LZMA-based  *
 * archive handlers, since they share a lot of boilerplate code for        *
 * interfacing with the LZMA SDK.                                          *
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

#ifndef __LIBGENSFILE_LZMASDK_HPP__
#define __LIBGENSFILE_LZMASDK_HPP__

#include "Archive.hpp"

// LZMA SDK includes.
#include "lzma/Alloc.h"
#include "lzma/7zFile.h"

namespace LibGensFile {

class LzmaSdk : public Archive
{
	public:
		/**
		 * Open a file with this archive handler.
		 * Check isOpen() afterwards to see if the file was opened.
		 * If it wasn't, check lastError() for the POSIX error code.
		 * @param filename Name of the file to open.
		 */
		LzmaSdk(const char *filename);
		virtual ~LzmaSdk();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		LzmaSdk(const LzmaSdk &);
		LzmaSdk &operator=(const LzmaSdk &);

	public:
		/**
		 * Close the archive file.
		 */
		virtual void close(void) override;

		/**
		 * Get information about all files in the archive.
		 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int getFileInfo(mdp_z_entry_t **z_entry_out) override;

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
				     void *buf, file_offset_t siz, file_offset_t *ret_siz) override;

	protected:
		/**
		 * Initialize the LZMA SDK.
		 * This function MUST be called by subclasses after
		 * checking for file magic.
		 *
		 * @return 0 on success; negative POSIX error code on error.
		 * If an error occurs, the file will NOT be closed;
		 * the caller must handle this. In addition, m_lastError
		 * is not set, since this is an internal function.
		 */
		int lzmaInit(void);

	private:
		// Set to true if the LZMA SDK CRC tables has been initialized.
		static bool ms_CrcInit;

	protected:
		// Memory allocators.
		ISzAlloc m_allocImp;
		ISzAlloc m_allocTempImp;

		// File stream wrappers.
		CFileInStream m_archiveStream;
		CLookToRead m_lookStream;
};

}

#endif /* __LIBGENSFILE_LZMASDK_HPP__ */
