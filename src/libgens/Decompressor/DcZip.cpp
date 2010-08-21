/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DcZip.cpp: Zip decompressor class.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "DcZip.hpp"

// C includes.
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#ifdef _WIN32
// Win32 requires io.h for dup().
#include <io.h>

// MiniZip Win32 I/O handler.
#include "../../extlib/minizip/iowin32u.h"
#endif /* _WIn32 */

// LOG_MSG() subsystem.
#include "macros/log_msg.h"

// C++ includes.
#include <string>
using std::string;

// gzclose_r() and gzclose_w() were introduced in zlib-1.2.4.
#if (ZLIB_VER_MAJOR > 1) || \
    (ZLIB_VER_MAJOR == 1 && ZLIB_VER_MINOR > 2) || \
    (ZLIB_VER_MAJOR == 1 && ZLIB_VER_MINOR == 2 && ZLIB_VER_REVISION >= 4)
// zlib-1.2.4 or later
#else
#define gzclose_r(file) gzclose(file)
#define gzclose_w(file) gzclose(file)
#endif

namespace LibGens
{

/**
 * DcZip(): Create a new DcZip object.
 * @param f File pointer.
 * @param filename Filename.
 */
DcZip::DcZip(FILE *f, const utf8_str *filename)
	: Decompressor(f, filename)
{
	// MiniZip doesn't support opening files by fd.
	if (!f || !filename)
	{
		// No filename specified.
		m_file = NULL;
		m_filename.clear();
		m_unzFile = NULL;
		return;
	}
	
	// Open the file using MiniZip.
#ifdef _WIN32
	zlib_filefunc64_def ffunc;
	fill_win32_filefunc64U(&ffunc);
	m_unzFile = unzOpen2_64(filename, &ffunc);
#else
	m_unzFile = unzOpen(filename);
#endif
	if (!m_unzFile)
	{
		// Error opening the file.
		m_file = NULL;
		m_filename.clear();
		return;
	}
}

/**
 * ~DcZip(): Delete the Zip Decompressor object.
 */
DcZip::~DcZip()
{
	// Close the Zip file.
	if (m_unzFile)
		unzClose(m_unzFile);
}


/**
 * DetectFormat(): Detect if the file can be handled by this decompressor.
 * This function should be reimplemented by derived classes.
 * NOTE: Do NOT call this function like a virtual function!
 * @param f File pointer.
 * @return True if the file can be handled by this decompressor.
 */
bool DcZip::DetectFormat(FILE *f)
{
	static const uint8_t zip_magic[] = {'P', 'K', 0x03, 0x04};
	
	// Seek to the beginning of the file.
	// TODO: fseeko()/fseeko64() support on Linux.
	fseek(f, 0, SEEK_SET);
	
	// Read the "magic number".
	uint8_t header[sizeof(zip_magic)];
	size_t ret = fread(&header, 1, sizeof(header), f);
	if (ret < sizeof(header))
	{
		// Error reading the "magic number".
		return false;
	}
	
	// Check the "magic number" and return true if it matches.
	return (!memcmp(header, zip_magic, sizeof(header)));
}


/**
 * getFileInfo(): Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code. [TODO]
 */
int DcZip::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	// Make sure a pointer to mdp_z_entry_t* was given.
	if (!z_entry_out)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the file is open.
	if (!m_file || !m_unzFile)
		return -2;
	
	// List head and tail.
	mdp_z_entry_t *z_entry_head = NULL;
	mdp_z_entry_t *z_entry_tail = NULL;
	
	// MiniZip buffers.
	char rom_filename[256];
	unz_file_info zinfo;
	
