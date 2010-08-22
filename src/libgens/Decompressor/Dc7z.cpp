/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Dc7z.cpp: 7-Zip decompressor class.                                     *
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

/**
 * Based on 7zMain.c from the LZMA SDK 9.12.
 * http://www.7-zip.org/sdk.html
 */

#include "Dc7z.hpp"

// C includes.
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

// LOG_MSG() subsystem.
#include "macros/log_msg.h"

// C++ includes.
#include <string>
using std::string;

// 7-Zip includes.
#include "lzma/7z/7zAlloc.h"
#include "lzma/7z/7zVersion.h"
#include "lzma/lzmabase/7zCrc.h"

namespace LibGens
{

/** Static class variable initialization. **/

/**
 * ms_CrcInit: Set to true if the 7z CRC table has been initialized.
 */
bool Dc7z::ms_CrcInit = false;

/**
 * Dc7z(): Create a new Dc7z object.
 * @param f File pointer.
 * @param filename Filename.
 */
Dc7z::Dc7z(FILE *f, const utf8_str *filename)
	: Decompressor(f, filename)
{
	SRes res;
	
	/**
	 * NOTE: This initialization MUST be done before checking
	 * for a filename error!
	 */
	
	// Initialize the miscellaneous 7-Zip variables.
	m_blockIndex = ~0;
	m_outBuffer = NULL;
	m_outBufferSize = 0;
	
	// Initialize the memory allocators.
	m_allocImp.Alloc = SzAlloc;
	m_allocImp.Free = SzFree;
	
	// Initialize the temporary memory allocators.
	m_allocTempImp.Alloc = SzAllocTemp;
	m_allocTempImp.Free = SzFreeTemp;
	
	/** End of basic Dc7z initialization. **/
	
	// LZMA SDK doesn't support opening files by fd.
	if (!f || !filename)
	{
		// No filename specified.
		m_file = NULL;
		m_filename.clear();
		return;
	}
	
	// Attempt to open the file.
	// TODO: Unicode filename support?
	if (InFile_Open(&m_archiveStream.file, filename))
	{
		// Error opening the file.
		m_file = NULL,
		m_filename.clear();
		return;
	}
	
	// Initialize VTables.
	FileInStream_CreateVTable(&m_archiveStream);
	LookToRead_CreateVTable(&m_lookStream, false);
	
	m_lookStream.realStream = &m_archiveStream.s;
	LookToRead_Init(&m_lookStream);
	
	// Generate the CRC table.
	if (!ms_CrcInit)
	{
		CrcGenerateTable();
		ms_CrcInit = true;
	}
	
	SzArEx_Init(&m_db);
	res = SzArEx_Open(&m_db, &m_lookStream.s, &m_allocImp, &m_allocTempImp);
	if (res != SZ_OK)
	{
		// Error opening the file.
		SzArEx_Free(&m_db, &m_allocImp);
		File_Close(&m_archiveStream.file);
		
		m_file = NULL;
		m_filename.clear();
		return;
	}
	
	// 7-Zip file is opened.
}

/**
 * ~Dc7z(): Delete the 7-Zip Decompressor object.
 */
Dc7z::~Dc7z()
{
	// Free the output buffer, if it's allocated.
	if (m_outBuffer)
	{
		IAlloc_Free(&m_allocImp, m_outBuffer);
		m_outBuffer = NULL;
	}
	
	// Close the 7-Zip file.
	if (m_file)
	{
		SzArEx_Free(&m_db, &m_allocImp);
		File_Close(&m_archiveStream.file);
	}
}


/**
 * DetectFormat(): Detect if the file can be handled by this decompressor.
 * This function should be reimplemented by derived classes.
 * NOTE: Do NOT call this function like a virtual function!
 * @param f File pointer.
 * @return True if the file can be handled by this decompressor.
 */
bool Dc7z::DetectFormat(FILE *f)
{
	static const uint8_t _7z_magic[] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};
	
	// Seek to the beginning of the file.
	// TODO: fseeko()/fseeko64() support on Linux.
	fseek(f, 0, SEEK_SET);
	
	// Read the "magic number".
	uint8_t header[sizeof(_7z_magic)];
	size_t ret = fread(&header, 1, sizeof(header), f);
	if (ret < sizeof(header))
	{
		// Error reading the "magic number".
		return false;
	}
	
	// Check the "magic number" and return true if it matches.
	return (!memcmp(header, _7z_magic, sizeof(header)));
}


/**
 * getFileInfo(): Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code. [TODO]
 */
