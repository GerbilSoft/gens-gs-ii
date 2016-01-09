/***************************************************************************
 * libgensfile: Gens file handling library.                                *
 * Rar.hpp: RAR archive handler.                                           *
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

#ifndef __LIBGENSFILE_RAR_HPP__
#define __LIBGENSFILE_RAR_HPP__

#include "Archive.hpp"
#include "UnRAR_dll.hpp"

namespace LibGensFile {

class Rar : public Archive
{
	public:
		/**
		 * Open a file with this archive handler.
		 * Check isOpen() afterwards to see if the file was opened.
		 * If it wasn't, check lastError() for the POSIX error code.
		 * @param filename Name of the file to open.
		 */
		Rar(const char *filename);
		virtual ~Rar();

	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Rar(const Rar &);
		Rar &operator=(const Rar &);

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
		// UnRAR.dll filename.
		static const char m_unrarDll_filename[];

		// UnRAR.dll instance.
		UnRAR_dll m_unrarDll;

		/**
		 * Open the Archive's file using UnRAR.dll.
		 * @param mode RAR open mode.
		 * @return RAR handle, or nullptr on error. (TODO: Error code?)
		 */
		HANDLE openRar(int mode);

		// RAR state.
		struct RarState_t {
			uint8_t *buf;	// Buffer.
			size_t siz;	// Size of buffer.
			size_t pos;	// Current position.

			Rar *owner;	// Rar instance that owns this RarState_t.
		};

		/**
		 * UnRAR.dll callback function.
		 */
		static int CALLBACK RarCallback(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2);

	public:
		// TODO: Port to Archive.
#if 0
		struct ExtPrgInfo {
			// DLL version number.
			uint16_t dll_major;
			uint16_t dll_minor;
			uint16_t dll_revision;
			uint16_t dll_build;

			// API version number.
			int api_version;

			// Executable type.
			enum RarExecType {
				RAR_ET_UNKNOWN = 0,
				RAR_ET_UNRAR = 1,
				RAR_ET_RAR = 2,
				RAR_ET_UNRAR_DLL = 3,
			};
			RarExecType rar_type;
		};

		/**
		 * Check if the specified external RAR program is usable.
		 * @param extprg	[in] External RAR program filename.
		 * @param prg_info	[out] If not nullptr, contains RAR/UnRAR version information.
		 * @return Possible error codes:
		 * -  0: Program is usable.
		 * - -1: File not found.
		 * - -2: File isn't executable
		 * - -3: File isn't a regular file. (e.g. it's a directory)
		 * - -4: Error calling stat().
		 * - -5: Wrong DLL API version. (Win32 only)
		 * - -6: Version information not found.
		 * - -7: Not RAR, UnRAR, or UnRAR.dll.
		 * TODO: Use MDP error code constants.
		 */
		static uint32_t CheckExtPrg(const char *extprg, ExtPrgInfo *prg_info);

		/**
		 * Set the external RAR program filename.
		 * @param extprg External RAR program filename.
		 */
		static void SetExtPrg(const char *extprg)
			{ ms_RarBinary = std::string(extprg); }

		/**
		 * Set the external RAR program filename.
		 * @return External RAR program filename.
		 */
		static const char *GetExtPrg(void)
			{ return ms_RarBinary.c_str(); }

	private:
		/**
		 * RAR executable filename.
		 * Unix: Filename of "rar" or "unrar".
		 * Windows: Filename of "unrar.dll".
		 */
		static std::string ms_RarBinary;
#endif
};

}

#endif /* __LIBGENSFILE_RAR_HPP__ */
