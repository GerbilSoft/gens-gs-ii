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

namespace LibGens
{

/**
 * Decompressor(): Create a new Decompressor object.
 * @param f File pointer.
 */
Decompressor::Decompressor(FILE *f)
{
	// Keep a copy of the file pointer.
	m_file = f;
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
 * getFile(): Get a file from the archive.
 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract. [TODO]
 * @param buf		[out] Buffer to read the file into.
 * @param siz		[in]  Size of buf.
 * @param ret_siz	[out] Pointer to size_t to store the number of bytes read.
 * @return MDP error code. [TODO]
 */
int Decompressor::getFile(const void *z_entry, void *buf, size_t siz, size_t *ret_siz)
{
	// Seek to the beginning of the file.
	fseeko(m_file, 0, SEEK_SET);
	
	// Read the file into the buffer.
	*ret_siz = fread(buf, 1, siz, m_file);
	return 0;
}

}