int Dc7z::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	// Make sure a pointer to mdp_z_entry_t* was given.
	if (!z_entry_out)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the file is open.
	if (!m_file)
		return -2;
	
	// List head and tail.
	mdp_z_entry_t *z_entry_head = NULL;
	mdp_z_entry_t *z_entry_tail = NULL;
	
	// Filename buffer. (UTF-16)
	uint16_t *wFilename = NULL;
	size_t wFilename_len = 0;
	
	// Read the filenames.
	for (unsigned int i = 0; i < m_db.db.NumFiles; i++)
	{
		const CSzFileItem *f = (m_db.db.Files + i);
		if (f->IsDir)
			continue;
		
		// Get the filename.
		size_t len = SzArEx_GetFileNameUtf16(&m_db, i, NULL);
		if (len > wFilename_len)
		{
			// Increase the wFilename buffer.
			// TODO: Check for malloc() errors?
			free(wFilename);
			wFilename_len = len;
			wFilename = (uint16_t*)malloc(wFilename_len * sizeof(uint16_t));
		}
		SzArEx_GetFileNameUtf16(&m_db, i, wFilename);
		
		// Convert the filename to UTF-8.
		// TODO: For now, we'll just use the LSB of each UTF-16 character.
		utf8_str *rom_filename = (utf8_str*)malloc(wFilename_len);
		for (unsigned int chr = 0; chr < len; chr++)
		{
			rom_filename[chr] = (wFilename[chr] & 0xFF);
		}
		
		// Allocate memory for the next file list element.
		// NOTE: C-style malloc() is used because MDP is a C API.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
		
		// Store the ROM file information.
		// TODO: f->Size is 64-bit...
		z_entry_cur->filename = rom_filename;
		z_entry_cur->filesize = (size_t)f->Size;
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
int Dc7z::getFile(const mdp_z_entry_t *z_entry, void *buf, size_t siz, size_t *ret_siz)
{
	SRes res;
	
	// Make sure all parameters are specified.
	if (!z_entry || !buf || !siz || !ret_siz)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the file is open.
	if (!m_file)
		return -2; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the z_entry filename is valid.
	if (!z_entry->filename)
	{
		// Invalid filename.
		return -1; // TODO: Return an appropriate MDP error code.
	}
	
	// Filename buffer. (UTF-16)
	uint16_t *wFilename = NULL;
	size_t wFilename_len = 0;
	
	// Locate the file in the 7-Zip archive.
	unsigned int i = 0;
	for (i = 0; i < m_db.db.NumFiles; i++)
	{
		const CSzFileItem *f = (m_db.db.Files + i);
		if (f->IsDir)
			continue;
		
		// Get the filename.
		size_t len = SzArEx_GetFileNameUtf16(&m_db, i, NULL);
		if (len > wFilename_len)
		{
			// Increase the wFilename buffer.
			// TODO: Check for malloc() errors?
			free(wFilename);
			wFilename_len = len;
			wFilename = (uint16_t*)malloc(wFilename_len * sizeof(uint16_t));
		}
		SzArEx_GetFileNameUtf16(&m_db, i, wFilename);
		
		// Convert the filename to UTF-8.
		// TODO: For now, we'll just use the LSB of each UTF-16 character.
		// TODO: Convert the input filename to UTF-16 to improve performance?
		utf8_str *rom_filename = (utf8_str*)malloc(wFilename_len);
		for (unsigned int chr = 0; chr < len; chr++)
		{
			rom_filename[chr] = (wFilename[chr] & 0xFF);
		}
		
		// Check if the filename matches.
#ifdef _WIN32
		if (strcasecmp(z_entry->filename, rom_filename) != 0)
#else
		if (strcmp(z_entry->filename, rom_filename) != 0)
#endif
		{
			// Not the correct file.
			free(rom_filename);
			continue;
		}
		
		// Found the file.
		free(rom_filename);
		
		// Extract the file into the buffer.
		size_t offset;
		size_t outSizeProcessed;
		res = SzArEx_Extract(&m_db, &m_lookStream.s, i,
				     &m_blockIndex, &m_outBuffer, &m_outBufferSize,
				     &offset, &outSizeProcessed,
				     &m_allocImp, &m_allocTempImp);
		
		if (res != SZ_OK)
		{
			// Error extracting the file.
			// TODO: Return an appropriate MDP error code.
			// For now, just break out of the loop.
			i = m_db.db.NumFiles;
			break;
		}
		
		// Copy the 7z buffer to the output buffer.
		*ret_siz = (siz < outSizeProcessed ? siz : outSizeProcessed);
		memcpy(buf, (m_outBuffer + offset), *ret_siz);
		
		// ROM processed.
		break;
	}
	
	if (i >= m_db.db.NumFiles)
	{
		// File not found.
		// TODO: Could also be an error in extracting the file.
		return -1; // TODO: return -MDP_ERR_Z_FILE_NOT_FOUND_IN_ARCHIVE;
	}
	
	// File extracted successfully.
	return 0; // TODO: return MDP_ERR_OK;
}

}
