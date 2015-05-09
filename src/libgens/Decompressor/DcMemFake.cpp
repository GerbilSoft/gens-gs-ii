/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DcMemFake.hpp: Fake decompressor using a ROM already loaded in memory.  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include "DcMemFake.hpp"

// C includes.
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
// Win32 requires io.h for dup() and close().
#include <io.h>
#endif /* _WIn32 */

// C++ includes.
#include <string>
using std::string;

namespace LibGens {

DcMemFake::DcMemFake(FILE *f, const utf8_str *filename)
	: Decompressor(f, filename)
{
	// This function should NOT be called.
	((void)f);
	((void)filename);
	abort();
}

/**
 * Create a new DcMemFake object.
 * @param rom_data ROM data.
 * @param rom_size ROM size.
 */
DcMemFake::DcMemFake(const uint8_t *rom_data, unsigned int rom_size)
	: Decompressor(nullptr, "")
	, rom_data(rom_data)
	, rom_size(rom_size)
	, pos(0)
{ }

/**
 * Delete the GZip decompressor object.
 */
DcMemFake::~DcMemFake()
{ }

/**
 * Detect if the file can be handled by this decompressor.
 * This function should be reimplemented by derived classes.
 * NOTE: Do NOT call this function like a virtual function!
 * @param f File pointer.
 * @return True if the file can be handled by this decompressor.
 */
bool DcMemFake::DetectFormat(FILE *f)
{
	// This decompressor can't handle files.
	return false;
}

/**
 * Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code. [TODO]
 */
int DcMemFake::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	// Make sure a pointer to mdp_z_entry_t* was given.
	if (!z_entry_out)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;

	// Allocate an mdp_z_entry_t.
	// NOTE: C-style malloc() is used because MDP is a C API.
	mdp_z_entry_t *z_entry = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));

	// Set the phony entry.
	z_entry->filesize = rom_size;
	z_entry->filename = nullptr;
	z_entry->next = nullptr;

	// Return the list.
	*z_entry_out = z_entry;
	return 0; // TODO: return MDP_ERR_OK;
}

/**
 * Get a file from the archive.
 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract. [TODO]
 * @param buf		[out] Buffer to read the file into.
 * @param siz		[in]  Size of buf.
 * @param ret_siz	[out] Pointer to size_t to store the number of bytes read.
 * @return MDP error code. [TODO]
 */
int DcMemFake::getFile(const mdp_z_entry_t *z_entry, void *buf, size_t siz, size_t *ret_siz)
{
	// Make sure all parameters are specified.
	if (!z_entry || !buf || !siz || !ret_siz)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;

	// Are we in range?
	*ret_siz = siz;
	if (*ret_siz > rom_size) {
		// Too big. Truncate the read amount.
		*ret_siz = rom_size;
	}

	// Read the file into the buffer.
	memcpy(buf, rom_data, *ret_siz);
	return 0; // TODO: return MDP_ERR_OK;
}

}
