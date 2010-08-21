/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Decompressor.cpp: Decompressor base class.                              *
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

#include "Decompressor.hpp"

// C includes.
#include <stdlib.h>
#include <string.h>

// C++ includes.
#include <string>
using std::string;

namespace LibGens
{

/**
 * Decompressor(): Create a new Decompressor object.
 * @param f File pointer.
 * @param filename Filename.
 */
Decompressor::Decompressor(FILE *f, const utf8_str *filename)
{
	// Keep copies of the file pointer and filename.
	m_file = f;
	m_filename = string(filename);
}

/**
 * ~Decompressor(): Delete the Decompressor object.
 */
Decompressor::~Decompressor()
{
	// NOTE: File pointer is NOT closed by the Decompressor object.
	m_file = NULL;
}


/**
 * getFileInfo(): Get information about all files in the archive.
 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code. [TODO]
 */
int Decompressor::getFileInfo(mdp_z_entry_t **z_entry_out)
{
	// Make sure a pointer to mdp_z_entry_t* was given.
	if (!z_entry_out)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// TODO: fseeko()/ftello()/fseeko64()/ftello64() support on Linux.
	size_t filesize;
	fseek(m_file, 0, SEEK_END);
	filesize = ftell(m_file);
	
	// Allocate an mdp_z_entry_t.
	// NOTE: C-style malloc() is used because MDP is a C API.
	mdp_z_entry_t *z_entry = (mdp_z_entry_t*)malloc(sizeof(mdp_z_entry_t));
	
	// Set the elements of the list entry.
	z_entry->filesize = filesize;
	z_entry->filename = (m_filename.size() > 0 ? strdup(m_filename.c_str()) : NULL);
	z_entry->next = NULL;
	
	// Return the list.
	*z_entry_out = z_entry;
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
int Decompressor::getFile(const mdp_z_entry_t *z_entry, void *buf, size_t siz, size_t *ret_siz)
{
	// Make sure all parameters are specified.
	if (!z_entry || !buf || !siz || !ret_siz)
		return -1; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Make sure the file is open.
	if (!m_file)
		return -2; // TODO: return -MDP_ERR_INVALID_PARAMETERS;
	
	// Seek to the beginning of the file.
	// TODO: fseeko()/fseeko64() support on Linux.
	fseek(m_file, 0, SEEK_SET);
	
	// Read the file into the buffer.
	*ret_siz = fread(buf, 1, siz, m_file);
	return 0; // TODO: return MDP_ERR_OK;
}


/**
 * z_entry_t_free(): Free an allocated mdp_z_entry_t list.
 * @param z_entry Pointer to the first entry in the list.
 */
void Decompressor::z_entry_t_free(mdp_z_entry_t *z_entry)
{
	// Delete the mdp_z_entry_t list.
	for (mdp_z_entry_t *next; z_entry != NULL; z_entry = next)
	{
		// Save the next pointer.
		next = z_entry->next;
		
		// Free the filename and the mdp_z_entry_t.
		free(z_entry->filename);
		free(z_entry);
	}
}

}
