/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * DcZip.hpp: Zip decompressor class.                                      *
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

#ifndef __LIBGENS_DECOMPRESSOR_DCZIP_HPP__
#define __LIBGENS_DECOMPRESSOR_DCZIP_HPP__

#include "Decompressor.hpp"
#include "../../extlib/minizip/unzip.h"

namespace LibGens {

class DcZip : public Decompressor
{
	public:
		DcZip(FILE *f, const char *filename);
		virtual ~DcZip();

		/**
		 * Detect if the file can be handled by this decompressor.
		 * This function should be reimplemented by derived classes.
		 * NOTE: Do NOT call this function like a virtual function!
		 * @param f File pointer.
		 * @return True if the file can be handled by this decompressor.
		 */
		static bool DetectFormat(FILE *f);

		/**
		 * Get information about all files in the archive.
		 * @param z_entry_out Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
		 * @return MDP error code. [TODO]
		 */
		int getFileInfo(mdp_z_entry_t **z_entry_out);

		/**
		 * Get a file from the archive.
		 * @param z_entry	[in]  Pointer to mdp_z_entry_t describing the file to extract.
		 * @param buf		[out] Buffer to read the file into.
		 * @param siz		[in]  Size of buf.
		 * @param ret_siz	[out] Pointer to size_t to store the number of bytes read.
		 * @return MDP error code. [TODO]
		 */
		int getFile(const mdp_z_entry_t *z_entry, void *buf, size_t siz, size_t *ret_siz);

	private:
		unzFile m_unzFile;
};

}

#endif /* __LIBGENS_DECOMPRESSOR_DECOMPRESSOR_HPP__ */
