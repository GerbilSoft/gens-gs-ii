/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * Archive.hpp: Archive file reader. (Base Class)                          *
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

/**
 * DUMMY archive handler. This is a base class intended to be
 * subclassed for actual archive handlers.
 *
 * Using this class directly will effectively result in a nop.
 */

#ifndef __LIBGENSFILE_ARCHIVE_HPP__
#define __LIBGENSFILE_ARCHIVE_HPP__

// PACKED macro from libgens/macros/common.h.
// TODO: MSVC version; common version from MDP?
#if !defined(PACKED)
#if defined(__GNUC__)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif /* defined(__GNUC__) */
#endif /* !defined(PACKED) */

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <cstdio>
// C++ includes.
#include <algorithm>
#include <string>

// TODO: Use the MDP headers for mdp_z_entry_t.
#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
typedef struct PACKED _mdp_z_entry_t {
	char *filename;
	size_t filesize;

	struct _mdp_z_entry_t *next;
} mdp_z_entry_t;
#pragma pack()

#ifdef __cplusplus
}
#endif

// TODO: Use MDP error codes later.
// For now, using POSIX error codes.

namespace LibGensFile {

class Archive
{
	public:
		/**
		 * Open a file with this archive handler.
		 * Check isOpen() afterwards to see if the file was opened.
		 * If it wasn't, check lastError() for the POSIX error code.
		 * @param filename Name of the file to open.
		 */
		Archive(const char *filename);
		virtual ~Archive();

	private:
		// COMMIT NOTE: Maybe add to the initial commit?
		// Otherwise, note that it was copied from the Factory class.
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Archive(const Archive &);
		Archive &operator=(const Archive &);

	public:
		/**
		 * Get the last error code. (POSIX error codes)
		 * This value is NOT reset if a function is successful.
		 *
		 * Unlike function return values, this error code is
		 * always a positive value.
		 *
		 * @return Last error code, or 0 if no error occurred.
		 */
		inline int lastError(void) const;

		/**
		 * Is the file open?
		 * @return True if open; false if not.
		 */
		inline bool isOpen(void) const;

		/**
		 * Close the archive file.
		 */
		virtual void close(void);

		// Using 64-bit file offsets.
		// TODO: Update MDP to always use 64-bit file offsets?
		typedef int64_t file_offset_t;

		/**
		 * Get information about all files in the archive.
		 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		virtual int getFileInfo(mdp_z_entry_t **z_entry_out);

		/**
		 * Read an entire file from the archive.
		 *
		 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract.
		 * @param buf		[out] Buffer to read the file into.
		 * @param siz		[in]  Size of buf. (Must be >= the size of the file.)
		 * @param ret_siz	[out] Pointer to file_offset_t to store the number of bytes read.
		 * @return 0 on success; negative POSIX error code on error.
		 */
		inline int readFile(const mdp_z_entry_t *z_entry, void *buf,
				    file_offset_t siz, file_offset_t *ret_siz);

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
				     void *buf, file_offset_t siz, file_offset_t *ret_siz);

		/**
		 * Free an allocated mdp_z_entry_t list.
		 * @param z_entry Pointer to the first entry in the list.
		 */
		static void z_entry_t_free(mdp_z_entry_t *z_entry);

	protected:
		/**
		 * Check for magic at the beginning of the file.
		 * @param magic	[in] Magic bytes.
		 * @param siz	[in] Size of magic.
		 * @return 0 on success; negative POSIX error code on error.
		 * m_lastError is NOT set by this function, since it's
		 * for use by subclasses only.
		 */
		int checkMagic(const uint8_t *magic, size_t siz);

	protected:
		// Common variables accessible by subclasses.
		std::string m_filename;	// Filename.
		FILE *m_file;		// Opened file handle.
		int m_lastError;	// Last error. (POSIX error code)
};

/**
 * Get the last error code. (POSIX error codes)
 * This value is NOT reset if a function is successful.
 *
 * Unlike function return values, this error code is
 * always a positive value.
 *
 * @return Last error code, or 0 if no error occurred.
 */
inline int Archive::lastError(void) const
{
	return m_lastError;
}

/**
 * Is the file open?
 * @return True if open; false if not.
 */
inline bool Archive::isOpen(void) const
{
	return (m_file != NULL);
}

/**
 * Read an entire file from the archive.
 *
 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract.
 * @param buf		[out] Buffer to read the file into.
 * @param siz		[in]  Size of buf. (Must be >= the size of the file.)
 * @param ret_siz	[out] Pointer to file_offset_t to store the number of bytes read.
 * @return 0 on success; negative POSIX error code on error.
 */
inline int Archive::readFile(const mdp_z_entry_t *z_entry, void *buf, file_offset_t siz, file_offset_t *ret_siz)
{
	// TODO: Convert z_entry->filesize to file_offset_t?
	file_offset_t max_size = std::min(static_cast<file_offset_t>(z_entry->filesize), siz);
	return readFile(z_entry, 0, max_size, buf, siz, ret_siz);
}

}

#endif /* __LIBGENSFILE_ARCHIVE_HPP__ */