	// Find the first ROM file in the Zip archive.
	// TODO: Get UTF-8 filenames from the extra field.
	// TODO: Convert regular filename field from original encoding.
	int i = unzGoToFirstFile(m_unzFile);
	while (i == UNZ_OK)
	{
		// TODO: Support unzGetCurrentFileInfo64().
		unzGetCurrentFileInfo(m_unzFile, &zinfo, rom_filename, sizeof(rom_filename), NULL, 0, NULL, 0);
		rom_filename[sizeof(rom_filename)-1] = 0x00;
		
		if (zinfo.external_fa & 0x10)
		{
			// Directory. (0x10 == MS-DOS attribute for directory.)
			// Skip this file.
			i = unzGoToNextFile(m_unzFile);
			continue;
		}
		
		// Allocate memory for the next file list element.
		// NOTE: C-style malloc() is used because MDP is a C API.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
		
		// Store the ROM file information.
		z_entry_cur->filename = (rom_filename[0] != 0x00 ? strdup(rom_filename) : NULL);
		z_entry_cur->filesize = zinfo.uncompressed_size;
		z_entry_cur->next = NULL;
		
		if (!z_entry_head)
		{
			// List hasn't been created yet. Create it.
			z_entry_head = z_entry_cur;
			z_entry_tail = z_entry_cur;
		}
		else
		{
			// Append the mdp_z_entry to the end of the list.
			z_entry_tail->next = z_entry_cur;
			z_entry_tail = z_entry_cur;
		}
		
		// Go to the next file.
		i = unzGoToNextFile(m_unzFile);
	}
	
	// If there are no files in the archive, return an error.
	if (!z_entry_head)
		return -1; // TODO: return -MDP_ERR_Z_NO_FILES_IN_ARCHIVE;
	
	// Return the list of files.
	*z_entry_out = z_entry_head;
	return 0; // TODO: return MDP_ERR_OK;
}


/**
 * getFile(): Get a file from the archive.
 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract. [TODO]
 * @param buf		[out] Buffer to read the file into.
 * @param siz		[in]  Size of buf.
 * @param ret_siz	[out] Pointer to size_t to store the number of bytes read.
 * @return MDP error code. [TODO]
 */
int DcZip::getFile(const mdp_z_entry_t *z_entry, void *buf, size_t siz, size_t *ret_siz)
{
	// Make sure all parameters are specified.
	if (!z_entry || !buf || !siz || !ret_siz)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the file is open.
	if (!m_file || !m_unzFile)
		return -2; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the z_entry filename is valid.
	if (!z_entry->filename)
	{
		// Invalid filename.
		return -1; // TODO: Return an appropriate MDP error code.
	}
	
	// Locate the ROM in the Zip file.
	// TODO: unzLocateFile() might not work with UTF-8 filenames.
	if (unzLocateFile(m_unzFile, z_entry->filename, 1) != UNZ_OK)
	{
		// File not found.
		return -2; // TODO: return -MDP_ERR_Z_FILE_NOT_FOUND_IN_ARCHIVE;
	}
	
	if (unzOpenCurrentFile(m_unzFile) != UNZ_OK)
	{
		// Error opening the file in the Zip archive.
		// TODO: Specific error code?
		return -3; // TODO: return -MDP_ERR_Z_FILE_NOT_FOUND_IN_ARCHIVE;
	}
	
	// Decompress the ROM.
	int zResult = unzReadCurrentFile(m_unzFile, buf, siz);
	if ((zResult <= 0) || ((size_t)zResult != siz))
	{
		const char *zip_err;
		
		// TODO: Add MDP Z errors for these.
		switch (zResult)
		{
			case UNZ_ERRNO:
				zip_err = "Unknown...";
				break;
			case UNZ_EOF:
				zip_err = "Unexpected end of file.";
				break;
			case UNZ_PARAMERROR:
				zip_err = "Parameter error.";
				break;
			case UNZ_BADZIPFILE:
				zip_err = "Bad ZIP file.";
				break;
			case UNZ_INTERNALERROR:
				zip_err = "Internal error.";
				break;
			case UNZ_CRCERROR:
				zip_err = "CRC error.";
				break;
			default:
				zip_err = "Unknown error.";
				break;
		}
		
		LOG_MSG(z, LOG_MSG_LEVEL_CRITICAL,
			"Error extracting file '%s' from archive '%s': %s",
			z_entry->filename, m_filename.c_str(), zip_err);
		return -4; // TODO: return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}
	
	// File extracted successfully.
	*ret_siz = (size_t)zResult;
	return 0; // TODO: return MDP_ERR_OK;
}

}
