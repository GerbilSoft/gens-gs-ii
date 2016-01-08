/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * MemFake.cpp: Fake archive handler using data already loaded in memory.  *
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

#include "MemFake.hpp"

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <cstdlib>
#include <cstring>

namespace LibGens { namespace File {

/**
 * This archive handler does NOT support opening files.
 * Calling this function will result in SIGABRT.
 * @param filename
 */
MemFake::MemFake(const char *filename)
	: Archive(filename)
{
	// This function should NOT be called.
	((void)filename);
	abort();
}

/**
 * Open an area of memory as if it's an archive.
 * This is useful for some test suites.
 *
 * NOTE: The specified memory area is NOT copied.
 * Do NOT free the memory unless you either call
 * MemFake::close() or delete the MemFake object.
 *
 * Check isOpen() afterwards to see if the file was opened.
 * If it wasn't, check lastError() for the POSIX error code.
 *
 * @param rom_data ROM data.
 * @param rom_size Size of rom_data.
 */
MemFake::MemFake(const uint8_t *rom_data, unsigned int rom_size)
	: Archive("")
	, m_rom_data(rom_data)
	, m_rom_size(rom_size)
	, m_pos(0)
{ }

/**
 * Delete the MemFake object.
 */
MemFake::~MemFake()
{ }

/**
 * Close the archive file.
 */
void MemFake::close(void)
{
	m_rom_data = nullptr;
	m_rom_size = 0;
	m_pos = 0;

	// Base class closes the FILE*.
	Archive::close();
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return 0 on success; negative POSIX error code on error.
 */
int MemFake::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	if (!z_entry_out) {
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_rom_data) {
		// Note that m_file is not checked since we're not using a file.
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// Allocate an mdp_z_entry_t.
	// NOTE: C-style malloc() is used because MDP is a C API.
	mdp_z_entry_t *z_entry = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
	if (!z_entry) {
		m_lastError = ENOMEM;
		return -m_lastError;
	}

	// Set the phony entry.
	z_entry->filesize = m_rom_size;
	z_entry->filename = nullptr;
	z_entry->next = nullptr;

	// Return the list.
	*z_entry_out = z_entry;
	return 0; // TODO: return MDP_ERR_OK;
}

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
int MemFake::readFile(const mdp_z_entry_t *z_entry,
		      file_offset_t start_pos, file_offset_t read_len,
		      void *buf, file_offset_t siz, file_offset_t *ret_siz)
{
	// We're using m_rom_size instead of z_entry->filesize.
	if (!z_entry || !buf ||
	    start_pos < 0 || start_pos >= m_rom_size ||
	    read_len < 0 || m_rom_size - read_len < start_pos ||
	    siz <= 0 || siz < read_len)
	{
		m_lastError = EINVAL;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	} else if (!m_rom_data) {
		// Note that m_file is not checked since we're not using a file.
		m_lastError = EBADF;
		return -m_lastError; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	}

	// Read the file into the buffer.
	memcpy(buf, &m_rom_data[start_pos], read_len);
	*ret_siz = read_len;
	return 0; // TODO: return MDP_ERR_OK;
}

} }
